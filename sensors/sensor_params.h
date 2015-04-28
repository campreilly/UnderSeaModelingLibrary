/**
 * @file sensor_params.h
 * Attributes common to both source_params and receiver_params classes.
 */
#pragma once

#include <list>

#include <usml/threads/smart_ptr.h>

#include <usml/usml_config.h>
#include <usml/types/seq_vector.h>
#include <usml/sensors/beam_pattern_model.h>
#include <usml/sensors/beam_pattern_map.h>

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
     * Data type used for store beam patterns in this sensor.
     */
    typedef std::list<beam_pattern_model::id_type> beam_pattern_list;

	/**
	 * Identification used to find this sensor type in
	 * source_params_map and/or receiver_params_map.
	 */
	sensor_params::id_type paramsID() const {
		return _paramsID;
	}

    /**
     * Gets the minimum active frequency of the sensor. 
     * Lower active bound of the sensor.
     * @return minium active frequency of the sensor.
     */
    double min_active_freq()  {
        return _min_active_freq;
    }

    /**
    * Gets the maximum active frequency of the sensor.
    * Upper active bound of the sensor.
    * @return maxium active frequency of the sensor.
    */
    double max_active_freq() {
        return _max_active_freq;
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
     * Const reference to the beam pattern container.
     */
    size_t num_patterns() const {
        return _beam_list.size();
    }

    /**
     * Const reference to the beam pattern container.
     */
    const beam_pattern_list& beam_list() const {
        return _beam_list;
    }

    /**
     * Searchs the beam pattern list for a specific beam pattern
     * with the requested ID
     * @param beamID ID for the beam_pattern to return.
     * @return beam_pattern shared pointer
     */
    beam_pattern_model::reference beam_pattern(beam_pattern_model::id_type beamID) const {
        if ( std::find(_beam_list.begin(), _beam_list.end(), beamID)
            == _beam_list.end() ) {
            return beam_pattern_model::reference();
        }
        return beam_pattern_map::instance()->find(beamID);
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
     * @param	min_freq		Minimum active frequency for the sensor. Lower
     *                          active bound of the sensor.
     * @param	max_freq		Maximum active frequency for the sensor. Upper
     *                          active bound of the sensor.
     * @param	frequencies		Frequencies of transmitted pulse.
     * 							Multiple frequencies can be used to compute
     * 						    multiple results at the same time.
     * 							These are the frequencies at which transmission
     * 							loss and reverberation are computed.
     * 							This is cloned during construction.
     * @param   beam_list	    List of beamIds associated with this sensor.
     * 						    The actual beams are extracted from beam_pattern_map
     * 						    using these beamIDs.
	 * @param	multistatic		Optional. Defaults to true.
     *                          Only requires setting for sensor's in which 
     *                          the mode is BOTH. Must be set true for sensor's
     *                          of mode SOURCE or RECEIVER.
     *                          Bistatic sensor_pair objects are only created
	 * 						    for sources and receivers that have this flag
	 * 							set to true. Set to false for monostatic sensors.
	 */
    sensor_params(sensor_params::id_type paramsID, double max_freq, double min_freq, 
        const seq_vector& frequencies, const beam_pattern_list& beam_list, 
        bool multistatic = true)
        : _paramsID(paramsID), _min_active_freq( min_freq), _max_active_freq(max_freq),
        _frequencies(frequencies.clone()), _beam_list(beam_list),
        _multistatic(multistatic)
	{
	}

    /**
     * Clone sensor parameters from an existing sensor.
     */
    sensor_params(const sensor_params& other)
        : _paramsID(other._paramsID), _min_active_freq(other._min_active_freq),
        _max_active_freq(other._max_active_freq), _frequencies(other._frequencies.get()),
        _beam_list(other._beam_list), _multistatic(other._multistatic) 
    {}

private:

	/**
	 * Identification used to find this sensor type in
	 * source_params_map and/or receiver_params_map.
	 */
	sensor_params::id_type _paramsID;

    /**
     * Minimum active frequency for the sensor.
     */
    double _min_active_freq;

    /**
     *  Maximum active frequency for the sensor.
     */
    double _max_active_freq;

    /**
     * Frequencies of transmitted pulse. Multiple frequencies can be
     * used to compute multiple results at the same time. These are the
     * frequencies at which transmission loss and reverberation are computed.
     */
    const unique_ptr<seq_vector> _frequencies;

    /**
     * List of all beam pattern IDs associated with this receiver
     * parameter.
     */
    beam_pattern_list _beam_list;

	/**
	 * Bistatic sensor_pair objects are only created for sources and receivers
	 * that have this flag set to true.  Set to false for monostatic sensors.
	 */
	const bool _multistatic;
};

/// @}
}// end of namespace sensors
} // end of namespace usml
