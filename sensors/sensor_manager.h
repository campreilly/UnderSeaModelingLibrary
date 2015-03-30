/**
 *  @file sensor_manager.h
 *  Definition of the Class sensor_manager
 *  Created on: 12-Feb-2015 3:41:30 PM
 */

#pragma once

#include <usml/usml_config.h>

#include <usml/threads/threads.h>
#include <usml/sensors/sensor.h>
#include <usml/sensors/source_params_map.h>
#include <usml/sensors/receiver_params_map.h>
#include <usml/sensors/sensor_map_template.h>


namespace usml {
namespace sensors {

using namespace usml::threads;

class sensor;

/// @ingroup sensors
/// @{

/**
 * Container for all the sensor's in use by the USML. This class inherits from
 * the map_template class. This class implements the singleton GOF pattern.
 * The map stores pointers to sensor's and take's ownership of the pointers.
 * See usml/sensors/map_template.h A typedef of sensor::id_type has been defined
 * to allow for modification of the key of the map at a later time if needed.
 *
 * @author Ted Burns, AEgis Technologies Group, Inc.
 * @version 1.0
 * @updated 27-Feb-2015 3:15:03 PM
 */
class USML_DECLSPEC sensor_manager : public sensor_map_template<const sensor::id_type, sensor*>
{
public:

    /**
     * Singleton Constructor - Creates sensor_manager instance just once.
     * Accessible everywhere.
     * @return  pointer to the instance of the singleton sensor_manager
     */
    static sensor_manager* instance();

    /**
     * Singleton Destructor - Deletes source_params_map instance
     * Accessible everywhere.
     */
    static void destroy();

    /**
     * Inserts the supplied sensor pointer into the sensor_manager and the
     * sensor_pair_manager map with the sensorID provided.
     * @param sensorID is the associated key to the sensor pointer.
     * @param sensor_ is sensor pointer to be inserted.
     * @return false if sensorID was already in the map.
     */
    bool insert(const sensor::id_type sensorID, sensor* sensor_);

    /**
     * Deletes the sensor pointer from the heap and the associated sensorID
     * from the map. Also deletes the sensor from the sensor_pair_manager
     * @param sensorID is the associated key to the map.
     * @return false if sensorID was not in the map or the sensor_pair_manager.
     */
    bool erase(const sensor::id_type sensorID);

    /**
     * Updates a pre-existing the sensor and the associated sensorID
     * in the the map. Also updates the sensor in the sensor_pair_manager
     * @param sensorID is the associated key to the map.
     * @param sensor_ is sensor pointer to be inserted.
     * @return false if sensorID was not in the map or the sensor_pair_manager.
     */
    bool update(const sensor::id_type sensorID, sensor* sensor_);

private:

	/**
     * The singleton access pointer.
     */
    static unique_ptr<sensor_manager> _instance;

    /**
     * The _mutex for the singleton pointer.
     */
    static read_write_lock _mutex ;
};

/// @}
} // end of namespace sensors
} // end of namespace usml
