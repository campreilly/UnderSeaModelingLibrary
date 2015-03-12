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
 * Singleton Constructor - Double Check Locking Pattern DCLP
 */
sensor_map* sensor_map::instance()
{
    sensor_map* tmp = _instance;
    // TODO: insert memory barrier.
    if (tmp == NULL)
    {
        write_lock_guard(_mutex);
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
    delete _instance;
    _instance = NULL;
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

bool sensor_map::erase(const sensorIDType sensorID)
{
    // Insert in the map
    bool result = false;
    result = map_template<const sensorIDType, sensor*>::erase(sensorID);

    if (result != false) {
        //Add to the sensor_pair_manager
        result = _sensor_pair_manager->remove_sensor(sensorID);
    }
    return result;
}
