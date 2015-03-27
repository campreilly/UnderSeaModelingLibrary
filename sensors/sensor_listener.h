///////////////////////////////////////////////////////////
//  @file sensor_listener.h
//  Definition of the Interface sensor_listener
//  Created on:      26-Feb-2015 5:46:28 PM
//  Original author: Ted Burns, AEgis Technologies
///////////////////////////////////////////////////////////

#pragma once

#include <usml/usml_config.h>
#include <usml/sensors/sensor.h>

namespace usml {
namespace sensors {

class sensor;

/// @ingroup sensors
/// @{

/* @class sensor_listener interface
 * This class is part of a Observer/Subject pattern for the sensor class.
 * and allows other classes to call methods in the class that implemented it.
 * The update_fathometers and update_eigenverbs methods must be defined in
 * each class which inherits it. The primary purpose for this class was to access
 * sensor_pair data.
 */
      
class USML_DECLSPEC sensor_listener
{
  public:

    /**
     * Destructor.
     */
    virtual ~sensor_listener() {}

    /**
     * update_fathometers
     * Pure virtual method to update the fathometers in the sensor_pair
     * @param  Pointer to the sensor object which contains fathometers to update.
     */
    virtual void update_fathometers(sensor* the_sensor) = 0;

    /**
     * update_eigenverbs
     * Pure virtual method to update the eigenverb_collection sensor_pair
     * @param  Pointer to the sensor object which contains eigenverbs to update.
     */
    virtual void update_eigenverbs(sensor* the_sensor) = 0;

   /**
    * remove_sensor
    * Pure virtual method to remove the sensor object from the sensor_pair.
    * @param  Pointer to the sensor object which will be removed.
    */
    virtual void remove_sensor(sensor* the_sensor) = 0;

    /**
    * sensor_complement
    * Pure virtual method to return the complement sensor of the sensor_pair
    * @return  Pointer to the sensor object which will be removed.
    */
    virtual sensor* sensor_complement (sensor* the_sensor) = 0;

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

