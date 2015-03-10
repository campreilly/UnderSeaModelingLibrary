/**
 *  @file source_params_map.h
 *  Definition of the Class source_params_map
 *  Created on: 10-Feb-2015 12:49:09 PM
 */

#pragma once

#include <cstddef>
#include <iostream>

#include <usml/usml_config.h>
#include <usml/sensors/map_template.h>
#include <usml/sensors/paramsIDType.h>
#include <usml/sensors/source_params.h>

namespace usml {
namespace sensors {

/// @ingroup sensors
/// @{

/**
 * Container for all the source_params's in use by the USML. This class inherits
 * from the map_template class. This class implements the singleton GOF pattern.
 * The map stores pointers to source_params's and take's ownership of the pointers.
 * See usml/sensors/map_template.h A typedef of paramsIDType has been defined
 * to allow for modification of the key of the map at a later time if needed.
 *
 * @author Ted Burns, AEgis Technologies Group, Inc.
 * @version 1.0
 * @updated 27-Feb-2015 3:15:08 PM
 */
class USML_DECLSPEC source_params_map : public map_template <const paramsIDType, const source_params*>
{

public:

    /**
     * Singleton Constructor - Creates source_params_map instance just once.
     * Accessible everywhere.
     * @return  pointer to the instance of the singleton source_params_map
     */
    static source_params_map* instance();

    /**
     * Singleton Destructor - Deletes source_params_map instance
     * Accessible everywhere.
     */
    static void destroy();

private:

    /**
     * Default Constructor
     *   Prevent creation/access other than static instance()
     */
    source_params_map() {}

    /**
     * Destructor - See map_template destructor.
     *   Prevent use of delete, use static destroy above.
     */
    virtual ~source_params_map() {}

    /**
     * Prevent access to copy constructor
     */
    source_params_map(source_params_map const&);

    /**
     * Prevent access to assignment operator
     */
    source_params_map& operator=(source_params_map const&);

    /**
     * The singleton access pointer.
     */
    static source_params_map* _instance;
};

/// @}
} // end of namespace sensors
} // end of namespace usml
