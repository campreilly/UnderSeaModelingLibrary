/**
 *  @file sensor_map.h
 *  Definition of the Class sensor_map
 *  Created on: 12-Feb-2015 3:41:30 PM
 */

#pragma once

#include <usml/usml_config.h>

#include <usml/sensors/sensor.h>
#include <usml/sensors/sensorIDType.h>
#include <usml/sensors/source_params_map.h>
#include <usml/sensors/receiver_params_map.h>
#include <usml/sensors/map_template.h>

namespace usml {
namespace sensors {

/// @ingroup sensors
/// @{

/**
 * Container for all the sensor's in use by the USML. This class inherits from
 * the map_template class. This class implements the singleton GOF pattern.
 * The map stores pointers to sensor's and take's ownership of the pointers.
 * See usml/sensors/map_template.h A typedef of sensorIDType has been defined
 * to allow for modification of the key of the map at a later time if needed.
 *
 * @author Ted Burns, AEgis Technologies Group, Inc.
 * @version 1.0
 * @updated 27-Feb-2015 3:15:03 PM
 */
class USML_DECLSPEC sensor_map : public map_template <const sensorIDType, sensor*>
{
public:

    /**
     * Singleton Constructor - Creates sensor_map instance just once.
     * Accessible everywhere.
     * @return  pointer to the instance of the singleton sensor_map
     */
    static sensor_map* instance();

    /**
     * Singleton Destructor - Deletes source_params_map instance
     * Accessible everywhere.
     */
    static void destroy();

private:

	/**
     * Default Constructor
     *   Prevent creation/access other than static instance()
     */
    sensor_map() {}

    /**
     * Destructor - See map_template destructor.
     *  Prevent use of delete, use static destroy above.
     */
    virtual ~sensor_map() {}

	/**
     * Prevent access to copy constructor
     */
	sensor_map(sensor_map const&);

    /**
     * Prevent access to assignment operator
     */
	sensor_map& operator=(sensor_map const&);

	/**
     * The singleton access pointer.
     */
    static sensor_map* _instance;
};

/// @}
} // end of namespace sensors
} // end of namespace usml
