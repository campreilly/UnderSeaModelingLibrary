/**
 * @file receiver_params.cc
 * Sensor characteristics for the receiver behaviors of a sensor.
 */
#include <usml/sensors/receiver_params.h>
#include <usml/sensors/beam_pattern_map.h>
#include <boost/foreach.hpp>
#include <algorithm>

using namespace usml::sensors;

/**
 * Construct new class of receiver.
 */
receiver_params::receiver_params(receiver_params::id_type receiverID,
  bool bistatic, const std::list<beam_pattern_model::id_type>& beamList)
  : _receiverID(receiverID), _bistatic(bistatic)
{
	BOOST_FOREACH( beam_pattern_model::id_type beamID, beamList ) {
		add_beam_pattern( beamID );
	}
}

/**
 * Clone receiver parameters from an existing receiver class.
 */
receiver_params::receiver_params(const receiver_params& other)
	: _receiverID(other._receiverID), _bistatic(other._bistatic),
	  _beam_patterns(other._beam_patterns)
{}

/**
 * Add a beam pattern to the receiver parameters using beamID.
 */
void receiver_params::add_beam_pattern(beam_pattern_model::id_type beamID) {
	_beam_patterns[beamID] = beam_pattern_map::instance()->find(beamID) ;
}

/**
 * Delete a beam pattern to the receiver parameters using beamID.
 */
void receiver_params::remove_beam_pattern(beam_pattern_model::id_type beamID) {
	_beam_patterns.erase(beamID) ;
}

