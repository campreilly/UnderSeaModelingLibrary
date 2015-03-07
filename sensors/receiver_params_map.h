/**
 *  @file receiver_params_map.h
 *  Definition of the Class receiver_params_map
 *  Created on: 10-Feb-2015 12:49:09 PM
 */

#pragma once

#include <usml/usml_config.h>
#include <usml/sensors/singleton_map.h>
#include <usml/sensors/paramsIDType.h>
#include <usml/sensors/receiver_params.h>

namespace usml {
namespace sensors {

/// @ingroup sensors
/// @{

/**
 * Storage for all the receiver_params's in use by the USML. This class inherits
 * from the templated singleton_map class. The map stores pointers to
 * receiver_params and take's ownership of the pointers. See
 * usml/sensors/singleton_map.h A typedef of paramsIDType has been defined to
 * allow for modification of the key of the map at a later time if needed.
 *
 * @author Ted Burns, AEgis Technologies Group, Inc.
 * @version 1.0
 * @updated 27-Feb-2015 3:15:00 PM
 */
	
class USML_DECLSPEC receiver_params_map : public singleton_map <const paramsIDType, const receiver_params*>
{
public:
    /**
     * Destructor - See singleton_map destructor.
     */
    virtual ~receiver_params_map();

protected:
    /**
     * Default Constructor
     *   Protected to prevent access.
     */
	receiver_params_map();

private:
	/**
     * Prevent access to copy constructor
     */
	receiver_params_map(receiver_params_map const&);

    /**
     * Prevent access to assignment operator
     */
	receiver_params_map& operator=(receiver_params_map const&);

};

typedef singleton_map<const paramsIDType,const receiver_params*>* receiver_params_map_ptr;

/// @}
} // end of namespace sensors
} // end of namespace usml
