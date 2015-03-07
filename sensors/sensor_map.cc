/**
 *  @file sensor_map.cc
 *  Implementation of the Class sensor_map
 *  Created on: 12-Feb-2015 3:41:31 PM
 */

#include "sensor_map.h"

using namespace usml::sensors;

/**
 * Destructor - See singleton_map destructor.
 */
sensor_map::~sensor_map()
{

}

/**
 * Removes a sensor pointer from the sensor_map
 */
bool sensor_map::erase(const sensorIDType sensorID)
{
    bool result = false;  // EVAR needs to return correct code
                          // when keyID does not pre-exist
    // Check for Pre-existance
    if (find(sensorID) != 0)
    {
        //_map.erase(sensorID );
        result = true;
    }
    return result;
}

/**
 *  * Updates sensor pointer at the pre-existing sensorID key.
 */
bool sensor_map::update(const sensorIDType sensorID, sensor* sensor)
{
    // EVAR needs to return correct error code
    // when keyID does not pre-exist
    bool result = false;

    // Check for Pre-existance
    if (find(sensorID) != 0)
    {
        result = insert(sensorID, sensor);
    }
    return result;
}
