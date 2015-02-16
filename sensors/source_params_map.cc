/**
 *  @file source_params_map.cc
 *  Implementation of the Class source_params_map
 *  Created on: 10-Feb-2015 12:49:09 PM
 */

#include "source_params_map.h"

using namespace usml::sensors;

source_params_map* source_params_map::_instance = NULL;

source_params_map::source_params_map()
{

}

source_params_map::~source_params_map()
{

}

source_params_map* source_params_map::instance()
{
	if( _instance == NULL )
        _instance = new source_params_map();
	return _instance;
}

void source_params_map::insert(const paramsIDType sourceID, const source_params* source_params)
{

}

source_params* source_params_map::find(const paramsIDType sourceID)
{
	return  NULL;
}