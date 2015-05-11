/**
 * @file source_params.h
 * Sensor characteristics for the source behaviors of a sensor.
 */
#pragma once

#include <usml/sensors/sensor_params.h>

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
	 * 							(dB//uPa at 1m)
	 * @param  	pulse_length	Duration of the transmitted signal (sec).
	 * @param   reverb_duration Duration of the reverberation envelope (sec).
     * @param	min_freq		Minimum active frequency for the sensor. Lower
     *                          active bound of the sensor.
     * @param	max_freq		Maximum active frequency for the sensor. Upper
     *                          active bound of the sensor.
	 * @param	frequencies		Frequencies of transmitted pulse.
	 * 							Multiple frequencies can be used to compute
	 * 							multiple results at the same time.
	 * 							These are the frequencies at which transmission
	 * 							loss and reverberation are computed.
	 * 							This is cloned during construction.
	 * @param   beam_list	    List of beamIds associated with this sensor.
     * 						    The actual beams are extracted from beam_pattern_map
     * 						    using these beamIDs.
     * @param	multistatic		Optional, defaults to true. 
     *                          Bistatic sensor_pair objects are only created
	 * 							for sources and receivers that have this flag
	 * 							set to true.  Set to false for monostatic sensors.
	 */
	source_params( sensor_params::id_type paramsID, vector<double> source_level,
			double pulse_length, double reverb_duration,
            double min_freq, double max_freq, const seq_vector& frequencies, 
            const beam_pattern_list& beam_list, bool multistatic = true);
	/**
	 * Clone source parameters from an existing source class.
	 */
    source_params(const source_params& other)
        : sensor_params(other), _source_level(other._source_level) 
    {}

	/**
	 * Virtual destructor
	 */
	virtual ~source_params() {
	}

	/**
	 * Peak intensity of the transmitted pulse (dB//uPa at 1m)
	 */
	vector<double> source_level() const {
		return _source_level ;
	}

	/**
	 * Duration of the transmitted signal (sec).
	 */
	double pulse_length() {
		return _pulse_length ;
	}

	/**
	 * Duration of the reverberation envelope (sec).
	 */
	double reverb_duration() {
		return _reverb_duration  ;
	}

private:

	/**
	 * Peak intensity of the transmitted pulse (dB//uPa at 1m)
	 */
	const vector<double> _source_level;

	/**
	 * Duration of the transmitted signal (sec).
	 */
	double _pulse_length ;

	/**
	 * Duration of the reverberation envelope (sec).
	 */
	double _reverb_duration ;

};

/// @}
}// end of namespace sensors
} // end of namespace usml
