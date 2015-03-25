/**
 * @file source_params.h
 * Sensor characteristics for the source behaviors of a sensor.
 */
#pragma once

//#include <usml/usml_config.h>
#include <usml/sensors/beam_pattern_model.h>
//#include <list>
//#include <map>

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
 * in the source_params_map.  Then, each sensor makes a clone of these
 * parameters for its own use.
 *
 */
class USML_DECLSPEC source_params {
public:

	/**
	 * Data type used for sensorID.
	 */
	typedef int id_type;

	/**
	 * Data type used for reference to source_params.
	 */
	typedef unique_ptr<const source_params> const_reference;

	/**
	 * Data type used for store beam patterns in this sensor.
	 */
	typedef std::map<beam_pattern_model::id_type, beam_pattern_model::reference> beam_pattern_container;

	/**
	 * Construct new class of source.
	 *
	 * @param	sensorID		Identification used to find this sensor type
	 * 							in source_params_map.
	 * @param   source_level	Peak intensity of the transmitted pulse
	 * 							(dB//uPa@1m)
	 * @param	frequencies		Frequencies of transmitted pulse.
	 * 							Multiple frequencies can be used to compute
	 * 							multiple results at the same time.
	 * 							These are the frequencies at which transmission
	 * 							loss and reverberation are computed.
	 * 							This is cloned during construction.
	 * @param   beamID			Reference to the beam pattern used during
	 * 							transmission.
	 */
	source_params(source_params::id_type sensorID, double source_level,
		const seq_vector& frequencies, beam_pattern_model::id_type beamID);

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
	 * Identification used to find this sensor type in source_params_map.
	 */
	source_params::id_type sourceID() const {
		return _sourceID;
	}

	/**
	 * Identification used to find this sensor type in source_params_map.
	 */
	void sourceID(source_params::id_type sensorID) {
		_sourceID = sensorID;
	}

	/**
	 * Set a beam pattern for the source using beamID.
	 * @params beamID to be used for this source.
	 */
	void beam_pattern(beam_pattern_model::id_type beamID);

private:

	/**
	 * Prevent access to default constructor
	 */
	source_params();

	/**
	 * Prevent access to assignment operator
	 */
	source_params& operator=(source_params const&);

	/**
	 * Identification used to find this sensor type in source_params_map.
	 */
	id_type _sourceID;

	/**
	 * Peak intensity of the transmitted pulse (dB//uPa@1m)
	 */
	double _source_level;

	/**
	 * Frequencies of transmitted pulse. Multiple frequencies can be
	 * used to compute multiple results at the same time. These are the
	 * frequencies at which transmission loss and reverberation are computed.
	 */
	unique_ptr<seq_vector> _frequencies ;

	/**
	 * Shared reference to the beam patterns for this source.
	 */
	beam_pattern_model::reference _beam_pattern;
};

/// @}
}// end of namespace sensors
} // end of namespace usml
