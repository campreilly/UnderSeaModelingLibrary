///////////////////////////////////////////////////////////
//  @file sensor_listener.h
//  Definition of the Interface sensor_listener
//  Created on:      26-Feb-2015 5:46:28 PM
//  Original author: Ted Burns, AEgis Technologies
///////////////////////////////////////////////////////////

#pragma once

#include <usml/usml_config.h>
#include <usml/sensors/sensor.h>
#include <usml/sensors/sensorIDType.h>
#include <usml/sensors/xmitRcvModeType.h>

namespace usml {
namespace sensors {

/// @ingroup sensors
/// @{

/**
* This class is part of a Observer/Subject pattern for the sensor class
* and allows for multiple sensor_listener's to be added to the sensor class.
* The sensor_changed call must be defined in each class which inherits it.
*/
      
class USML_DECLSPEC sensor_listener
{
  public:

    /**
     * Destructor.
     */
    virtual ~sensor_listener() {}

    /**
     * sensor_changed
     * Pure virtual method to add eigenray to an object.
     *  @param mode  sensor type - Receiver, Source, or Both
     *  @param   sensorID  of the sensor.
     */
    virtual bool sensor_changed(xmitRcvModeType mode, sensorIDType sensorID) = 0;

  protected:

    /**
     * Constructor - protected
     */
    sensor_listener() {}

  private:

    /** 
     * Disabling default copy constructor and default
     * assignment operator.
     */ 
    sensor_listener(const sensor_listener& yRef);
    sensor_listener& operator=(const sensor_listener& yRef);
};

/// @}
} // end of namespace sensors
} // end of namespace usml

