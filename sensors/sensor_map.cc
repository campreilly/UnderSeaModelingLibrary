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
