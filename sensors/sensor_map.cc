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
 * Singleton Constructor
 */
sensor_map* sensor_map::instance()
{
    if (_instance == NULL)
    {
        _instance = new sensor_map();
    }
    return _instance;
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
