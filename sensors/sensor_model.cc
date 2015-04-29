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

/**
 * Construct a new instance of a specific sensor type.
 */
sensor_model::sensor_model(sensor_model::id_type sensorID, sensor_params::id_type paramsID,
	const std::string& description)
	: _sensorID(sensorID), _paramsID(paramsID), _description(description),
	  _position(NAN, NAN, NAN), _orient()
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

    // Select frequency band from _source or _receiver 
    // with min/max active included
    select_frequencies();
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
 * Asynchronous update of eigenrays data from the wavefront task.
 */
void sensor_model::update_eigenrays(eigenray_collection::reference& eigenrays)
{
    // Don't allow updates to _sensor_listeners
	write_lock_guard guard(_sensor_listeners_mutex);
	#ifdef USML_DEBUG
		cout << "sensor_model: update_eigenrays(" << _sensorID << ")" << endl ;
	#endif
	{   // Scope for lock on _eigenray_collection
        write_lock_guard guard(_eigenrays_mutex);
	    _eigenray_collection = eigenrays;
	}
	BOOST_FOREACH( sensor_listener* listener, _sensor_listeners ) {
	    // Get complement sensor
	    const sensor_model* complement = listener->sensor_complement(this);
        // Find complement's sensorID's index in target_id_map
        std::map<sensor_model::id_type,int>::const_iterator iter =
                            _target_id_map.find(complement->sensorID());
        // Make sure it found the complement's sensorID
        if (iter != _target_id_map.end()) {
            read_lock_guard guard(_eigenrays_mutex);
            int row = iter->second;
            // Get eigenray_list for listener, ie sensor_pair
            // Get complement's row's eigenray_list
            eigenray_list* list = _eigenray_collection->eigenrays(row, 0);
             // Send out eigenray_list to listener
            listener->update_fathometer(_sensorID, list);
        }
	}
}

/**
 * Last set of eigenverbs computed for this sensor.
 */
eigenverb_collection::reference sensor_model::eigenverbs() const {
	read_lock_guard guard(_update_eigenverbs_mutex);
	return _eigenverbs;
}

/**
 * Asynchronous update of eigenverbs data from the wavefront task.
 * Passes this data onto all sensor listeners.
 * Blocks until update is complete.
 */
void sensor_model::update_eigenverbs( eigenverb_collection::reference& eigenverbs ) {
	write_lock_guard guard(_update_eigenverbs_mutex);
	#ifdef USML_DEBUG
		cout << "sensor_model: update_eigenverbs(" << _sensorID << ")" << endl ;
	#endif
	_eigenverbs = eigenverbs;
	BOOST_FOREACH( sensor_listener* listener, _sensor_listeners ) {
		listener->update_eigenverbs(this);
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
 * to require a new WaveQ3D run.
 *
 * @todo using dummy values for prototyping
 */
bool sensor_model::check_thresholds(const wposition1& position,
		const orientation& orientation)
{
	// force update if old values not valid

	if (isnan(_position.rho()) || isnan(_position.theta())
			|| isnan(_position.phi()) || isnan(_orient.heading())
			|| isnan(_orient.pitch()) || isnan(_orient.roll())) {
		return true;
	}

	// check difference between old and new values

	return true;// using dummy values for prototyping
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
wposition sensor_model::target_positions(std::list<const sensor_model*>& list) {

	// builds wposition container of target positions from the list provided.

	wposition target_pos(list.size(), 1);
	int row = -1;
	BOOST_FOREACH( const sensor_model* target, list ){
		++row;
		wposition1 pos = target->position();
		target_pos.latitude( row, 0, pos.latitude());
		target_pos.longitude(row, 0, pos.longitude());
		target_pos.altitude( row, 0, pos.altitude());
	}
	return target_pos ;
}

/**
 * Utility to select frequencies band from sensor including min and max active frequencies
 */
void sensor_model::select_frequencies() {

    double min; 
    double max;

    double band_min;
    double band_max;    
    size_t band_size;

    switch ( _mode ) {

        case usml::sensors::RECEIVER:
        {
            min = _receiver->min_active_freq();
            max = _receiver->max_active_freq();

            band_min = *( _receiver->frequencies()->begin() );
            band_max = *( _receiver->frequencies()->end() - 1 );
            band_size = _receiver->frequencies()->size();
            break;
        }
        default:  // SOURCE or BOTH
        {
            min = _source->min_active_freq();
            max = _source->max_active_freq();

            band_min = *( _source->frequencies()->begin() );
            band_max = *( _source->frequencies()->end() - 1 );
            band_size = _source->frequencies()->size();
        }
    }

    // Set min/max 
    if ( band_min  < min ) {
        min = band_min;
    }

    if ( band_max  > max ) {
        max = band_max;
    }
    
    _frequencies.reset(new seq_linear(min, (max-min)/band_size, max));
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

        // Get the targets sensor references
        std::list<const sensor_model*> targets = sensor_targets();

        // Store the targetID's for later use in sending on to sensor_pairs
        target_ids(targets);

        // Get the target positions for wavefront_generator
        wposition target_pos = target_positions(targets);

        // Create the wavefront_generator
        wavefront_generator* generator = new wavefront_generator(
            ocean_shared::current(), _position, target_pos, frequencies(), this);

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
