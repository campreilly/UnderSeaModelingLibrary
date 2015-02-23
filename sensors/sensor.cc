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
 * @param xmitRcvMode
 * @param position
 * @param tilt_angle
 * @param tilt_direction
 * @param pitch
 * @param yaw
 * @param description
 */
sensor::sensor(const sensorIDType sensorID, const paramsIDType paramsID, const xmitRcvModeType xmitRcvMode, 
	const wposition1 position, const double tilt_angle, const double tilt_direction, 
	const double pitch, const double yaw, const std::string description) 
	:	_sensorID(sensorID),
		_paramsID(paramsID),
		_xmitRcvMode(xmitRcvMode),
		_position(position),
		_tilt_angle(tilt_angle),
		_tilt_direction(tilt_direction),
		_pitch(pitch),
		_yaw(yaw),
		_description(description)
{

}

/**
 * Default Constructor
 */
sensor::sensor()
{

}

/**
 * Destructor
 */
sensor::~sensor()
{

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
double sensor::latitude(){

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
 * @param longitude
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
 * Updates the sensor.
 */
void sensor::update()
{

}

/**
 * Run the waveq3d model.
 */
void sensor::run_waveq3d()
{

}

std::list<int>* sensor::get_envelope()
{
	return  NULL;
}

std::list<int>* sensor::get_discrete_envelope()
{

	return  NULL;
}