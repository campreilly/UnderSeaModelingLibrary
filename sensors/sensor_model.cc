/**
 *  @file sensor_model.cc
 *  Implementation of the Class sensor_model
 *  Created on: 10-Feb-2015 12:49:09 PM
 */
#include <usml/sensors/sensor_model.h>
#include <usml/sensors/source_params_map.h>
#include <usml/sensors/receiver_params_map.h>
#include <usml/eigenverb/wavefront_generator.h>
#include <usml/ocean/ocean_shared.h>
#include <boost/foreach.hpp>

using namespace usml::sensors;
using namespace usml::waveq3d;

const double sensor_model::alt_threshold = 10.0 ;       // meters
const double sensor_model::lat_threshold = 0.01 ;       // degrees
const double sensor_model::lon_threshold = 0.01 ;       // degrees
const double sensor_model::pitch_threshold = 5.0 ;      // degrees
const double sensor_model::heading_threshold = 20.0 ;   // degrees
const double sensor_model::roll_threshold = 10.0 ;      // degrees

/**
 * Construct a new instance of a specific sensor type.
 */
sensor_model::sensor_model(sensor_model::id_type sensorID, sensor_params::id_type paramsID,
	const std::string& description)
	: _sensorID(sensorID), _paramsID(paramsID), _description(description),
	  _position(NAN, NAN, NAN), _orient(), _initial_update(true)
{
	_source = source_params_map::instance()->find(paramsID);
	_receiver = receiver_params_map::instance()->find(paramsID);
    bool has_source = _source.get() != NULL;
    bool has_receiver = _receiver.get() != NULL;
    
    if ( has_source && has_receiver )
    {
        _mode = BOTH;
    }
    else if ( has_source )
    {
        _mode = SOURCE;
    }
    else if ( has_receiver )
    {
        _mode = RECEIVER;
    }
    else
    {
        _mode = NONE;
    }

    // Set frequency band from _source or _receiver 
    // and min/max active frequencies
    frequencies();
}

/**
 * Removes a sensor_model instance from simulation.
 */
sensor_model::~sensor_model() {
	if ( _wavefront_task.get() != 0 ) {
		_wavefront_task->abort();
	}
}

/**
 * Queries the sensor's ability to support source and/or receiver behaviors.
 */
xmitRcvModeType sensor_model::mode() const {
	return _mode;
}

/**
 * Location of the sensor in world coordinates.
 */
wposition1 sensor_model::position() const {
	read_lock_guard guard(_update_sensor_mutex);
	return _position;
}

/**
 * Orientation of the sensor in world coordinates.
 */
orientation sensor_model::orient() const {
	read_lock_guard guard(_update_sensor_mutex);
	return _orient;
}

/**
 * Checks to see if new position and orientation have changed enough
 * to require a new WaveQ3D run.
 */
void sensor_model::update_sensor(const wposition1& position,
		const orientation& orientation, bool force_update)
{
    { // For scope of lock
        write_lock_guard guard(_update_sensor_mutex);
        if (!force_update) {
            if (!check_thresholds(position, orientation)) {
                return;
            }
        }
        #ifdef USML_DEBUG
            cout << "sensor_model: update_sensor(" << _sensorID << ")" << endl ;
        #endif
        _position = position;
        _orient = orientation;
    }
	run_wave_generator();
}

/**
 * Last set of eigenverbs computed for this sensor.
 */
eigenverb_collection::reference sensor_model::eigenverbs() const {
	read_lock_guard guard(_eigenverbs_mutex);
	return _eigenverb_collection;
}

/**
 * Asynchronous update of eigenrays and eigenverbs data from the wavefront task.
 * Passes this data onto all sensor listeners.
 * Blocks until update is complete.
 */
void sensor_model::update_wavefront_data(eigenray_collection::reference& eigenrays,
                              eigenverb_collection::reference& eigenverbs) {
    // Don't allow updates to _sensor_listeners
    write_lock_guard guard(_sensor_listeners_mutex);
#ifdef USML_DEBUG
    cout << "sensor_model: update_wavefront_data(" << _sensorID << ")" << endl;
#endif

    {   // Scope for lock on _eigenray_collection
        write_lock_guard guard(_eigenrays_mutex);
        _eigenray_collection = eigenrays;
    }
    {   // Scope for lock on _eigenverb_collection
        write_lock_guard guard(_eigenverbs_mutex);
        _eigenverb_collection = eigenverbs;
        // For Source_eigenverbs generate rtrees to quickly query for overlaps
        if (_source.get() != NULL) {
        	_eigenverb_collection->generate_rtrees();
        }
    }

    // Store first ray arrival time for update_eigenverbs
    double first_ray_arrival_time = 0.0;

    // For each sensor_pair
    BOOST_FOREACH(sensor_listener* listener, _sensor_listeners) {
        // Get complement sensor
        const sensor_model* complement = listener->sensor_complement(this);
        // Find complement's sensorID's index in target_id_map
        std::map<sensor_model::id_type, int>::const_iterator iter =
            _target_id_map.find(complement->sensorID());
        // Make sure it found the complement's sensorID
        if ( iter != _target_id_map.end() ) {
            read_lock_guard guard(_eigenrays_mutex);
            int row = iter->second;
            // Get eigenray_list for listener, ie sensor_pair
            // Get complement's row's eigenray_list
            eigenray_list* list = _eigenray_collection->eigenrays(row, 0);
#ifdef USML_DEBUG
            cout << "sensor_model: update_wavefront_data eigenray list size " << list->size() << endl;
#endif
            // Only update when eigenrays are found
            if ( list->size() > 0 ) {
                // Get first eigenrays arrival time
                std::list<eigenray>::const_iterator ray_iter = list->begin();
                first_ray_arrival_time = ray_iter->time;
                // Send out eigenray_list to listener
                listener->update_fathometer(_sensorID, list);
            }
        }
        listener->update_eigenverbs(first_ray_arrival_time, this);
    }
}

