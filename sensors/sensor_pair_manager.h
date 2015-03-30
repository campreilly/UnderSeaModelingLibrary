///////////////////////////////////////////////////////////
//  @file sensor_pair_manager.h
//  Definition of the Class sensor_pair_manager
//  Created on:      26-Feb-2015 5:46:35 PM
//  Original author: Ted Burns, AEgis Technologies Group, Inc.
///////////////////////////////////////////////////////////

#pragma once

#include <cstddef>
#include <list>
#include <map>
#include <sstream>
#include <utility>

#include <usml/usml_config.h>
#include <usml/types/wposition1.h>
#include <usml/waveq3d/proploss.h>
#include <usml/sensors/sensor.h>
#include <usml/sensors/sensor_manager.h>
#include <usml/sensors/xmitRcvModeType.h>
#include <usml/sensors/sensor_pair.h>
#include <usml/threads/read_write_lock.h>
#include <usml/eigenverb/envelope_collection.h>
#include <usml/eigenverb/eigenverb_collection.h>

namespace usml {
namespace sensors {

using namespace usml::threads ;
using namespace usml::waveq3d ;
using namespace usml::eigenverb ;

class sensor_pair;

/// @ingroup sensors
/// @{

/**
 * Manages the containers for all the sensor pair's in use by the USML.
 * A sensor pair contains a source, receiver acoustic pair and it's
 * associated data. The each sensor_pair uses boost::shared_ptrs to the data
 * required. The sensor_pair_manager has a std::map, sensor_pair_map, that
 * uses a key that is a std::string type and consists of the sourceID + "_" + receiverID.
 * The payload of the sensor_pair_map is a pointer to the sensor_pair data.
 */

/**
 * @author Ted Burns, AEgis Technologies Group, Inc.
 * @version 1.0
 * @updated 6-Mar-2015 3:15:03 PM
 */



class USML_DECLSPEC sensor_pair_manager
{

public:

    typedef std::list<sensor::id_type> sensor_list_type;
    typedef std::list<sensor::id_type>::iterator sensor_list_iter;

    typedef std::map <std::string, sensor_pair*> sensor_pair_map_type;
    typedef std::map <std::string, sensor_pair*>::iterator sensor_pair_map_iter;

    /**
     * Singleton Constructor - Creates sensor_pair_manager instance just once.
     * Accessible everywhere.
     * @return  pointer to the instance of the sensor_pair_manager.
     */
    static sensor_pair_manager* instance();

     /**
     * Destructor - Deletes all pointers its has taken ownership to.
     *   Prevent use of delete, use static destroy above.
     */
    virtual ~sensor_pair_manager();

    /**
     * Gets the fathometers for the sourceID requested
     * @param sourceID ID for the source
     * @return proploss pointer
     */
    proploss* get_fathometers(sensor::id_type sourceID);

    /**
     * Gets the envelopes for the receiverID requested
     * @param receiverID ID for the receiver
     * @return envelopes in the envelope_collection pointer
     */
    envelope_collection* get_envelopes(sensor::id_type receiverID);

    /**
     * Adds the sensor to the sensor_pair_manager
     * @param sensor_ pointer to the sensor to add.
     */
    void add_sensor(sensor* sensor_);

    /**
     * Removes a sensor from the sensor_pair_manager
     * @param sensor_ pointer to the sensor to remove.
     * @return false if sensorID was not in Source or Receiver list.
     */
    bool remove_sensor(sensor* sensor_);

    /**
     * Gets the sensor pair map.
     * @return const pointer to the _sensor_pair_map
     */
    const sensor_pair_map_type* sensor_pair_map() const
    {
        read_lock_guard guard(_instance_mutex);
        return &_sensor_pair_map;
    }

private:

    /**
     * Inserts the sensor into sensor_pair_map
     * @param sensor_ pointer to the sensor to insert in the sensor_pair_map
     */
    void map_insert(sensor* sensor_);

    /**
     * Erases the sensor from the sensor_pair_map
     * @param sensor_ pointer to the sensor to erase from the sensor_pair_map
     */
    void map_erase(sensor* sensor_);

//    /**
//     * Updates the sensor data in  sensor_pair_map
//     * @param sensor_ pointer to the sensor to update in the sensor_pair_map
//     */
//    void map_update(sensor* sensor_);

    /**
     * Prints to console the all source and receiver maps
     */
    void print_sensor_pairs();

    /**
     * Default Constructor
     *   private to prevent access.
     */
    sensor_pair_manager() {}

    /**
    * Prevent access to copy constructor
    */
    sensor_pair_manager(sensor_pair_manager const&);

    /**
     * Prevent access to assignment operator
     */
    sensor_pair_manager& operator=( sensor_pair_manager const& );

private:

    /**
     * The singleton access pointer.
     */
    static unique_ptr<sensor_pair_manager> _instance;

    /**
     * The _mutex for the singleton pointer.
     */
    static read_write_lock _instance_mutex;

    /**
     * List of all active Source's
     */
    std::list <sensor::id_type> _src_list;

    /**
     * List of all active Receiver's
     */
    std::list <sensor::id_type> _rcv_list;

    /**
     * sensor pair map container.
     *  Key is a string concatenation of
     *  "sourceID" + "_" + receiverID"
     *  Payload is a pointer to sensor_pair object.
     */
    std::map <std::string, sensor_pair*> _sensor_pair_map;

};

/// @}
}  // end of namespace sensors
}  // end of namespace usml
