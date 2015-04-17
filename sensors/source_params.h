/**
 * @file source_params.h
 * Sensor characteristics for the source behaviors of a sensor.
 */
#pragma once

#include <usml/sensors/sensor_params.h>
#include <usml/sensors/beam_pattern_model.h>

using namespace boost::numeric::ublas ;

namespace usml {
namespace sensors {

/// @ingroup sensors
/// @{

/**
 * Sensor characteristics for the source behaviors of a sensor.
 * Each sensor_params object represents the settings for the transmission
 * of one kind of pulse and a single beam pattern. But, it supports an
 * option to compute results at multiple frequencies.
 *
 * Initially, the sonar training system passes this information to the
 * reverberation model, and the reverberation model stores this information
 * in the source_params_map.  Then, each sensor gets a share_ptr reference
 * to these parameters for its own use.
 * All of the attributes in this class are immutable.
 */
class USML_DECLSPEC source_params : public sensor_params {
public:

	/**
	 * Data type used for reference to source_params.
	 */
	typedef shared_ptr<source_params> reference;

	/**
	 * Construct new class of source.
	 *
	 * @param	paramsID		Identification used to find this sensor type in
	 * 							source_params_map and/or receiver_params_map.
	 * @param   source_level	Peak intensity of the transmitted pulse
	 * 							(dB//uPa@1m)
	 * @param	frequencies		Frequencies of transmitted pulse.
	 * 							Multiple frequencies can be used to compute
	 * 							multiple results at the same time.
	 * 							These are the frequencies at which transmission
	 * 							loss and reverberation are computed.
	 * 							This is cloned during construction.
	 * @param   beamID			Reference to the beam pattern used during
	 * 							transmission.  Will be NULL if beamID is not
	 * 							found in beam_pattern_map.
     * @param	multistatic		Optional, defaults to true. 
     *                          Bistatic sensor_pair objects are only created
	 * 							for sources and receivers that have this flag
	 * 							set to true.  Set to false for monostatic sensors.
	 */
	source_params( sensor_params::id_type paramsID, vector<double> source_level,
            const seq_vector& frequencies, beam_pattern_model::id_type beamID, 
            bool multistatic = true);
	/**
	 * Clone source parameters from an existing source class.
	 */
	source_params(const source_params& other) ;

	/**
	 * Virtual destructor
	 */
	virtual ~source_params() {
	}

	/**
	 * Peak intensity of the transmitted pulse (dB//uPa@1m)
	 */
	vector<double> source_level() const {
		return _source_level ;
	}

	/**
	 * Shared reference to the beam patterns for this source.
	 */
	beam_pattern_model::reference beam_pattern() const {
		return _beam_pattern ;
	}

private:

	/**
	 * Peak intensity of the transmitted pulse (dB//uPa@1m)
	 */
	const vector<double> _source_level;

	/**
	 * Shared reference to the beam patterns for this source.
	 */
	const beam_pattern_model::reference _beam_pattern;
};

/// @}
}// end of namespace sensors
} // end of namespace usml
