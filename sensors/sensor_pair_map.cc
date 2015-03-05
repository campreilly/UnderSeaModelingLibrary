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
void sensor_pair_map::add_sensor_source(sensorIDType sourceID)
{
    if (_map.count(sourceID) != 0)
    {
        _map[sourceID];
    }
}

/**
*
*/
void sensor_pair_map::add_sensor_pair(sensorIDType sourceID, sensorIDType receiverID)
{
    // Create outer and inner maps
    // if outer sourceID already exist and inner receiverID doesn't
    // the following one line will do it all
     _map[sourceID][receiverID];

     cout << "add_sensor_pair: created sourceID " << sourceID << " with receiverID " << receiverID << endl;

#ifdef USML_DEBUG

    outer_map_type::iterator result = _map.find(sourceID);
    if ( result != _map.end() )
    {
        cout << "Map contents: " << endl;
        for (outer_map_type::iterator oi = _map.begin(); oi != _map.end(); ++oi)
        {
            inner_map_type &im = oi->second;
            for (inner_map_type::iterator ii = im.begin(); ii != im.end(); ++ii)
            {
                cout << "_map[" << oi->first << "][" << ii->first << "] =" << endl;
            }
        }
    }
#endif

}

/**
*
*/
void sensor_pair_map::remove_sensor_pair(sensorIDType sourceID, sensorIDType receiverID)
{
   
}

/**
* finds the data_collections associated with the key_pair.
*/
inner_map_type* sensor_pair_map::find(sensorIDType sourceID)
{

    if ( _map.count(sourceID) == 0 )
    {
        return NULL;
    }
    return &(_map.find(sourceID)->second);

}

/**
* finds the data_collections associated with the mode, and sensorID.
*/
void sensor_pair_map::find(const usml::sensors::xmitRcvModeType mode, sensorIDType sensorID)
{
    inner_map_type inner_map;

    switch (mode)
    {
        default:
            assert(false);
            break;
        case usml::sensors::SOURCE:
        {
            inner_map = _map.find(sensorID)->second;
            break;
        }
        case usml::sensors::RECEIVER:
        {
            inner_map = _map.find(sensorID)->second;
            break;
        }
        case usml::sensors::BOTH:
        {
            inner_map = _map.find(sensorID)->second;
        }
    }
   
    //if ( _map.count(sensorID) == 0 )
    //{
    //    return ( NULL );
    //}
    //return _map.find(sensorID)->second;

}

/**
* finds the data_collections associated with the key_pair.
*/
void sensor_pair_map::find_pair(const int key_pair) const
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
void sensor_pair_map::insert(int key_pair, data_collections& payload)
{
    //_map.insert(std::pair<key_type, data_collections& >(key_pair, payload));
}
