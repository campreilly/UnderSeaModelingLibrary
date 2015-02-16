/**
 *  @file source_params_map.h
 *  Definition of the Class source_params_map
 *  Created on: 10-Feb-2015 12:49:09 PM
 */

#pragma once

#include <map>

#include <usml/usml_config.h>
#include <usml/sensors/paramsIDType.h>
#include <usml/sensors/source_params.h>

namespace usml {
namespace sensors {

/**
 * @author tedburns
 * @version 0.1
 * @created 10-Feb-2015 12:49:09 PM
 */
class USML_DECLSPEC source_params_map : public std::map<paramsIDType, source_params*>
{

public:
    /**
    * Singleton Constructor
    *   Creates receiver_params_map instance just once, then accesable everywhere.
    */
	static source_params_map* instance();

    /**
    * Destructor
    */
    virtual ~source_params_map();

	/**
	 * 
	 * @param sourceID
	 * @param source_params
	 */
	void insert(const paramsIDType sourceID, const source_params* source_params);

	/**
	 * 
	 * @param sourceID
	 */
	source_params* find(const paramsIDType sourceID);

protected:
    /**
     * Default Constructor
     *   Protected to prevent access other than instance call
     */
	source_params_map();

private:
    /**
     * The singleton access pointer
     */
	static source_params_map* _instance;

};

} // end of namespace sensors
} // end of namespace usml