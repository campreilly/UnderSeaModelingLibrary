/**
 *  @file sensor.h
 *  Definition of the Class sensor
 *  Created on: 10-Feb-2015 12:49:09 PM
 */

#pragma once

#include <list>

#include <usml/types/wposition1.h>
#include <usml/waveq3d/proploss.h>
#include <usml/waveq3d/wave_queue.h>
#include <usml/sensors/sensorIDType.h>
#include <usml/sensors/paramsIDType.h>
#include <usml/sensors/xmitRcvModeType.h>

#include <usml/sensors/source_params.h>
#include <usml/sensors/receiver_params.h>
#include <usml/sensors/source_params_map.h>
#include <usml/sensors/receiver_params_map.h>
#include <usml/sensors/sensor_listener.h>
#include <usml/eigenverb/envelope_collection.h>
#include <usml/eigenverb/eigenverb_collection.h>


namespace usml {
namespace sensors {

using namespace usml::eigenverb ;
using namespace usml::waveq3d ;

class sensor_listener;

/// @ingroup sensors
/// @{

/**
 * All active sensors in a simulation are represented by an instance of this class.
 * As the sensor moves all required attributes are updated. If the attributes
 * change beyond established thresholds a new reverb generation is started.
 *
 * @author Ted Burns, AEgis Technologies Group, Inc.
 * @version 1.0
 * @updated 18-Mar-2015 12:18:44 PM
 */
class USML_DECLSPEC sensor
{
public:

	/**
	 * Default Constructor
	 */
	sensor() :
	    _sensorID(-1),
        _paramsID(-1),
        _src_rcv_mode(usml::sensors::SOURCE),
        _position(wposition1(0.0, 0.0, 0.0)),
        _tilt_angle(0.0),
        _tilt_direction(0.0),
        _pitch(0.0),
        _yaw(0.0),
        _source(NULL),
        _receiver(NULL),
        _fathometers(NULL),
        _eigenverbs(NULL){}

	/**
	 * Constructor
	 * Uses the paramID and mode, looks up source and/or receiver from there associated map.
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
	sensor(const sensorIDType sensorID, const paramsIDType paramsID, const xmitRcvModeType mode,
				const wposition1 position, const double tilt_angle, const double tilt_direction, 
				const double pitch, const double yaw, const std::string description = std::string());

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
	 * Set method for the _src_rcv_mode attribute.
	 * The mode defines if the sensor is a source, or receive or both.
	 * @param mode of the xmitRcvModeType.
	 */
	void mode(xmitRcvModeType mode)
	{
		_src_rcv_mode = mode;
	}

	/**
	 * Get method for the _src_rcv_mode attribute.
	 * @return _src_rcv_mode of the xmitRcvModeType
	 */
	xmitRcvModeType mode()
	{
		return _src_rcv_mode;
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
	 * Sets the source_params of the sensor by
	 * making a deep copy of the data.
	 * @param src_params source_params reference
     */
    void source(const source_params& src_params)
    {
        // Assumes constructor populated
        if (_source != NULL) {
            delete _source;
        }
		_source = new source_params(src_params);
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
	 * Sets the receiver_params of the sensor by
     * making a deep copy of the data.
	 * @param rcv_params receiver_params reference
	 */
	void receiver(const receiver_params& rcv_params)
	{
	    if (_receiver != NULL ) {
	        delete _receiver;
	    }
		_receiver = new receiver_params(rcv_params);
	}

	/**
	 * Gets the receiver_params of the sensor.
	 * @return receiver_params pointer.
	 */
	receiver_params* receiver()
	{
		return _receiver;
	}

	/**
	 * Initialize the wave_generator thread  to start the waveq3d model.
	 */
	void init_wave_generator();
	
	/**
	 * Updates the sensor data, checks position, pitch, yaw, thresholds
	 * to determine if new wave_generator needs to be run, then kicks 
	 * off the waveq3d model.
	 * @param position  updated position data
	 * @param pitch     updated pitch value
	 * @param yaw       updated yaw value
	 * @param force_run defaults to false, set true to force new run
	 */
	void update_sensor(wposition1 position, double pitch, double yaw, bool force_update=false);
	
	/**
	 * Updates the sensors fathometers
	 * @param fathometers
	 */
	void update_fathometers(proploss* fathometers);

    /**
     * Updates the sensors eigenverb_collection
     * @param eigenverbs
     */
    void update_eigenverbs(eigenverb_collection* eigenverbs);

	/**
     * Add a sensor_listener to the _sensor_listener_vec vector
	 * @param listener
	 */
	bool add_sensor_listener(sensor_listener* listener);

	/**
	 * Remove a sensor_listener to the _sensor_listener_vec vector
	 * @param listener
	 */
	bool remove_sensor_listener(sensor_listener* listener);
		
    /**
    * For each sensor_listener in the _sensor_listener_vec vector
    * call the sensor_changed method of each registered class.
    */
    bool notify_sensor_listeners(sensorIDType sensorID);

private:

    sensorIDType _sensorID;
    paramsIDType _paramsID;
    xmitRcvModeType _src_rcv_mode;

	wposition1 _position;
	double _pitch;
    double _yaw;
	
	source_params* _source;
    receiver_params* _receiver;
  
    proploss* _fathometers;
	eigenverb_collection* _eigenverbs;
	std::string    _description;	

    /**
    * Vector containing the references of objects that will be used to
    * update classes that require sensor be informed as they are changed.
    * These classes must implement sensor_changed method.
    */
    std::vector<sensor_listener*> _sensor_listener_vec; 		
};

/// @}
}  // end of namespace sensors
}  // end of namespace usml
