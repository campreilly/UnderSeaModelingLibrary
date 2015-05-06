/**
 * @file receiver_params.h
 * Sensor characteristics for the receiver behaviors of a sensor.
 */
#pragma once

#include <usml/sensors/sensor_params.h>


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
	 * Construct new class of receiver.
	 *
	 * @param	paramsID	    Identification used to find this sensor type
	 * 						    in receiver_params_map.
     * @param	min_freq		Minimum active frequency for the sensor. Lower
     *                          active bound of the sensor.
     * @param	max_freq		Maximum active frequency for the sensor. Upper
     *                          active bound of the sensor.
     * @param	frequencies		Operating frequencies that this sensor will listen.
     * 							This is cloned during construction.
	 * @param   beam_list	    List of beamIds associated with this sensor.
     * 						    The actual beams are extracted from beam_pattern_map
     * 						    using these beamIDs.
     * @param   multistatic	    Optional. Defaults to true. 
     *                          When true, this receiver will pair up with
	 * 						    all other sources in the reverberation model.
	 * 						    When false, it will only pair up with its own source.
	 */
    receiver_params(sensor_params::id_type paramsID, double min_freq, 
        double max_freq, const seq_vector& frequencies, 
        const beam_pattern_list& beam_list, bool multistatic = true);

	/**
	 * Clone receiver parameters from an existing receiver class.
	 */
    receiver_params(const receiver_params& other)
        : sensor_params(other) 
    {}

	/**
	 * Virtual destructor
	 */
	virtual ~receiver_params() {
	}
};

/// @}
}// end of namespace sensors
} // end of namespace sensors
