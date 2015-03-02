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
* Destructor
*/
sensor_pair_map::~sensor_pair_map()
{

}

/**
*
* @return proploss pointer
*/
proploss* sensor_pair_map::get_fathometers()
{
    return NULL;
}

/**
*
* @return envelope_collection pointer
*/
envelope_collection* sensor_pair_map::get_envelopes()
{
    return NULL;
}

/**
*
* @param fathometers
*/
void sensor_pair_map::update_fathometers(proploss* fathometers)
{

}

/**
*
* @param envelopes
*/
void sensor_pair_map::update_envelopes(envelope_collection* envelopes)
{

}

/**
*
* @param eigenverbs
*/
void sensor_pair_map::update_eigenverbs(eigenverb_collection* eigenverbs)
{

}

/**
*
* @param sensorID
*/
bool sensor_pair_map::sensor_changed(sensorIDType sensorID)
{
    return false;
}


/**
 * 
 * @param sensor
 */
void sensor_pair_map::add_sensor(sensor sensor)
{

}

/**
 * 
 * @param sensor
 */
void sensor_pair_map::remove_sensor(sensor sensor)
{

}