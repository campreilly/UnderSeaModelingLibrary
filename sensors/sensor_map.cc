/**
 *  @file sensor_map.cc
 *  Implementation of the Class sensor_map
 *  Created on: 12-Feb-2015 3:41:31 PM
 */

#include "sensor_map.h"

using namespace usml::sensors;

/**
* Initialization of private static member _instance
*/
sensor_map* sensor_map::_instance = NULL;

/**
 * The _mutex for the singleton pointer.
 */
read_write_lock sensor_map::_mutex;

/**
 * Singleton Constructor - Double Check Locking Pattern DCLP
 */
sensor_map* sensor_map::instance()
{
    sensor_map* tmp = _instance;
    // TODO: insert memory barrier.
    if (tmp == NULL)
    {
        write_lock_guard guard(_mutex);
        tmp = _instance;
        if (tmp == NULL)
        {
            tmp = new sensor_map();
            // TODO: insert memory barrier
            _instance = tmp;
        }
    }
    return tmp;
}

/**
 * Singleton Destructor
 */
void sensor_map::destroy()
{
    write_lock_guard guard(_mutex);
    if ( _instance != NULL )
    {
        delete _instance;
        _instance = NULL;
    }
}

bool sensor_map::insert(const sensorIDType sensorID, sensor* in_sensor)
{
    // Insert in the map
    bool result = false;
    result = map_template<const sensorIDType, sensor*>::insert(sensorID, in_sensor);

    if (result != false) {
        //Add to the sensor_pair_manager
        _sensor_pair_manager->add_sensor(sensorID, in_sensor->mode() );
    }

    return result;
}

bool sensor_map::erase(const sensorIDType sensorID, xmitRcvModeType mode)
{
    // Insert in the map
    bool result = false;
    result = map_template<const sensorIDType, sensor*>::erase(sensorID);

    if (result != false) {
        //remove from the sensor_pair_manager
        result = _sensor_pair_manager->remove_sensor(sensorID, mode);
    }
    return result;
}

bool sensor_map::update(const sensorIDType sensorID, sensor* in_sensor)
{
	// Input sensor does not contain "all" sensor data
	// Get current data
	sensor* current_sensor = find(sensorID);
	
	// Ensure pre-existance
	if (current_sensor != 0) {
		
		current_sensor->update_sensor(in_sensor->position(), in_sensor->pitch(), in_sensor->yaw());
		return true;
	}

    return false;
}
