/**
 *  @file beam_pattern_map.cc
 *  Implementation of the Class beam_pattern_map
 *  Created on: 12-Feb-2015 3:41:31 PM
 */

#include "beam_pattern_map.h"

using namespace usml::sensors;

/**
* Initialization of private static member _instance
*/
beam_pattern_map* beam_pattern_map::_instance = NULL;

/**
 * Singleton Constructor
 */
beam_pattern_map* beam_pattern_map::instance()
{
    if (_instance == NULL)
    {
        _instance = new beam_pattern_map();
    }
    return _instance;
}
