/**
 *  @file sensor.cc
 *  Implementation of the Class sensor
 *  Created on: 10-Feb-2015 12:49:09 PM
 */
#include <usml/sensors/sensor.h>
#include <usml/sensors/source_params_map.h>
#include <usml/sensors/receiver_params_map.h>

using namespace usml::sensors;

/**
 * Construct a new instance of a specific sensor type.
 */
sensor::sensor( sensor::id_type sensorID, sensor_params::id_type paramsID,
		const std::string& description) :
		_sensorID(sensorID), _paramsID(paramsID), _description(description),
		_position(NAN,NAN,NAN), _orientation(NAN,NAN,NAN)
{
	_source = source_params_map::instance()->find(paramsID) ;
	_receiver = receiver_params_map::instance()->find(paramsID) ;
}

/**
 * Queries the sensor's ability to support source and/or receiver behaviors.
 */
xmitRcvModeType sensor::mode() const {
	bool has_source = _source.get() != NULL ;
	bool has_receiver = _receiver.get() != NULL ;
	xmitRcvModeType result ;
		if ( has_source && has_receiver ) {
			result = BOTH ;
		} else if ( has_source ) {
			result = SOURCE ;
		} else if ( has_receiver ) {
			result = RECEIVER ;
		} else {
			result = NONE ;
		}
	return result ;
}

/**
 * Checks to see if new position and orientation have changed enough
 * to require a new WaveQ3D run.
 *
 * @todo using dummy values for prototyping
 */
bool sensor::check_thresholds(wposition1 position,
		const sensor_orientation& orientation) {

	// force update if old values not valid

	if ( isnan(_position.rho()) || isnan(_position.theta())
  	  || isnan(_position.phi()) || isnan(_orientation.heading())
	  || isnan(_orientation.pitch()) || isnan(_orientation.roll()))
	{
		return true;
	}

	// check difference between old and new values

	return true;	// using dummy values for prototyping
}

/**
 * Initialize the wave_generator thread to start the waveq3d model.
 */
void sensor::init_wave_generator()
{
    int runID = 1;

    // Get the targets

    //wposition targets = sensor_pair_manager::instance()->get_targets(_src_rcv_mode);

    // Create the wavefront_generator
    wavefront_generator* generator = new wavefront_generator();

    generator->runID(runID);
    generator->wavefront_listener(this);
    generator->sensor_position(_position);
    //generator->targets(targets);
    generator->frequencies(new seq_log( 10e3, 1.0, 1 ));
    generator->ocean(ocean_shared::current());

    // Make wavefront_generator a wavefront_task, with use of shared_ptr
    _wavefront_task = thread_task::reference(generator);

    // Pass in to thread_pool
    thread_controller::instance()->run(_wavefront_task);
}

/**
 * Checks to see if new position and orientation have changed enough
 * to require a new WaveQ3D run.
 */
void sensor::update_sensor(const wposition1& position,
		const sensor_orientation& orientation, bool force_update) {
	write_lock_guard guard(_update_sensor_mutex) ;
	if (!force_update) {
		if (!check_thresholds(position, orientation)) {
			return ;
		}
	}
	_position = position;
	_orientation = orientation;
	init_wave_generator();
}

/**
 * Add a sensor_listener to the _sensor_listeners list
 * @param listener
 */
bool sensor::add_sensor_listener(sensor_listener* listener)
{  
    std::list<sensor_listener*>::iterator iter = find(_sensor_listeners.begin(), _sensor_listeners.end(), listener);
    if ( iter != _sensor_listeners.end() )
    {
        return false;
    }   
    _sensor_listeners.push_back(listener);
    return true;  
}

/**
 * Remove a sensor_listener from the _sensor_listeners list
 * @param listener
 */
bool sensor::remove_sensor_listener(sensor_listener* listener)
{
    std::list<sensor_listener*>::iterator iter = find(_sensor_listeners.begin(), _sensor_listeners.end(), listener);
    if ( iter == _sensor_listeners.end() )
    {
        return false;
    }
    else
    {
        _sensor_listeners.erase(remove(_sensor_listeners.begin(), _sensor_listeners.end(), listener));
    }
    return true;
}

/**
 * For each sensor_listener in the _sensor_listeners list call the
 * update_eigenverbs method of each registered class.
 */
bool sensor::update_eigenverb_listeners()
{
    for ( std::list<sensor_listener*>::iterator iter = _sensor_listeners.begin();
        iter != _sensor_listeners.end(); ++iter )
    {
        sensor_listener* sensor_pair_ = *iter;
        sensor_pair_->update_eigenverbs(this);
    }

    return ( _sensor_listeners.size() > 0 );
}

/**
 * For each sensor_listener in the _sensor_listeners list call the
 * update_fathometers method of each registered class.
 */
bool sensor::update_fathometer_listeners()
{
    for ( std::list<sensor_listener*>::iterator iter = _sensor_listeners.begin();
        iter != _sensor_listeners.end(); ++iter )
    {
        sensor_listener* listener = *iter;
        listener->update_fathometers(this);
    }

    return ( _sensor_listeners.size() > 0 );
}
