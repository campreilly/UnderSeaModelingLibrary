
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
#include <usml/sensors/sensorIDType.h>
#include <usml/sensors/xmitRcvModeType.h>
#include <usml/sensors/sensor_listener.h>
#include <usml/sensors/sensor_pair.h>
#include <usml/threads/read_write_lock.h>

namespace usml {
namespace sensors {

using namespace threads ;
using namespace waveq3d ;
using namespace eigenverb ;

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

typedef std::list<sensorIDType> sensor_list_type;
typedef std::list<sensorIDType>::iterator sensor_list_iter;

typedef std::map <std::string, sensor_pair*> sensor_pair_map_type;
typedef std::map <std::string, sensor_pair*>::iterator sensor_pair_map_iter;

class USML_DECLSPEC sensor_pair_manager : public sensor_listener
{

public:

    /**
     * Singleton Constructor - Creates sensor_pair_manager instance just once.
     * Accessible everywhere.
     * @return  pointer to the instance of the sensor_pair_manager.
     */
    static sensor_pair_manager* instance();

    /**
     * Singleton Destructor - Deletes sensor_pair_manager instance
     * Accessible everywhere.
     */
    static void destroy();

    /**
     * Gets the fathometers for the sourceID requested
     * @param sourceID ID for the source
     * @return proploss pointer
     */
    proploss* get_fathometers(sensorIDType sourceID);

    /**
     * Gets the envelopes for the receiverID requested
     * @param receiverID ID for the receiver
     * @return envelopes in the envelope_collection pointer
     */
    envelope_collection* get_envelopes(sensorIDType receiverID);

    /**
     * Update Proploss for the Source provided.
     * @param sourceID ID of the Source
     * @param fathometers shared_ptr to proploss class
     */
    void update_fathometers(sensorIDType sourceID, proploss_shared_ptr fathometers);

    /**
     * Update eigenverbs for the Source, Receiver or Both
     * @param sensorID sensor ID
     * @param mode  sensor type: Source, Receiver or Both
     * @param eigenverbs shared_ptr to eigenverb_collection class
     */
    void update_eigenverbs(sensorIDType sensorID, xmitRcvModeType mode, eigenverbs_shared_ptr eigenverbs);

    /**
    * Overloaded sensor_changed method via the sensor_listener interface
    * @param sensorID sensor ID
    * @param mode  sensor type: Source, Receiver or Both
    * @return false if sensorID was already activated.
    */
    bool sensor_changed(sensorIDType sensorID,  xmitRcvModeType mode);

    /**
     * Adds the sensor to the sensor_pair_manager
     * @param sensorID sensor ID
     * @param mode  sensor type: Source, Receiver or Both
     */
    void add_sensor(sensorIDType sensorID, xmitRcvModeType mode);

    /**
     * Removes a sensor from the sensor_pair_manager
     * @param sensorID sensor ID
     * @param mode  sensor type: Source, Receiver or Both
     * @return false if sensorID was in Source or Receiver list.
     */
    bool remove_sensor(sensorIDType sensorID, xmitRcvModeType mode);

    /**
     * Gets the sensor pair map.
     * @return const pointer to the _sensor_pair_map
     */
    const sensor_pair_map_type* sensor_pair_map() const
    {
        read_lock_guard guard(_mutex);
        return &_sensor_pair_map;
    }

private:

    /**
     * Inserts the sensor into sensor_pair_map
     * @param sensorID  ID of the sensor to insert.
     * @param mode  sensor type: Source, Receiver or Both
     */
    void map_insert(sensorIDType sensorID, xmitRcvModeType mode);

    /**
     * Erases the sensor from the sensor_pair_map
     * @param sensorID  ID of the sensor to erase
     * @param mode  sensor type: Source, Receiver or Both
     * @param the_list either the _src_list or rcv_list
     */
    void map_erase(sensorIDType sensorID, xmitRcvModeType mode);

    /**
     * Updates the sensor data in  sensor_pair_map
     * @param sensorID  ID of the sensor to update
     * @param mode  sensor type: Source, Receiver or Both
     */
    void map_update(sensorIDType sensorID, xmitRcvModeType mode);

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
     * Destructor - Deletes all pointers its has taken ownership to.
     *   Prevent use of delete, use static destroy above.
     */
    virtual ~sensor_pair_manager();

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
    static sensor_pair_manager* _instance;

    /**
     * The _mutex for the singleton pointer.
     */
    static read_write_lock _mutex;

    /**
     * List of all active Source's
     */
    std::list <sensorIDType> _src_list;

    /**
     * List of all active Receiver's
     */
    std::list <sensorIDType> _rcv_list;

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
