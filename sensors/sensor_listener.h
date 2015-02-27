///////////////////////////////////////////////////////////
//  sensor_listener.h
//  Definition of the Interface sensor_listener
//  Created on:      26-Feb-2015 5:46:28 PM
//  Original author: Ted Burns, AEgis Technologies
///////////////////////////////////////////////////////////

#pragma once

#include <usml/sensors/sensorIDType.h>
#include <usml/sensors/sensor.h>

namespace usml {
namespace sensors {

/// @ingroup sensors
/// @{

/**
* This class is part of a Observer/Subject pattern for the sensor class
* and allows for multiple sensor_listener to be added to the sensor class.
* The sensor_changed call must be defined in each class which inherits it.
*/
      
class sensor_listener
{
    public:
    /**
    * sensor_changed
    * Pure virtual method to add eigenray to an object.
    *  @param   sensorID  Index of the sensor
    *  @param   data      sensor data
    */
    virtual bool sensor_changed(sensorIDType sensorID, sensor data) = 0;

    protected:

    /**
    * Constructor - protected
    */
    sensor_listener() {}

    private:

    // -------------------------
    // Disabling default copy constructor and default
    // assignment operator.
    // -------------------------
    sensor_listener(const sensor_listener& yRef);
    sensor_listener& operator=(const sensor_listener& yRef);
};

/// @}
} // end of namespace sensors
} // end of namespace usml

