/**
 * @file source_params.cc
 * Sensor characteristics for the source behaviors of a sensor.
 */
#include <usml/sensors/source_params.h>
#include <usml/sensors/beam_pattern_map.h>
#include <algorithm>

using namespace usml::sensors ;

/**
 * Construct new class of source.
 */
source_params::source_params( sensor_params::id_type paramsID,
	vector<double> source_level, const seq_vector& frequencies,
    beam_pattern_model::id_type beamID, bool multistatic)
    : sensor_params(paramsID, frequencies, multistatic),
  _source_level( source_level ),
  _beam_pattern( beam_pattern_map::instance()->find(beamID) )
{

}
