///////////////////////////////////////////////////////////
//  @file sensor_pair_map.cc
//  Implementation of the Class sensor_pair_map
//  Created on:      26-Feb-2015 5:46:35 PM
//  Original author: Ted Burns, AEgis Technologies
///////////////////////////////////////////////////////////

#include "sensor_pair_map.h"

using namespace usml::sensors ;

/**
* Initialization of private static member _instance
*/
sensor_pair_map* sensor_pair_map::_instance = NULL;

/**
* Singleton Constructor
*/
sensor_pair_map* sensor_pair_map::instance()
{
    if ( _instance == NULL )
        _instance = new sensor_pair_map();
    return _instance;
}

/**
* Destructor
*/
sensor_pair_map::~sensor_pair_map()
{

}

/**
*
*/
proploss* sensor_pair_map::get_fathometers()
{
    return NULL;
}

/**
*
*/
envelope_collection* sensor_pair_map::get_envelopes()
{
    return NULL;
}

/**
*
*/
void sensor_pair_map::update_fathometers(proploss* fathometers)
{

}

/**
*
*/
void sensor_pair_map::update_envelopes(envelope_collection* envelopes)
{

}

/**
*
*/
void sensor_pair_map::update_eigenverbs(eigenverb_collection* eigenverbs)
{

}

/**
* Overloaded sensor_changed via the sensor_listener interface
*/
bool sensor_pair_map::sensor_changed(xmitRcvModeType mode, sensorIDType sensorID)
{
    return false;
}

/**
*
*/
void sensor_pair_map::add_sensor_pair(sensorIDType sourceID, sensorIDType receiverID, data_collections data)
{
    key_type key_pair(sourceID, receiverID);
    
    insert(key_pair, data);
}

/**
*
*/
void sensor_pair_map::remove_sensor_pair(sensorIDType sourceID, sensorIDType receiverID)
{
   
}

/**
* finds the data_collections associated with the mode, and sensorID.
*/
void sensor_pair_map::find(xmitRcvModeType mode, sensorIDType sensorID) const
{
   
//if ( _map.count(key_pair) == 0 )
//{
//    return ( NULL );
//}
//return _map.find(key_pair)->second;

}

/**
* finds the data_collections associated with the key_pair.
*/
void sensor_pair_map::find_pair(const key_type key_pair) const
{

//if ( _map.count(key_pair) == 0 )
//{
//    return ( NULL );
//}
//return _map.find(key_pair)->second;

}

/**
* Inserts the supplied payload_type into the map with the key provided.
*/
void sensor_pair_map::insert(key_type key_pair, data_collections payload)
{
    _map.insert(std::pair<key_type, data_collections >(key_pair, payload));
}