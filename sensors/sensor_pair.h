///////////////////////////////////////////////////////////
//  @file sensor_pair.h
//  Definition of the sensor_pair structure.
//  Created on:      27-Feb-2015 5:46:40 PM
//  Original author: Ted Burns, AEgis Technologies
///////////////////////////////////////////////////////////

#pragma once

#include <usml/usml_config.h>
#include <usml/sensors/sensor.h>
#include <usml/sensors/sensor_listener.h>
#include <usml/sensors/xmitRcvModeType.h>
#include <usml/waveq3d/proploss.h>
#include <usml/eigenverb/envelope_collection.h>
#include <usml/eigenverb/eigenverb_collection.h>

namespace usml {
namespace sensors{

using namespace waveq3d ;
using namespace eigenverb;

/// @ingroup sensors
/// @{

/**
 * Container for one sensor pair instance.
 * On construction a pointer to the source and receiver sensor are obtained.
 * Inherits the sensor_listener interface so a sensor instance can get
 * access to its complement sensor, and updates the eigenverbs and fathometers.
 */
class USML_DECLSPEC sensor_pair : public sensor_listener
{
public:

    /**
     * Constructor
     */
    sensor_pair(sensor* source, sensor* receiver)
        : _source(source),_receiver(receiver) {};

    /**
     * Destructor
     */
    ~sensor_pair()
    {

    }

    /**
     * Gets a pointer to the source sensor.
     * @return  Pointer to the source sensor
     */
    sensor* source() const {
        return _source;
    }

     /**
     * Gets a pointer to the receiver sensor.
     * @return  Pointer to the receiver sensor
     */
    sensor* receiver() const {
        return _receiver;
    }

    /**
     * update_fathometers
     * Overloads the sensor_listener method to update the fathometers in the sensor_pair
     * @param  Pointer to the sensor object which contains fathometers to update.
     */
    virtual void update_fathometers(sensor* the_sensor);

    /**
     * update_eigenverbs
     * Overloads the sensor_listener method to update the eigenverb_collection sensor_pair
     * @param  Pointer to the sensor object which contains eigenverbs to update.
     */
    virtual void update_eigenverbs(sensor* the_sensor);

   /**
    * remove_sensor
    * Overloads the sensor_listener method to remove the sensor object from the sensor_pair.
    * @param  Pointer to the sensor object which will be removed.
    */
    virtual void remove_sensor(sensor* the_sensor);

    /**
    * sensor_complement
    * Overloads the sensor_listener to return the complement sensor of the sensor_pair
    * @return  Pointer to the complement sensor object of the pair.
    */
    virtual sensor* sensor_complement (sensor* the_sensor);

private:

    sensor_pair()
       : _source(NULL),_receiver(NULL) {};

    /**
     * Pointer to the source sensor.
     */
    sensor* _source;

    /**
     * Pointer to the receiver sensor.
     */
    sensor* _receiver;

    /**
     * proploss - contains targets and eigenrays 
     */
    proploss::reference _proploss;

    /**
     * source eigenverbs - contains all source eigenverbs
     */
    eigenverbs_shared_ptr _src_eigenverbs;

    /**
     * receiver eigenverbs - contains all receiver eigenverbs
     */
    eigenverbs_shared_ptr _rcv_eigenverbs;

    /**
     * envelopes - contains the Reverb envelopes
     */
    envelopes_shared_ptr _envelopes;

};

/// @}
} // end of namespace sensors
} // end of namespace usml
