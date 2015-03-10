/**
 *  @file sensor.cc
 *  Implementation of the Class sensor
 *  Created on: 10-Feb-2015 12:49:09 PM
 */

#include "sensor.h"

using namespace usml::sensors;

/**
 * Constructor
 * 
 * @param sensorID
 * @param paramsID
 * @param mode
 * @param position
 * @param tilt_angle
 * @param tilt_direction
 * @param pitch
 * @param yaw
 * @param description
 */
sensor::sensor(const sensorIDType sensorID, const paramsIDType paramsID, const xmitRcvModeType mode,
	const wposition1 position, const double tilt_angle, const double tilt_direction, 
	const double pitch, const double yaw, const std::string description) 
	:	_sensorID(sensorID),
		_paramsID(paramsID),
		_src_rcv_mode(mode),
		_position(position),
		_tilt_angle(tilt_angle),
		_tilt_direction(tilt_direction),
		_pitch(pitch),
		_yaw(yaw),
		_description(description),
		_fathometers(NULL),
		_eigenverbs(NULL)
{
    // Use _paramsID to lookup source and receiver from map's
    // Make a copy of what was obtained from the associated map.
    // call to source() or receiver can replace.
    // Ownership of _source and/or _receiver starts here.

    source_params_map* source_map = source_params_map::instance();
    receiver_params_map* receiver_map = receiver_params_map::instance();

    switch (mode)
    {
        default:
            assert(false);
            _source = NULL;
            _receiver = NULL;
            break;
        case usml::sensors::SOURCE:
        {
            const source_params* sp = source_map->find(_paramsID);
            if (sp == NULL) {
                _source = new source_params(*sp);
            } else {
                _source = NULL;
            }
            break;
        }
        case usml::sensors::RECEIVER:
        {
            const receiver_params* rp = receiver_map->find(_paramsID);
            if (rp == NULL) {
                _receiver = new receiver_params(*rp);
            } else {
                _receiver = NULL;
            }
            break;
        }
        case usml::sensors::BOTH:
        {
            const source_params* sp = source_map->find(_paramsID);
            if (sp == NULL) {
                _source = new source_params(*sp);
            } else {
                _source = NULL;
            }
            const receiver_params* rp = receiver_map->find(_paramsID);
            if (rp == NULL) {
                _receiver = new receiver_params(*rp);
            } else {
                _receiver = NULL;
            }
        }
    }
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
 * Sets the latitude of the sensor. 
 * Expects latitude to be in decimal degrees.
 * @param latitude
 */
void sensor::latitude(double latitude)
{
    _position.latitude(latitude);
}

/**
 * Gets the latitude of the sensor.
 * @return latitude in decimal degrees.
 */
double sensor::latitude()
{
    return _position.latitude();
}

/**
 * Sets the longitude of the sensor. 
 * Expects longitude to be in decimal degrees.
 * @param longitude
 */
void sensor::longitude(double longitude)
{
    _position.longitude(longitude);
}

/**
 * Gets the longitude of the sensor.
 * @return longitude in decimal degrees.
 */
double sensor::longitude()
{
    return _position.longitude();
}

/**
 * Sets the depth of the sensor in meters.
 * Expects depth to be positive value from surface.
 * @param depth
 */
void sensor::depth(double depth)
{
    // Set wposition1.altitude
    _position.altitude(-depth);
}

/**
 * Gets the depth of the sensor.
 * @return depth in meters.
 */
double sensor::depth()
{
    return -(_position.altitude());
}

/**
 * Initialize the wave_generator thread  to start the waveq3d model.
 */
void sensor::init_wave_generator()
{

}

/**
 * Updates the sensor.
 */
void sensor::update_sensor()
{

}

/**
 * 
 * @param fathometers
 */
void sensor::update_fathometers(proploss* fathometers)
{

}

/**
 *
 * @param eigenverbs
 */
void sensor::update_eigenverbs(eigenverb_collection* eigenverbs)
{

}

/**
 * Add a sensor_listener to the _sensor_listener_vec vector
 * @param listener
 */
bool sensor::add_sensor_listener(sensor_listener* listener)
{  
    std::vector<sensor_listener*>::iterator iter = find(_sensor_listener_vec.begin(), _sensor_listener_vec.end(), listener);
    if ( iter != _sensor_listener_vec.end() )
    {
        return false;
    }   
    _sensor_listener_vec.push_back(listener);
    return true;  
}

/**
 * Remove a sensor_listener from the _sensor_listener_vec vector
 * @param listener
 */
bool sensor::remove_sensor_listener(sensor_listener* listener)
{
    std::vector<sensor_listener*>::iterator iter = find(_sensor_listener_vec.begin(), _sensor_listener_vec.end(), listener);
    if ( iter == _sensor_listener_vec.end() )
    {
        return false;
    }
    else
    {
        _sensor_listener_vec.erase(remove(_sensor_listener_vec.begin(), _sensor_listener_vec.end(), listener));
    }
    return true;
}

/**
 * For each sensor_listener in the _sensor_listener_vec vector
 * call the sensor_changed method with the sensorID.
 */
bool sensor::notify_sensor_listeners(sensorIDType sensorID)
{

    for ( std::vector<sensor_listener*>::iterator iter = _sensor_listener_vec.begin();
        iter != _sensor_listener_vec.end(); ++iter )
    {
        sensor_listener* pListener = *iter;
        pListener->sensor_changed(sensorID, _src_rcv_mode);
    }

    return ( _sensor_listener_vec.size() > 0 );
}
