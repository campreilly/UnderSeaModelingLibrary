/**
 *  @file source_params_map.cc
 *  Implementation of the Class source_params_map
 *  Created on: 12-Feb-2015 3:41:31 PM
 */

#include "source_params_map.h"

using namespace usml::sensors;

/**
* Initialization of private static member _instance
*/
source_params_map* source_params_map::_instance = NULL;

/**
 * Singleton Constructor - Double Check Locking Pattern DCLP
 */
source_params_map* source_params_map::instance()
{
    source_params_map* tmp = _instance;
    // TODO: insert memory barrier.
    if (tmp == NULL)
    {
        write_lock_guard(_mutex);
        tmp = _instance;
        if (tmp == NULL)
        {
            tmp = new source_params_map();
            // TODO: insert memory barrier
            _instance = tmp;
        }
    }
    return tmp;
}

/**
 * Singleton Destructor
 */
void source_params_map::destroy()
{
    delete _instance;
    _instance = NULL;
}
