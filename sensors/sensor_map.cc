/**
 *  @file sensor_map.cc
 *  Implementation of the Class sensor_map
 *  Created on: 12-Feb-2015 3:41:31 PM
 */

#include "sensor_map.h"

using namespace usml::sensors;

sensor_map* sensor_map::_instance = NULL;

sensor_map::sensor_map()
{

}

sensor_map::~sensor_map()
{

}

sensor_map* sensor_map::instance()
{
	if( _instance == NULL )
		_instance = new sensor_map();	
	return _instance;
}

void sensor_map::insert(const sensorIDType sensorID, const sensor sensor)
{

}

void sensor_map::erase(const sensorIDType sensorID)
{

}

void sensor_map::update(const sensorIDType sensorID, const sensor sensor)
{

}

sensor* sensor_map::find(const sensorIDType sensorID)
{
	return NULL;
}