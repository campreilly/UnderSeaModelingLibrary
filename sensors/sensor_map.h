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
#include <usml/sensors/sensor_map_template.h>
#include <usml/sensors/sensor_pair_manager.h>
#include <usml/threads/read_write_lock.h>

namespace usml {
namespace sensors {

using namespace threads;

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
class USML_DECLSPEC sensor_map : public sensor_map_template <const sensorIDType, sensor*>
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

    /**
     * Inserts the supplied sensor pointer into the sensor_map and the
     * sensor_pair_manager map with the sensorID provided.
     * @param sensorID is the associated key to the sensor pointer.
     * @param in_sensor is sensor pointer to be inserted.
     * @return false if sensorID was already in the map.
     */
    bool insert(const sensorIDType sensorID, sensor* in_sensor);

    /**
     * Deletes the sensor pointer from the heap and the associated sensorID
     * from the map. Also deletes the sensor from the sensor_pair_manager
     * @param sensorID is the associated key to the map.
     * @param mode is sensor pointer to be inserted.
     * @return false if sensorID was not in the map or the sensor_pair_manager.
     */
    bool erase(const sensorIDType sensorID, xmitRcvModeType mode);

    /**
     * Updates a pre-existing the sensor and the associated sensorID
     * in the the map. Also updates the sensor in the sensor_pair_manager
     * @param sensorID is the associated key to the map.
     * @return false if sensorID was not in the map or the sensor_pair_manager.
     */
    bool update(const sensorIDType sensorID, sensor* in_sensor);

private:

	/**
     * Default Constructor
     *   Prevent creation/access other than static instance()
     */
    sensor_map()
    {
        _sensor_pair_manager = sensor_pair_manager::instance();
    }

    /**
     * Destructor - See map_template destructor.
     *  Prevent use of delete, use static destroy above.
     */
    virtual ~sensor_map()
    {
        sensor_pair_manager::destroy();
    }

	/**
     * Prevent access to copy constructor
     */
	sensor_map(sensor_map const&);

    /**
     * Prevent access to assignment operator
     */
	sensor_map& operator=(sensor_map const&);

	/**
     * The sensor_pair_manager singleton.
     */
    sensor_pair_manager* _sensor_pair_manager;

	/**
     * The singleton access pointer.
     */
    static sensor_map* _instance;

    /**
     * The _mutex for the singleton pointer.
     */
    static read_write_lock _mutex ;
};

/// @}
} // end of namespace sensors
} // end of namespace usml
