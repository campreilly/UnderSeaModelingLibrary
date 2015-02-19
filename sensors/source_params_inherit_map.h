/**
 *  @file source_params_inherit_map.h
 *  Definition of the Class source_params_inherit_map
 *  Created on: 10-Feb-2015 12:49:09 PM
 */

#pragma once

#include <map>

#include <usml/usml_config.h>
#include <usml/sensors/paramsIDType.h>
#include <usml/sensors/source_params.h>

namespace usml {
namespace sensors {

/// @ingroup sensors
/// @{

/**
 * Storage for all the source_params's in use by the USML. It is defined as a GOF
 * singleton pattern, that is std::map.
 * std::map is a sorted associative container that contains key-value pairs with unique keys.
 * A typedef of paramsIDType has been defined to allow for modification of the Key of the map
 * at a later time if needed.
 *
/**
 * @author Ted Burns, AEgis Technologies Inc.
 * @version 1.0
 * @created 10-Feb-2015 12:49:09 PM
 */
class USML_DECLSPEC source_params_inherit_map : public std::map<const paramsIDType, const source_params*>
{

public:
   /**
    * Singleton Constructor - Creates beam_pattern_map instance just once, then
    * Accessible everywhere.
    * @return  pointer to the instance of the source_params_inherit_map.
    */
	static source_params_inherit_map* instance();

	/**
     * Destructor - Deletes pointers to beam_pattern_model's
     */
    virtual ~source_params_inherit_map();

protected:
    /**
     * Default Constructor
     *   Protected to prevent access.
     */
	source_params_inherit_map();

private:
    /**
     * The singleton access pointer
     */
	static source_params_inherit_map* _instance;

};

/// @}
} // end of namespace sensors
} // end of namespace usml
