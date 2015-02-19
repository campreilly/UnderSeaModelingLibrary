/**
 *  @file sensor_map.h
 *  Definition of the Class sensor_map
 *  Created on: 12-Feb-2015 3:41:30 PM
 */

#pragma once

#include <usml/usml_config.h>
#include <usml/sensors/singleton_map.h>
#include <usml/sensors/sensorIDType.h>
#include <usml/sensors/sensor.h>
#include <usml/sensors/source_params_map.h>
#include <usml/sensors/receiver_params_map.h>

namespace usml {
namespace sensors {

/// @ingroup sensors
/// @{

/**
 * Storage for all the sensor's in use by the USML.
 * This class inherits from the templated singleton_map class.
 * The map stores pointers to sensor's and take's
 * ownership of the pointers. See usml/sensors/singleton_map.h
 * A typedef of sensorIDType has been defined to allow for
 * modification of the key of the map at a later time if needed.
 *
 * @author Ted Burns, AEgis Technologies Inc.
 * @version 1.0
 * @created 12-Feb-2015 3:41:30 PM
 */
class USML_DECLSPEC sensor_map : public singleton_map < const sensorIDType, sensor* >
{
public:
    /**
     * Destructor - See singleton_map destructor.
     */
	virtual ~sensor_map();

	/**
	 * 
	 * @param sensorID
	 */
	void erase(const sensorIDType sensorID);

	/**
	 * 
	 * @param sensorID
	 * @param sensor
	 */
	void update(const sensorIDType sensorID, const sensor* sensor);

protected:
    /**
     * Default Constructor
     *   Protected to prevent access other than instance call
     */
	sensor_map();

private:
	/**
     * Prevent access to copy constructor
     */
	sensor_map(sensor_map const&);

    /**
     * Prevent access to assignment operator
     */
	sensor_map& operator=(sensor_map const&);

	/**
     * Access to source_params
     */
	source_params_map* _source_params_map;

	/**
     * Access to reciever_params
     */
	receiver_params_map* _receiver_params_map;

};

/// @}
} // end of namespace sensors
} // end of namespace usml