/**
 * Add a sensor_listener to the _sensor_listeners list
 */
void sensor_model::add_sensor_listener(sensor_listener* listener) {
	write_lock_guard guard(_sensor_listeners_mutex);
	_sensor_listeners.push_back(listener);
}

/**
 * Remove a sensor_listener from the _sensor_listeners list
 */
void sensor_model::remove_sensor_listener(sensor_listener* listener) {
	write_lock_guard guard(_sensor_listeners_mutex);
	_sensor_listeners.remove(listener);
}

/**
 * Checks to see if new position and orientation have changed enough
 * to require a new wave_queue to be run, as the data in the eigenrays
 * and eigenverbs are no longer sufficiently accurate.
 */
bool sensor_model::check_thresholds(const wposition1& position,
		const orientation& orient)
{
    // since the sensor defaults to NAN for positions
    // we already know that we need meet the threshold, so
    // don't worry about checking for this
    if( _initial_update ) {
        _initial_update = false ;
        return true ;
    }
    // check that the roll of the array hasn't changed too much
    double delta_alt = abs(position.altitude() - _position.altitude()) ;
    if(delta_alt > alt_threshold) return true ;

    // check that the roll of the array hasn't changed too much
    double delta_lat = abs(position.latitude() - _position.latitude()) ;
    if(delta_lat > lat_threshold) return true ;

    // check that the roll of the array hasn't changed too much
    double delta_lon = abs(position.longitude() - _position.longitude()) ;
    if(delta_lon > lon_threshold) return true ;

    // check that the pitch of the array hasn't changed too much
    double delta_pitch = abs(orient.pitch() - _orient.pitch()) ;
    if(delta_pitch > pitch_threshold) return true ;

    // check that the heading of the array hasn't changed too much
    double delta_heading = abs(orient.heading() - _orient.heading()) ;
    if(delta_heading > heading_threshold) return true ;

    // check that the roll of the array hasn't changed too much
    double delta_roll = abs(orient.roll() - _orient.roll()) ;
    if(delta_roll > roll_threshold) return true ;
    return false ;
}


/**
 * Queries the current list of sensor listeners for the complement
 * sensors of this sensor.
 */
std::list<const sensor_model*> sensor_model::sensor_targets() {

    read_lock_guard guard(_sensor_listeners_mutex);

    std::list<const sensor_model*> complements;

    BOOST_FOREACH( sensor_listener* listener, _sensor_listeners ) {
        complements.push_back(listener->sensor_complement(this));
    }
    return complements;
}

/**
 * Builds a list of sensorID's from the list of sensors provided.
 */
void sensor_model::target_ids(std::list<const sensor_model*>& list) {

    _target_id_map.clear();
    int row = -1;
    BOOST_FOREACH( const sensor_model* target, list ) {
        ++row;
        _target_id_map.insert(std::pair<sensor_model::id_type, int>(target->sensorID(), row));
    }
}

/**
 * Builds a list of target positions from the input list of sensors provided.
 */
const wposition* sensor_model::target_positions(std::list<const sensor_model*>& list) const {

	// builds wposition container of target positions from the list provided.

    wposition* target_pos = new wposition(list.size(), 1);
	int row = -1;
	BOOST_FOREACH( const sensor_model* target, list ){
		++row;
		wposition1 pos = target->position();
		target_pos->latitude( row, 0, pos.latitude());
		target_pos->longitude(row, 0, pos.longitude());
		target_pos->altitude( row, 0, pos.altitude());
	}
	return target_pos ;
}

/**
 * Utility to set frequencies from sensor including min and max active frequencies
 */
void sensor_model::frequencies() {

    switch ( _mode ) {

        case usml::sensors::RECEIVER:
        {
            _min_active_freq = _receiver->min_active_freq();
            _max_active_freq = _receiver->max_active_freq();
            _frequencies.reset(_receiver->frequencies()->clone());
            break;
        }
        default:  // SOURCE or BOTH
        {
            _min_active_freq = _source->min_active_freq();
            _max_active_freq = _source->max_active_freq();
            _frequencies.reset(_source->frequencies()->clone());
        }
    }  
}


/**
 * Run the wavefront_generator thread task to start the waveq3d model.
 */
void sensor_model::run_wave_generator() {

    // Only run wavefront generator if ocean_model pointer is not NULL
    if (ocean_shared::current().get() != NULL ) {

        #ifdef USML_DEBUG
            cout << "sensor_model: run_wave_generator(" << _sensorID << ")" << endl ;
        #endif

        const wposition* target_pos = NULL;

        // Get the targets sensor references
        std::list<const sensor_model*> targets = sensor_targets();

        if (targets.size() > 0) {
            // Store the targetID's for later use in sending on to sensor_pairs
            target_ids(targets);

            // Get the target positions for wavefront_generator
            target_pos = target_positions(targets);
        }

        // Create the wavefront_generator
        wavefront_generator* generator = new wavefront_generator (
            ocean_shared::current(), _position, target_pos, _frequencies.get(), this);

        // Make wavefront_generator a wavefront_task, with use of shared_ptr
        _wavefront_task = thread_task::reference(generator);

        // Pass in to thread_pool
        thread_controller::instance()->run(_wavefront_task);
    }

    #ifdef USML_DEBUG
        if (ocean_shared::current().get() == NULL ) {
             cout << "sensor_model: run_wave_generator no ocean provided !!! (" << _sensorID << ")" << endl ;
        }
    #endif
}
