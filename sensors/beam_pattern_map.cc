/**
 *  @file beam_pattern_map.cc
 *  Implementation of the Class beam_pattern_map
 *  Created on: 10-Feb-2015 12:49:08 PM
 */

#include "beam_pattern_map.h"

using namespace usml::sensors;

// Must set static value to NULL initially
beam_pattern_map* beam_pattern_map::_instance = NULL;

//Protected Constructor
beam_pattern_map::beam_pattern_map()
{}

// Destructor
// Clean up pointers to beam_pattern_model's
beam_pattern_map::~beam_pattern_map()
{
    std::map <const beamIDType, const beam_pattern_model*>::iterator iter;

    for (iter = _map.begin(); iter != _map.end(); ++iter) {
        delete iter->second;
    }

    _map.clear();
}

// Static instance method
beam_pattern_map* beam_pattern_map::instance()
{
	if( _instance == NULL )
		_instance = new beam_pattern_map();	
	return _instance;
}


const beam_pattern_model* beam_pattern_map::find(const beamIDType beamID)
{
    if (_map.count(beamID) == 0) {
        return (NULL);
    }
    return _map.find(beamID)->second;
}

void beam_pattern_map::insert(const beamIDType beamID, const beam_pattern_model* beam_pattern)
{
    _map.insert(std::pair<const beamIDType, const beam_pattern_model* >(beamID, beam_pattern) );
}
