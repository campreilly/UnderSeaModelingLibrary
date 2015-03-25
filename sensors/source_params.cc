/**
 * @file source_params.cc
 * Sensor characteristics for the source behaviors of a sensor.
 */
#include <usml/sensors/source_params.h>
#include <usml/sensors/beam_pattern_map.h>
#include <algorithm>

using namespace usml::sensors;

/**
 * Construct new class of source.
 */
source_params::source_params(source_params::id_type sourceID, double source_level,
	const seq_vector& frequencies, beam_pattern_model::id_type beamID)
	: _sourceID(sourceID),
	  _source_level(source_level)
{
	_frequencies.reset( frequencies.clone() ) ;
	_beam_pattern = beam_pattern_map::instance()->find(beamID) ;
}

/**
 * Clone source parameters from an existing source class.
 */
source_params::source_params(const source_params& other)
	: _sourceID(other._sourceID),
	  _source_level(other._source_level)
{
	_frequencies.reset( other._frequencies->clone() ) ;
	_beam_pattern = other._beam_pattern ;
}
