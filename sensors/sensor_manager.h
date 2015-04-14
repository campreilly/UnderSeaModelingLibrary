/**
 * @file sensor_manager.h
 * Container for all the sensor's in use by the USML.
 */
#pragma once

#include <usml/usml_config.h>

#include <usml/threads/threads.h>
#include <usml/sensors/sensor_model.h>
#include <usml/sensors/sensor_params.h>
#include <usml/sensors/sensor_pair_manager.h>
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
 * See usml/sensors/map_template.h A typedef of sensor_model::id_type has been defined
 * to allow for modification of the key of the map at a later time if needed.
 *
 * @author Ted Burns, AEgis Technologies Group, Inc.
 * @version 1.0
 * @updated 27-Feb-2015 3:15:03 PM
 */
class USML_DECLSPEC sensor_manager {
public:

	/**
	 * Singleton Constructor - Creates sensor_manager instance just once.
	 * Accessible everywhere.
	 * @return  pointer to the instance of the singleton sensor_manager
	 */
	static sensor_manager* instance();

	/**
     * Default destructor.
     */
    virtual ~sensor_manager() {
    }

    /**
     * Reset the sensor_manager singleton unique pointer to empty.
     */
    static void reset();

	/**
	 * Construct a new instance of a specific sensor type.
	 * Automatically invokes the sensor_pair_manager::add_sensor() on the
	 * newly created sensor. Note that the position and orientation are not
	 * valid until the update_sensor() is invoked for the first time.
	 *
	 * @param sensorID		Identification used to find this sensor instance
	 * 						in sensor_manager.
	 * @param paramsID		Identification used to lookup sensor type data
	 * 						in source_params_map and receiver_params_map.
	 * @param description	Human readable name for this sensor instance.
	 * @return 				False if sensorID already exists.
	 */
	bool add_sensor(sensor_model::id_type sensorID, sensor_params::id_type paramsID,
			const std::string& description = std::string());

	/**
	 * Removes an existing sensor instance by sensorID.
	 * Also deletes the sensor from the sensor_pair_manager.
	 *
	 * @param sensorID		Identification used to find this sensor instance
	 * 						in sensor_manager.
	 * @return 				False if sensorID was not found.
	 */
	bool remove_sensor(sensor_model::id_type sensorID);

	/**
	 * Updates an existing sensor instance by sensorID.
	 * Also updates the sensor in the sensor_pair_manager.
	 *
	 * @param sensorID		Identification used to find this sensor instance
	 * 						in sensor_manager.
	 * @param position  	Updated position data
	 * @param orientation	Updated orientation value
	 * @param force_update	When true, forces update without checking thresholds.
	 * @return 				False if sensorID was not found.
	 */
	bool update_sensor(sensor_model::id_type sensorID, const wposition1& position,
			const orientation& orientation, bool force_update = false);

	/**
	 * Finds the sensor parameters associated with the keyID.
	 *
	 * @param 	keyID 	Key used to lookup the sensor type parameters.
	 * @return			Sensor parameters if found, blank entry if not.
	 */
	sensor_model::reference find(sensor_model::id_type keyID) const {
		return _map.find(keyID) ;
	}


private:

	/**
     * Hide access to default constructor.
     */
    sensor_manager() {
    }

    /**
     * Hide access to copy constructor
     */
    sensor_manager(sensor_manager const&);

    /**
     * Hide access to assignment operator
     */
    sensor_manager& operator=(sensor_manager const&);

	/**
	 * The singleton access pointer.
	 */
	static unique_ptr<sensor_manager> _instance;

	/**
	 * The mutex for the singleton pointer.
	 */
	static read_write_lock _instance_mutex;

	/**
	 * The mutex for adding and removing sensors in manager.
	 */
	mutable read_write_lock _manager_mutex;

	/**
	 * Container used to store sensors.
	 */
	sensor_map_template<sensor_model::id_type, sensor_model::reference> _map ;
};

/// @}
}// end of namespace sensors
} // end of namespace usml
