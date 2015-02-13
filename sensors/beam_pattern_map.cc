/**
 *  @file beam_pattern_map.cc
 *  Implementation of the Class beam_pattern_map
 *  Created on: 10-Feb-2015 12:49:08 PM
 */

#include "beam_pattern_map.h"

using namespace usml::sensors;


beam_pattern_map::beam_pattern_map()
{

}


beam_pattern_map::~beam_pattern_map()
{

}


beam_pattern_map* beam_pattern_map::instance()
{
	if( _instance == NULL )
		_instance = new beam_pattern_map();	
	return _instance;
}


beam_pattern_model* beam_pattern_map::find(const beamIDType beamID)
{
	return NULL;
}

void beam_pattern_map::insert(const beamIDType beamID, const beam_pattern_model beam_pattern)
{

}