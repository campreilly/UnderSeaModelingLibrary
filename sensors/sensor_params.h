/**
 * @file sensor_params.h
 * Attributes common to both source_params and receiver_params classes.
 */
#pragma once

#include <usml/threads/smart_ptr.h>

#include <usml/usml_config.h>
#include <usml/types/seq_vector.h>

namespace usml {
namespace sensors {

    using namespace usml::types;
    using namespace usml::threads;

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
     * Frequencies of transmitted pulse. Multiple frequencies can be
     * used to compute multiple results at the same time. These are the
     * frequencies at which transmission loss and reverberation are computed.
     */
    const seq_vector* frequencies() const {
        return _frequencies.get();
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
     * @param	frequencies		Frequencies of transmitted pulse.
     * 							Multiple frequencies can be used to compute
     * 							multiple results at the same time.
     * 							These are the frequencies at which transmission
     * 							loss and reverberation are computed.
     * 							This is cloned during construction.
	 * @param	multistatic		Optional. Defaults to true.
     *                          Only requires setting for sensor's with that 
     *                          the mode is BOTH. Must be set true for sensor's
     *                          of mode SOURCE or RECEIVER.
     *                          Bistatic sensor_pair objects are only created
	 * 							for sources and receivers that have this flag
	 * 							set to true.  Set to false for monostatic sensors.
	 */
    sensor_params(sensor_params::id_type paramsID, const seq_vector& frequencies, 
        bool multistatic = true)
        : _paramsID(paramsID), _frequencies(frequencies.clone()), _multistatic(multistatic)
	{
	}

private:

	/**
	 * Identification used to find this sensor type in
	 * source_params_map and/or receiver_params_map.
	 */
	sensor_params::id_type _paramsID;

    /**
     * Frequencies of transmitted pulse. Multiple frequencies can be
     * used to compute multiple results at the same time. These are the
     * frequencies at which transmission loss and reverberation are computed.
     */
    const unique_ptr<seq_vector> _frequencies;

	/**
	 * Bistatic sensor_pair objects are only created for sources and receivers
	 * that have this flag set to true.  Set to false for monostatic sensors.
	 */
	const bool _multistatic;
};

/// @}
}// end of namespace sensors
} // end of namespace usml
