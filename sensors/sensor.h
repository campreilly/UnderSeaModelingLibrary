/**
 *  @file sensor.h
 *  Definition of the Class sensor
 *  Created on: 10-Feb-2015 12:49:09 PM
 */

#pragma once

#include <list>

#include <usml/threads/thread_controller.h>
#include <usml/waveq3d/wave_queue.h>
#include <usml/sensors/paramsIDType.h>
#include <usml/sensors/xmitRcvModeType.h>

#include <usml/sensors/source_params.h>
#include <usml/sensors/receiver_params.h>
#include <usml/sensors/source_params_map.h>
#include <usml/sensors/receiver_params_map.h>
#include <usml/sensors/sensor_listener.h>
#include <usml/eigenverb/eigenverb_collection.h>
#include <usml/eigenverb/wavefront_generator.h>
#include <usml/eigenverb/wavefront_listener.h>


namespace usml {
namespace sensors {

using namespace usml::waveq3d ;
using namespace usml::eigenverb ;

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
class USML_DECLSPEC sensor : public wavefront_listener
{
public:

    /**
     * Data type used for beamId.
     */
    typedef int id_type;

	/**
	 * Constructor
	 * Uses the paramID and mode, looks up source and/or receiver from there associated map.
	 * @param sensorID
	 * @param paramsID
	 * @param mode
	 * @param position
	 * @param pitch
	 * @param yaw
	 * @param description
	 */
	sensor(const id_type sensorID, const paramsIDType paramsID, const xmitRcvModeType mode,
				const wposition1 position, const double pitch, const double yaw, const double roll,
                                                const std::string description = std::string());

    /**
	 * Destructor
	 */
	virtual ~sensor();

	/**
	 * Get method for the sensorID attribute.
	 * @return sensorID of the id_type
	 */
	id_type sensorID()
	{
		return _sensorID;
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
     * Checks to see if new position, pitch and yaw have changed enough
     * to require a new WaveQ3D run.
     * @param position  updated position data
     * @param pitch     updated pitch value
     * @param yaw       updated yaw value
     * @return true when thresholds exceeded, requiring a rerun of the model for this sensor.
     */
    bool check_thresholds(wposition1 position, double pitch, double yaw);

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
	 * Gets the sensor fathometers, overload of the pure virtual method
	 * fathometers for the sensor_pair_listener.
	 * @param fathometers
	 */
	proploss* fathometers() {
	    return _fathometers;
	}

	/**
     * update_fathometers
     *  Overloaded wavefront_listener method to update the eigenrays for the object that implements it.
     *  @param  fathometers - Pointer to a proploss object which contains eigenrays
     */
    virtual void update_fathometers(proploss* fathometers) {
        _fathometers = fathometers;
        update_fathometer_listeners();
    }

    /**
     * Gets the sensor eigenverb_collection, overloads the pure virtual method
     * eigenverbs for the sensor_pair_listener
     * @param eigenverbs
     */
    eigenverb_collection* eigenverbs() {
        return _eigenverbs;
    }

    /**
     * update_eigenverbs
     * Overloaded wavefront_listener method to update the eigenverb_collection for the object that implements it.
     *  @param  eigenverbs - Pointer to a eigenverb_collection object which contains eigenverbs
     */
    virtual void update_eigenverbs(eigenverb_collection* eigenverbs) {
        _eigenverbs = eigenverbs;
        update_eigenverb_listeners();
    }

	/**
     * Add a sensor_listener to the _sensor_listeners list
	 * @param listener
	 */
	bool add_sensor_listener(sensor_listener* listener);

	/**
	 * Remove a sensor_listener to the _sensor_listeners list
	 * @param listener
	 */
	bool remove_sensor_listener(sensor_listener* listener);
		

protected:

/**
 * Default Constructor - protected access
 */
sensor( )
    :   _sensorID(-1),
        _paramsID(-1),
        _src_rcv_mode(usml::sensors::SOURCE),
        _position(0.0,0.0,0.0),
        _pitch(0.0),
        _yaw(0.0),
        _roll(0.0),
        _source(NULL),
        _receiver(NULL),
        _fathometers(NULL),
        _eigenverbs(NULL),
        _description(NULL) {}

private:

    /**
     * For each sensor_listener in the _sensor_listeners list call the
     * update_eigenverbs method of each registered class.
     */
    bool update_eigenverb_listeners();

    /**
     * For each sensor_listener in the _sensor_listeners list call the
     * update_fathometers method of each registered class.
     */
    bool update_fathometer_listeners();


    id_type         _sensorID;
    paramsIDType    _paramsID;
    xmitRcvModeType _src_rcv_mode;

	wposition1 _position;
	double     _pitch;
    double     _yaw;
    double     _roll;
	
	source_params*   _source;
    receiver_params* _receiver;
  
    proploss*              _fathometers;
	eigenverb_collection*  _eigenverbs;
	thread_task::reference _wavefront_task;
	std::string            _description;
	
    /**
    * List containing the references of objects that will be used to
    * update classes that require sensor data.
    * These classes must implement sensor_changed method.
    */
    std::list<sensor_listener*> _sensor_listeners; 		
};

/// @}
}  // end of namespace sensors
}  // end of namespace usml
