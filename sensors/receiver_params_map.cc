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
 * Singleton Constructor
 */
receiver_params_map* receiver_params_map::instance()
{
    if (_instance == NULL)
    {
        _instance = new receiver_params_map();
    }
    return _instance;
}

/**
 * Singleton Destructor
 */
void receiver_params_map::destroy()
{
    delete _instance;
    _instance = NULL;
}
