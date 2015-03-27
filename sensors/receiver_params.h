/**
 * @file receiver_params.h
 * Sensor characteristics for the receiver behaviors of a sensor.
 */
#pragma once

//#include <usml/usml_config.h>
#include <usml/sensors/beam_pattern_model.h>
#include <list>
#include <map>

namespace usml {
namespace sensors {

/// @ingroup sensors
/// @{

/**
 * Sensor characteristics for the receiver behaviors of a sensor.
 *
 * Initially, the sonar training system passes this information to the
 * reverberation model, and the reverberation model stores this information
 * in the receiver_params_map.  Then, each sensor makes a clone of these
 * parameters for its own use.
 */
class USML_DECLSPEC receiver_params {
public:

	/**
	 * Data type used for sensorID.
	 */
	typedef int id_type;

	/**
	 * Data type used for reference to receiver_params.
	 */
	typedef shared_ptr<const receiver_params> const_reference;

	/**
	 * Data type used for store beam patterns in this sensor.
	 */
	typedef std::map<beam_pattern_model::id_type, beam_pattern_model::reference> beam_pattern_container ;

	/**
	 * Construct new class of receiver.
	 *
	 * @param	sensorID	Identification used to find this sensor type
	 * 						in receiver_params_map.
	 * @param   bistatic	When true, this receiver will pair up with
	 * 						all other sources in the reverberation model.
	 * 						When false, it will only pair up with its own source.
	 * @param   beamList	List of beamIds associated with this receiver class.
	 * 						The actual beams are extracted from beam_pattern_map
	 * 						using these beamIDs.
	 */
	receiver_params(receiver_params::id_type sensorID, bool bistatic,
			const std::list<beam_pattern_model::id_type>& beamList);

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
	 * Identification used to find this sensor type in receiver_params_map.
	 */
	receiver_params::id_type receiverID() const {
		return _receiverID;
	}

	/**
	 * Identification used to find this sensor type in receiver_params_map.
	 */
	void receiverID(receiver_params::id_type receiverID) {
		_receiverID = receiverID;
	}

	/**
	 * When true, this receiver will pair up with all other sources
	 * in the reverberation model.  When false, it will only pair up
	 * with its own source.
	 */
	bool bistatic() const {
		return _bistatic;
	}

	/**
	 * When true, this receiver will pair up with all other sources
	 * in the reverberation model.  When false, it will only pair up
	 * with its own source.
	 */
	void bistatic(bool bistatic) {
		_bistatic = bistatic;
	}

	/**
	 * Add a beam pattern to the receiver using beamID.
	 * If the beamID does not exist in the beam_pattern_map
	 * a NULL beam is added to the list.
	 *
	 * @params beamID to be added to the list of receiver beams
	 */
	void add_beam_pattern(beam_pattern_model::id_type beamID);

	/**
	 * Delete a beam pattern to the receiver parameters using beamID.
	 * If the beamID does not exist in this sensor, this does nothing.
	 *
	 * @params beamID to be removed from the list of receiver beams
	 */
	void remove_beam_pattern(beam_pattern_model::id_type beamID);

	/**
	 * Const reference to the beam pattern container.
	 */
	const beam_pattern_container& beam_patterns() {
		return _beam_patterns ;
	}

private:

	/**
	 * Prevent access to default constructor
	 */
	receiver_params();

	/**
	 * Prevent access to assignment operator
	 */
	receiver_params& operator=(receiver_params&);

	/**
	 * Identification used to find this sensor type in receiver_params_map.
	 */
	id_type _receiverID;

	/**
	 * When true, this receiver will pair up with all other sources
	 * in the reverberation model.  When false, it will only pair up
	 * with its own source.
	 */
	bool _bistatic;

	/**
	 * Shared reference to the beam patterns for this receiver.
	 */
	beam_pattern_container _beam_patterns;
};

/// @}
}// end of namespace sensors
} // end of namespace sensors
