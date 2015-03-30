/**
 * @file sensor_params.h
 * Attributes common to both source_params and receiver_params classes.
 */
#pragma once

#include <usml/usml_config.h>

namespace usml {
namespace sensors {

/// @ingroup sensors
/// @{

/**
 * Attributes common to both source_params and receiver_params classes.
 * All of the attributes in this class are immutable.
 */
class USML_DECLSPEC sensor_params {
public:

	/**
	 * Data type used for paramsID.
	 */
	typedef int id_type;

	/**
	 * Identification used to find this sensor type in
	 * source_params_map and/or receiver_params_map.
	 */
	sensor_params::id_type paramsID() const {
		return _paramsID;
	}

	/**
	 * Bistatic sensor_pair objects are only created for sources and receivers
	 * that have this flag set to true.  Set to false for monostatic sensors.
	 */
	bool multistatic() const {
		return _multistatic;
	}

protected:

	/**
	 * Protect constructor so that only sub-classes may call it.
	 *
	 * @param	paramsID		Identification used to find this sensor type in
	 * 							source_params_map and/or receiver_params_map.
	 * @param	multistatic		Bistatic sensor_pair objects are only created
	 * 							for sources and receivers that have this flag
	 * 							set to true.  Set to false for monostatic sensors.
	 */
	sensor_params( sensor_params::id_type paramsID, bool multistatic )
		: _paramsID( paramsID ), _multistatic(multistatic)
	{
	}

private:

	/**
	 * Identification used to find this sensor type in
	 * source_params_map and/or receiver_params_map.
	 */
	const sensor_params::id_type _paramsID;

	/**
	 * Bistatic sensor_pair objects are only created for sources and receivers
	 * that have this flag set to true.  Set to false for monostatic sensors.
	 */
	const bool _multistatic;
};

/// @}
}// end of namespace sensors
} // end of namespace usml
