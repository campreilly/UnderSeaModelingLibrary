/**
 *  @file sensor_manager.cc
 *  Implementation of the sensor_manager
 *  Created on: 12-Feb-2015 3:41:31 PM
 */

#include <usml/sensors/sensor_manager.h>

using namespace usml::sensors;

/**
 * Initialization of private static member _instance
 */
unique_ptr<sensor_manager> sensor_manager::_instance;

/**
 * The _mutex for the singleton sensor_manager.
 */
read_write_lock sensor_manager::_instance_mutex;

/**
 * Singleton Constructor sensor_manager
 */
sensor_manager* sensor_manager::instance() {
	sensor_manager* tmp = _instance.get();
	if (tmp == NULL) {
		write_lock_guard guard(_instance_mutex);
		tmp = _instance.get();
		if (tmp == NULL) {
			tmp = new sensor_manager();
			_instance.reset(tmp);
		}
	}
	return tmp;
}

/**
 * Construct a new instance of a specific sensor type.
 */
bool sensor_manager::add_sensor(sensor::id_type sensorID,
		sensor_params::id_type paramsID, const std::string& description )
{
	if (find(sensorID) != 0) {
		return false;
	}
	sensor::reference created( new sensor(sensorID, paramsID, description));
	return insert(sensorID,created);
}

/**
 * Removes an existing sensor instance by sensorID.
 */
bool sensor_manager::remove_sensor(const sensor::id_type sensorID) {
	// sensor_pair_manager::instance()->remove_sensor(find(sensorID));
	return erase(sensorID);
}

/**
 * Updates an existing sensor instance by sensorID.
 */
bool sensor_manager::update_sensor(const sensor::id_type sensorID,
		const wposition1& position, const sensor_orientation& orientation,
		bool force_update)
{
	sensor::reference current_sensor = find(sensorID);
	if (current_sensor.get() != NULL ) {
		current_sensor->update_sensor(position, orientation, force_update);
		return true;
	}
	return false;
}
