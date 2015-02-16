/**
 *  @file receiver_params_map.cc
 *  Implementation of the Class receiver_params_map
 *  Created on: 10-Feb-2015 12:49:09 PM
 */

#include "receiver_params_map.h"

using namespace usml::sensors;

receiver_params_map* receiver_params_map::_instance = NULL;

receiver_params_map::receiver_params_map()
{

}

receiver_params_map::~receiver_params_map()
{

}

receiver_params_map* receiver_params_map::instance()
{
	if( _instance == NULL )
        _instance = new receiver_params_map();
	return _instance;
}

receiver_params* receiver_params_map::find(paramsIDType receiverID)
{
	return NULL;
}

void receiver_params_map::insert(const paramsIDType receiverID, const receiver_params* receiver_params)
{

}