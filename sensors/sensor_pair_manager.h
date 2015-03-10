
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
#include <utility>

#include <usml/usml_config.h>
#include <usml/types/wposition1.h>
#include <usml/waveq3d/proploss.h>
#include <usml/sensors/sensor.h>
#include <usml/sensors/sensorIDType.h>
#include <usml/sensors/xmitRcvModeType.h>
#include <usml/sensors/sensor_listener.h>
#include <usml/eigenverb/source_data.h>
#include <usml/eigenverb/receiver_data.h>

namespace usml {
namespace sensors {

using namespace waveq3d ;
using namespace eigenverb ;

/// @ingroup sensors
/// @{

/**
 * Manages the containers for all the sensor pair's in use by the USML.
 * A sensor pair contains a source, receiver acoustic pair.
 * The sensor_pair_manager stores boost::shared_ptrs pointers
 * to the data required for each sensor pair.
 * The sensor_pair_manager consists of a map for the both source and receiver data.
 * A Map which the uses the sourceID as a key to all the receiverID's for that source.
 * Another map which uses the receiverID as the key to all sourceID's for that receiver,
 * and lists of all the active sources and receivers.
 * The first one represents the source an the second key represents the receiver.
 */

/**
 * @author Ted Burns, AEgis Technologies Group, Inc.
 * @version 1.0
 * @updated 6-Mar-2015 3:15:03 PM
 */

typedef std::list<sensorIDType>::iterator sensor_iter;

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
     * Update Reverb Envelopes for the Receiver provided.
     * @param receiverID ID of the Receiver
     * @param envelopes shared_ptr to envelope_collection class
     */
    void update_envelopes(sensorIDType receiverID, envelopes_shared_ptr envelopes);

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
     */
    void remove_sensor(sensorIDType sensorID, xmitRcvModeType mode);

    /**
     * Adds the source and receiver sensor pair to the sensor_pair_manager
     * @param sourceID the source sensor ID
     * @param receiverID the receiver sensor ID
     */
    void add_sensor_pair(sensorIDType sourceID, sensorIDType receiverID);

    /**
     * Removes the source and receiver sensor pair from the sensor_pair_manager
     * @param sourceID the source sensor ID
     * @param receiverID the receiver sensor ID
     */
    void remove_sensor_pair(sensorIDType sourceID, sensorIDType receiverID);

private:

    /**
     * Synchronizes the source and receiver maps to
     * the active source and receiver lists.
     */
    void synch_sensor_pairs();

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
     * List of all active Source's
     */
    std::list <sensorIDType> _src_list;

    /**
     * List of all active Receiver's
     */
    std::list <sensorIDType> _rcv_list;

    /**
     * SourceID to source data map
     */
    std::map <sensorIDType, source_data> _src_data_map;

    /**
     * SourceID to receiver data map
     */
    std::map <sensorIDType, receiver_data> _rcv_data_map;

    /**
     * SourceID to list of ReceiverID's map
     */
    std::map <sensorIDType, std::list<sensorIDType> > _src_rcv_pair_map;

    /**
     * ReceiverID to list of SourceID's map
     */
    std::map <sensorIDType, std::list<sensorIDType> > _rcv_src_pair_map;
};

/// @}
}  // end of namespace sensors
}  // end of namespace usml
