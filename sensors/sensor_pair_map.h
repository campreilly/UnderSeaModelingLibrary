
///////////////////////////////////////////////////////////
//  @file sensor_pair_map.h
//  Definition of the Class sensor_pair_map
//  Created on:      26-Feb-2015 5:46:35 PM
//  Original author: Ted Burns, AEgis Technologies
///////////////////////////////////////////////////////////

#pragma once

#include <cstddef>
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


/**
 * The _map is a std:map < sourceID_key, payload >. The payload of the outer
 * map is a std::map < receiverID_key, data>. The payload of the inner map
 * is a reference to data_collections structure.
 */
typedef std::map<sensorIDType, data_collections> inner_map_type;
typedef std::map<sensorIDType, inner_map_type> outer_map_type;


/**
 * Provides storage for all the sensor pair's in use by the USML. 
 * The sensor pair contains a source, receiver acoustic pair.
 * The sensor_pair_map stores pointers to the data required for each sensor pair.
 * This class inherits it's base map from the std::map.
 * The sensor_pair_map consists of two keys and a pointer to the data_collections.
 * The first one represents the source an the second key represents the receiver.
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
    */
    void add_sensor_source(sensorIDType sourceID);

	/**
	 * 
	 * @param sourceID
     * @param receiverID
	 */
    void add_sensor_pair(sensorIDType sourceID, sensorIDType receiverID);

    /**
    *
    * @param sourceID
    * @param receiverID
    */
    void remove_sensor_pair(sensorIDType sourceID, sensorIDType receiverID);

    /**
    * Returns an iterator to the beginning of the map
    * @result outer_map_type::iterator
    */
    outer_map_type::iterator begin(){
        return _map.begin();
    }

    /**
    * Returns an iterator to the end of the map
    * @result outer_map_type::iterator
    */
    outer_map_type::iterator end(){
        return _map.end();
    }

    /**
     * finds data associated with sourceID.
     * @param sourceID
     * @result inner_map_type pointer
     */
    inner_map_type* find(sensorIDType sourceID);

    /**
     * finds the data_collections associated with the mode, and sensorID.
     * @param mode  sensor type - Receiver, Source, or Both
     * @param sensorID
     */
    void find(const usml::sensors::xmitRcvModeType mode, sensorIDType sensorID);


private:

    /**
    * finds the data_collections structure associated with the key_pair.
    * @param key_pair is the associated key.
    * @return data_collections structure.
    */
    void find_pair(const int key_pair) const;

    /**
    * Inserts the supplied data_collections structure into the map 
    * with the key_pair provided.
    * @param key_pair is the associated key to the payload_type.
    * @param payload to be inserted.
    */
    void insert(int key_pair, data_collections& payload);

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
    * The _map is a std:map < sourceID_key, payload >. The payload of the outer
    * map is a std::map < receiverID_key, data>. The payload of the inner map
    * is a reference to data_collections structure.
    */
     outer_map_type _map ;

};

/// @}
}  // end of namespace sensors
}  // end of namespace usml
