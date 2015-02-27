/**
 *  @file source_params_inherit_map.cc
 *  Implementation of the Class source_params_inherit_map
 *  Created on: 10-Feb-2015 12:49:09 PM
 */

#include "source_params_inherit_map.h"

using namespace usml::sensors;

// Must set static value to NULL initially
source_params_inherit_map* source_params_inherit_map::_instance = NULL;

/**
 * Destructor - Deletes pointers to beam_pattern_model's
 */
source_params_inherit_map::~source_params_inherit_map()
{
    std::map <const paramsIDType, const source_params*>::iterator iter;

    for (iter = begin(); iter != end(); ++iter) {
        delete iter->second;
    }

    clear();
}

/**
 * Singleton Constructor - Creates beam_pattern_map instance just once, then
 * Accessible everywhere.
 * @return  pointer to the instance of the source_params_inherit_map.
 */
source_params_inherit_map* source_params_inherit_map::instance()
{
	if( _instance == NULL )
        _instance = new source_params_inherit_map();
	return _instance;
}

