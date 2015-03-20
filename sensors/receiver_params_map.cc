/**
 *  @file receiver_params_map.cc
 *  Implementation of the Class receiver_params_map
 *  Created on: 12-Feb-2015 3:41:31 PM
 */

#include "receiver_params_map.h"

using namespace usml::sensors;

/**
* Initialization of private static member _instance
*/
receiver_params_map* receiver_params_map::_instance = NULL;

/**
 * The _mutex for the singleton pointer.
 */
read_write_lock receiver_params_map::_mutex;

/**
 * Singleton Constructor - Double Check Locking Pattern DCLP
 */
receiver_params_map* receiver_params_map::instance()
{
    receiver_params_map* tmp = _instance;
    // TODO: insert memory barrier.
    if (tmp == NULL)
    {
        write_lock_guard guard(_mutex);
        tmp = _instance;
        if (tmp == NULL)
        {
            tmp = new receiver_params_map();
            // TODO: insert memory barrier
            _instance = tmp;
        }
    }
    return tmp;
}

/**
* Singleton Destructor
*/
void receiver_params_map::destroy()
{
    write_lock_guard guard(_mutex);
    if ( _instance != NULL )
    {
        delete _instance;
        _instance = NULL;
    }
}
