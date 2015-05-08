/**
 * @file source_params.cc
 * Sensor characteristics for the source behaviors of a sensor.
 */
#include <usml/sensors/source_params.h>

using namespace usml::sensors ;

/**
 * Construct new class of source.
 */
source_params::source_params( sensor_params::id_type paramsID,
	vector<double> source_level, double min_freq, double max_freq, 
    const seq_vector& frequencies, const beam_pattern_list& beam_list, bool multistatic)
    : sensor_params(paramsID, min_freq, max_freq, frequencies, beam_list, multistatic),
  _source_level( source_level )
{
}
