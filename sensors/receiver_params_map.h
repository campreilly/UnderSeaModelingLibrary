/**
 *  @file receiver_params_map.h
 *  Definition of the Class receiver_params_map
 *  Created on: 10-Feb-2015 12:49:09 PM
 */

#pragma once

#include <cstddef>

#include <usml/usml_config.h>
#include <usml/sensors/sensor_map_template.h>
#include <usml/sensors/paramsIDType.h>
#include <usml/sensors/receiver_params.h>
#include <usml/threads/read_write_lock.h>

namespace usml {
namespace sensors {

using namespace usml::threads;

/// @ingroup sensors
/// @{

/**
 * Container for all the receiver_params's in use by the USML. This class inherits
 * from the map_template class. This class implements the singleton GOF pattern.
 * The map stores pointers to receiver_params's and take's ownership of the pointers.
 * See usml/sensors/map_template.h A typedef of paramsIDType has been defined
 * to allow for modification of the key of the map at a later time if needed.
 *
 * @author Ted Burns, AEgis Technologies Group, Inc.
 * @version 1.0
 * @updated 27-Feb-2015 3:15:00 PM
 */
	
class USML_DECLSPEC receiver_params_map : public sensor_map_template <const paramsIDType, const receiver_params*>
{

public:

    /**
     * Singleton Constructor - Creates receiver_params_map instance just once.
     * Accessible everywhere.
     * @return  pointer to the instance of the singleton receiver_params_map
     */
    static receiver_params_map* instance();

    /**
     * Singleton Destructor - Deletes receiver_params_map instance
     * Accessible everywhere.
     */
    static void destroy();



private:

	/**
     * Default Constructor
     *   Prevent creation/access other than static instance()
     */
    receiver_params_map() {}

     /**
     * Destructor - See map_template destructor.
     *   Prevent use of delete, use static destroy above.
     */
    virtual ~receiver_params_map() {}

	/**
     * Prevent access to copy constructor
     */
	receiver_params_map(receiver_params_map const&);

    /**
     * Prevent access to assignment operator
     */
	receiver_params_map& operator=(receiver_params_map const&);

	/**
     * The singleton access pointer.
     */
    static receiver_params_map* _instance;

     /**
     * The _mutex for the singleton pointer.
     */
    static read_write_lock _mutex;
};

/// @}
} // end of namespace sensors
} // end of namespace usml
