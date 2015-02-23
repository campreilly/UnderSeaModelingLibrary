/**
 *  @file sensor.h
 *  Definition of the Class sensor
 *  Created on: 10-Feb-2015 12:49:09 PM
 */

#pragma once

#include <list>

#include <usml/types/wposition1.h>
#include <usml/sensors/sensorIDType.h>
#include <usml/sensors/paramsIDType.h>
#include <usml/sensors/xmitRcvModeType.h>

#include <usml/sensors/source_params.h>
#include <usml/sensors/receiver_params.h>

#include <usml/waveq3d/wave_queue.h>

namespace usml {
namespace sensors {

using namespace usml::waveq3d;

/// @ingroup sensors
/// @{

/**
 * All active sensors in a simulation are represented by an instance of this class.
 * As the sensor moves all required attributes are updated. If the attributes change beyond 
 * established thresholds a new reverb generation is started.
 *
 * @author Ted Burns, AEgis Technologies Inc.
 * @version 1.0
 * @updated 23-Feb-2015 3:47:23 PM
 */
class USML_DECLSPEC sensor
{

public:
	/**
	 * Constructor
	 */
	sensor();
	/**
	 * Constructor
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
	sensor(const sensorIDType sensorID, const paramsIDType paramsID, const xmitRcvModeType xmitRcvMode, 
				const wposition1 position, const double tilt_angle, const double tilt_direction, 
				const double pitch, const double yaw, const std::string description = NULL);

    /**
	 * Destructor
	 */
	virtual ~sensor();
	/**
	 * Set method for the sensorID attribute. The sensorID attribute is used as the
	 * key to lookup sensors in the sensor_map.
	 * 
	 * @param sensorID of the sensorIDType
	 */
	void sensorID(sensorIDType sensorID)
	{
		_sensorID = sensorID;
	}
	/**
	 * Get method for the sensorID attribute.
	 * @return sensorID of the sensorIDType
	 */
	sensorIDType sensorID()
	{
		return _sensorID;
	}

	/**
	 * Set method for the paramsID attribute. The paramsID attribute is used as the
	 * key to lookup the source or receiver parameters in the source_params_map and
	 * receiver_params_map.
	 * @param paramsID of the paramsIDType.
	 */
	void paramsID(paramsIDType paramsID)
	{
		_paramsID = paramsID;
	}

	/**
	 * Get method for the paramsID attribute.
	 * @return paramsID of the paramsIDType
	 */
	paramsIDType paramsID()
	{
		return _paramsID;
	}

	/**
	 * Set method for the xmitRcvMode attribute. 
	 * The xmitRcvMode defines if the sensor is a source, or receive or both.
	 * @param xmitRcvMode of the xmitRcvModeType.
	 */
	void xmitRcvMode(xmitRcvModeType xmitRcvMode)
	{
		_xmitRcvMode = xmitRcvMode;
	}

	/**
	 * Get method for the xmitRcvMode attribute.
	 * @return xmitRcvMode of the xmitRcvModeType
	 */
	xmitRcvModeType xmitRcvMode()
	{
		return _xmitRcvMode;
	}

	/**
	 * Sets the position of the sensor. 
	 * Expects a wposition1 type.
	 * @param position of wposition1 type.
	 */
	void position(wposition1 position)
	{
		_position = position;
	}

	/**
	 * Gets the position of the sensor
	 * @return position of type wposition1
	 */
	wposition1 position()
	{
		return _position;
	}
	
	/**
	 * Sets the latitude of the sensor. 
	 * Expects latitude to be in decimal degrees.
	 * @param latitude
	 */
	void latitude(double latitude);
	
	/**
	 * Get method for the latitude of the sensor.
	 * @return latitude in decimal degrees.
	 */
	double latitude();

	
	/**
	 * Sets the longitude of the sensor. 
	 * Expects longitude to be in decimal degrees.
	 * @param longitude
	 */
	void longitude(double longitude);
	
	/**
	 * Get method for the longitude of the sensor.
	 * @return longitude in decimal degrees.
	 */
	double longitude();
	
	/**
	 * Sets the depth of the sensor.
     * Expects depth to be in meters.	 
	 * @param depth
	 */
	void depth(double depth);
	
	/**
	 * Get method for the depth of the sensor.
	 * @return depth of the sensor in meters.
	 */
	double depth();
	
	/**
	 * Sets the tilt_angle attribute of the sensor. 
	 * Expects tilt_angle to be in radians.
	 * @param tilt_angle
	 */
	void tilt_angle(double tilt_angle)
	{
		_tilt_angle = tilt_angle;
	}
	/**
	 * Gets the tilt_angle of the sensor.
	 * @return tilt_angle in radians.
	 */
	double tilt_angle()
	{
		return _tilt_angle;
	}
	/**
	 * Sets the tilt_direction attribute of the sensor. 
	 * @param tilt_direction in radians.
	 */
	void tilt_direction(double tilt_direction)
	{
		_tilt_direction = tilt_direction;
	}
	/**
	 * Gets the tilt_direction of the sensor.
	 * @return tilt_direction in radians.
	 */
	double tilt_direction()
	{
		return _tilt_direction;
	}
	/**
	 * Sets the pitch of the sensor. 
	 * Expects pitch to be in radians.
	 * @param pitch
	 */
	void pitch(double pitch)
	{
		_pitch = pitch;
	}
	/**
	 * Gets the pitch of the sensor.
	 * @return pitch in radians.
	 */
	double pitch()
	{
		return _pitch;
	}

	/**
	 * Sets the yaw of the sensor. 
	 * Expects yaw to be in radians.
	 * @param yaw
	 */
	void yaw(double yaw)
	{
		_yaw = yaw;
	}

	/**
	 * Gets the yaw of the sensor.
	 * @return yaw in radians.
	 */
	double yaw()
	{
		return _yaw;
	}
	
	/**
	 * Sets the source_params of the sensor. 
	 * @param source_params pointer
     */
    void source(source_params* source_params)
    {
		_source = source_params;
	}

	/**
	 * Gets the source_params of the sensor.
	 * @return source_params pointer.
	 */
	source_params* source()
	{
		return _source;
	}

	/**
	 * Sets the receiver_params of the sensor. 
	 * @param receiver_params pointer
	 */
	void receiver(receiver_params* receiver_params)
	{
		_receiver = receiver_params;
	}

	/**
	 * Gets the receiver_params of the sensor.
	 * @return receiver_params pointer.
	 */
	receiver_params* receiver()
	{
		return _receiver;
	}
	
	void update();
	void run_waveq3d();
	std::list<int>* get_envelope();
	std::list<int>* get_discrete_envelope();
			

private:
    sensorIDType _sensorID;
    paramsIDType _paramsID;
    xmitRcvModeType _xmitRcvMode;

	wposition1 _position;
	double _tilt_angle;
	double _tilt_direction;	
	double _pitch;
    double _yaw;
	
	source_params* _source;
    receiver_params* _receiver;
  
	std::list<int> _eigenverbs;
	wave_queue*    _wave;
	std::string    _description;			
};

/// @}
}  // end of namespace sensors
}  // end of namespace usml
