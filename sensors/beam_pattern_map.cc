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
 * Singleton Constructor - Double Check Locking Pattern DCLP
 */
beam_pattern_map* beam_pattern_map::instance()
{
    beam_pattern_map* tmp = _instance;
    // TODO: insert memory barrier.
    if (tmp == NULL)
    {
        write_lock_guard(_mutex);
        tmp = _instance;
        if (tmp == NULL)
        {
            tmp = new beam_pattern_map();
            // TODO: insert memory barrier
            _instance = tmp;
        }
    }
    return tmp;
}

/**
 * Singleton Destructor
 */
void beam_pattern_map::destroy()
{
    delete _instance;
    _instance = NULL;
}
