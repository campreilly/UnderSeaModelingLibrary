/**
 *  @file sensor.cc
 *  Implementation of the Class sensor
 *  Created on: 10-Feb-2015 12:49:09 PM
 */

#include "sensor.h"

using namespace usml::sensors;

/**
 * Constructor Uses the paramID and mode, looks up source and/or receiver from
 * there associated map.
 * 
 * @param sensorID
 * @param paramsID
 * @param mode
 * @param position
 * @param pitch
 * @param yaw
 * @param description
 */
sensor::sensor(const id_type sensorID, const paramsIDType paramsID, const xmitRcvModeType mode,
	const wposition1 position, const double pitch, const double yaw, const std::string description) 
	:	_sensorID(sensorID),
		_paramsID(paramsID),
		_src_rcv_mode(mode),
		_position(position),
		_pitch(pitch),
		_yaw(yaw),
		_roll(0.0),
		_source(NULL),
		_receiver(NULL),
		_fathometers(NULL),
		_eigenverbs(NULL),
        _description(description)
{
    // Use _paramsID to lookup source and receiver from map's
    // Make a copy of what was obtained from the associated map.
    // call to source() or receiver can replace.
    // Ownership of _source and/or _receiver starts here.

///@todo

//    source_params_map* source_map = source_params_map::instance();
//    receiver_params_map* receiver_map = receiver_params_map::instance();
//
//    switch (mode)
//    {
//        default:
//            assert(false);
//            _source = NULL;
//            _receiver = NULL;
//            break;
//        case usml::sensors::SOURCE:
//        {
//            source_params::reference sp = source_map->find(_paramsID);
//            if (sp == NULL) {
//                _source = new source_params(*sp);
//            } else {
//                _source = NULL;
//            }
//            break;
//        }
//        case usml::sensors::RECEIVER:
//        {
//            const receiver_params* rp = receiver_map->find(_paramsID);
//            if (rp == NULL) {
//                _receiver = new receiver_params(*rp);
//            } else {
//                _receiver = NULL;
//            }
//            break;
//        }
//        case usml::sensors::BOTH:
//        {
//            const source_params* sp = source_map->find(_paramsID);
//            if (sp == NULL) {
//                _source = new source_params(*sp);
//            } else {
//                _source = NULL;
//            }
//            const receiver_params* rp = receiver_map->find(_paramsID);
//            if (rp == NULL) {
//                _receiver = new receiver_params(*rp);
//            } else {
//                _receiver = NULL;
//            }
//        }
//    }
}

/**
 * Destructor
 */
sensor::~sensor()
{
    if (_source != NULL) {
        delete _source;
    }
    if (_receiver != NULL){
        delete _receiver;
    }
}

/**
 * Sets the latitude of the sensor. Expects latitude to be in decimal degrees.
 * @param latitude    latitude
 */
void sensor::latitude(double latitude)
{
    _position.latitude(latitude);
}

/**
 * Get method for the latitude of the sensor.
 * @return latitude in decimal degrees.
 */
double sensor::latitude()
{
    return _position.latitude();
}

/**
 * Sets the longitude of the sensor. Expects longitude to be in decimal degrees.
 * @param longitude    longitude
 */
void sensor::longitude(double longitude)
{
    _position.longitude(longitude);
}

/**
 * Get method for the longitude of the sensor.
 * @return longitude in decimal degrees.
 */
double sensor::longitude()
{
    return _position.longitude();
}

/**
 * Sets the depth of the sensor. Expects depth to be in meters.
 * @param depth    depth
 */
void sensor::depth(double depth)
{
    // Set wposition1.altitude
    _position.altitude(-depth);
}

/**
 * Get method for the depth of the sensor.
 * @return depth of the sensor in meters.
 */
double sensor::depth()
{
    return -(_position.altitude());
}

/**
 * Checks to see if new position, pitch and yaw have changed enough
 * to require a new WaveQ3D run.
 */
bool sensor::check_thresholds(wposition1 position, double pitch, double yaw)
{
    return false;

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
 * Updates the sensor data, checks position, pitch, yaw, thresholds to determine
 * if new wave_generator needs to be run, then kicks off the waveq3d model.
 * @param force_run defaults to false, set true to force new run
 * 
 * @param position    updated position data
 * @param pitch    updated pitch value
 * @param yaw    updated yaw value
 * @param force_update
 */
void sensor::update_sensor(wposition1 position, double pitch, double yaw, bool force_run)
{
	if ( !force_run )
	{
	     if (!check_thresholds(position, pitch, yaw)) {
            return;
        }
	}
	_position = position;
	_pitch = pitch;
	_yaw = yaw;
	init_wave_generator();

	return;
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
