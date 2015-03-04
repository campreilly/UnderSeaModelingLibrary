
///////////////////////////////////////////////////////////
//  sensor_pair_map.h
//  Implementation of the Class sensor_pair_map
//  Created on:      26-Feb-2015 5:46:35 PM
//  Original author: Ted Burns, AEgis Technologies
///////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <utility>

#include <usml/usml_config.h>
#include <usml/types/wposition1.h>
#include <usml/waveq3d/proploss.h>
#include <usml/sensors/sensor.h>
#include <usml/sensors/sensorIDType.h>
#include <usml/sensors/xmitRcvModeType.h>
#include <usml/sensors/sensor_listener.h>
#include <usml/eigenverb/data_collections.h>
#include <usml/eigenverb/eigenverb_collection.h>
#include <usml/eigenverb/envelope_collection.h>

namespace usml {
namespace sensors {

using namespace waveq3d ;
using namespace eigenverb ;

/// @ingroup sensors
/// @{

typedef std::pair<sensorIDType, sensorIDType> key_type;


/**
 * Provides storage for all the sensor pair's in use by the USML. 
 * The sensor pair contains a source, receiver acoustic pair.
 * The sensor_pair_map stores pointers to the data required for each sensor pair.
 * This class inherits it's base map from the std::map.
 * The sensor_pair_map contists of two keys and a pointer to the data_collections. 
 * The first one represents the source an the second key represents the reciever. 
 */
class USML_DECLSPEC sensor_pair_map : public sensor_listener
{

public:    

    /**
    * Singleton Constructor - Creates sensor_pair_map instance just once.
    * Accessible everywhere.
    * @return  pointer to the instance of the sensor_pair_map.
    */
    static sensor_pair_map* instance();
	
    /**
    * Destructor - Deletes all pointers its has taken ownership to.
    */
    virtual ~sensor_pair_map();


    /**
    *
    * @return envelopes
    */
    envelope_collection* get_envelopes();
    /**
    *
    * @return proploss
    */
    proploss* get_fathometers();

  
    /**
    *
    * @param fathometers
    */
    void update_fathometers(proploss* fathometers);

    /**
    *
    * @param envelopes
    */
    void update_envelopes(envelope_collection* envelopes);

    /**
    *
    * @param eigenverbs
    */
    void update_eigenverbs(eigenverb_collection* eigenverbs);


    /**
    * Overloaded sensor_changed via the sensor_listener interface
    * @param mode  sensor type - Receiver, Source, or Both
    * @param sensorID
    */
    bool sensor_changed(xmitRcvModeType mode, sensorIDType sensorID);

	/**
	 * 
	 * @param sourceID
     * @param receiverID
     * @param data the data_collections structure 
	 */
    void add_sensor_pair(sensorIDType sourceID, sensorIDType receiverID, data_collections data);

    /**
    *
    * @param sourceID
    * @param receiverID
    */
    void remove_sensor_pair(sensorIDType sourceID, sensorIDType receiverID);

private:

    /**
    * finds the data_collections associated with the mode, and sensorID.
    * @param mode  sensor type - Receiver, Source, or Both
    * @param sensorID
    */
    void find(xmitRcvModeType mode, sensorIDType sensorID) const;

    /**
    * finds the data_collections structure associated with the key_pair.
    * @param key_pair is the associated key.
    * @return data_collections structure.
    */
    void find_pair(const key_type key_pair) const;

    /**
    * Inserts the supplied data_collections structure into the map 
    * with the key_pair provided.
    * @param key_pair is the associated key to the payload_type.
    * @param payload to be inserted.
    */
    void insert(key_type key_pair, data_collections payload);

protected:
    /**
    * Default Constructor
    *   Protected to prevent access.
    */
    sensor_pair_map() {}

 private: 
    /**
    * Prevent access to copy constructor
    */
     sensor_pair_map(sensor_pair_map const&);

    /**
    * Prevent access to assignment operator
    */
     sensor_pair_map& operator=( sensor_pair_map const& );

    /**
    * The singleton access pointer.
    */
     static sensor_pair_map* _instance;

    /**
    * The std::map that stores the sourceID and receiverID in a std::pair for the "first" item
    * of the map. The second item is the payload of the map and is a data_collections structure.
    */
    std::map< std::pair<sensorIDType, sensorIDType>, data_collections& > _map ;

};

/// @}
}  // end of namespace sensors
}  // end of namespace usml