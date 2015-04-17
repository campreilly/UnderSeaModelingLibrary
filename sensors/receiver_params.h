/**
 * @file receiver_params.h
 * Sensor characteristics for the receiver behaviors of a sensor.
 */
#pragma once

#include <usml/sensors/sensor_params.h>
#include <usml/sensors/beam_pattern_map.h>
#include <algorithm>
#include <list>

namespace usml {
namespace sensors {

/// @ingroup sensors
/// @{

/**
 * Sensor characteristics for the receiver behaviors of a sensor.
 *
 * Initially, the sonar training system passes this information to the
 * reverberation model, and the reverberation model stores this information
 * in the receiver_params_map.  Then, each sensor gets a share_ptr reference
 * to these parameters for its own use.
 *
 * All of the attributes in this class are immutable.
 */
class USML_DECLSPEC receiver_params : public sensor_params {
public:

	/**
	 * Data type used for reference to receiver_params.
	 */
	typedef shared_ptr<receiver_params> reference;

	/**
	 * Data type used for store beam patterns in this sensor.
	 */
	typedef std::list<beam_pattern_model::id_type> beam_pattern_list ;

	/**
	 * Construct new class of receiver.
	 *
	 * @param	paramsID	    Identification used to find this sensor type
	 * 						    in receiver_params_map.
     * @param	frequencies		Operating frequencies that this sensor will listen.
     * 							This is cloned during construction.
	 * @param   beamList	    List of beamIds associated with this receiver class.
	 * 						    The actual beams are extracted from beam_pattern_map
	 * 						    using these beamIDs.
     * @param   multistatic	    Optional. Defaults to true. 
     *                          When true, this receiver will pair up with
	 * 						    all other sources in the reverberation model.
	 * 						    When false, it will only pair up with its own source.
	 */
    receiver_params(sensor_params::id_type paramsID, const seq_vector& frequencies,
        const beam_pattern_list& beamList, bool multistatic = true);

	/**
	 * Clone receiver parameters from an existing receiver class.
	 */
	receiver_params(const receiver_params& other) ;

	/**
	 * Virtual destructor
	 */
	virtual ~receiver_params() {
	}

	/**
	 * Const reference to the beam pattern container.
	 */
	size_t num_patterns() const {
		return _beam_list.size() ;
	}

	/**
	 * Shared reference to a single beam in the pattern pattern.
	 */
	beam_pattern_model::reference beam_pattern( beam_pattern_model::id_type beamID ) const ;

	/**
	 * Const reference to the beam pattern container.
	 */
	const beam_pattern_list& beam_list() const {
		return _beam_list ;
	}

private:

	/**
	 * List of all beam pattern IDs associated with this receiver
	 * parameter.
	 */
	beam_pattern_list _beam_list ;
};

/// @}
}// end of namespace sensors
} // end of namespace sensors
