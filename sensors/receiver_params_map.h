/**
 *  @file receiver_params_map.h
 *  Definition of the Class receiver_params_map
 *  Created on: 10-Feb-2015 12:49:09 PM
 */

#pragma once

#include <map>

#include <usml/usml_config.h>
#include <usml/sensors/paramsIDType.h>
#include <usml/sensors/receiver_params.h>

namespace usml {
namespace sensors {

/**
 * @version 1.0
 * @created 10-Feb-2015 12:49:09 PM
 */
class USML_DECLSPEC receiver_params_map : public std::map<paramsIDType, receiver_params>
{

public:
    
    /**
     * Singleton Constructor 
     *   Creates receiver_params_map instance just once, then accesable everywhere.
     */
	static receiver_params_map* instance();

    /**
     * Destructor
     */
    virtual ~receiver_params_map();

    /**
	 * 
	 * @param receiverID
	 */
    receiver_params* find(const paramsIDType receiverID);

    /**
	 * 
	 * @param receiverID
	 * @param receiver_params
	 */
    void insert(const paramsIDType receiverID, const receiver_params receiver_params);

protected:
    /**
     * Default Constructor 
     *   Protected to prevent access other than instance call 
     */
	receiver_params_map();

private:
    /**
    * The singleton access pointer
    */
	static receiver_params_map* _instance;

};

} // end of namespace sensors
} // end of namespace usml