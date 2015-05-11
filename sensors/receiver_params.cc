/**
 * @file receiver_params.cc
 * Sensor characteristics for the receiver behaviors of a sensor.
 */
#include <usml/sensors/receiver_params.h>

using namespace usml::sensors;

/**
 * Construct new class of receiver.
 */
receiver_params::receiver_params(sensor_params::id_type paramsID, 
    double min_freq, double max_freq, const seq_vector& frequencies, 
    const beam_pattern_list& beam_list, bool multistatic)
    : sensor_params(paramsID, min_freq, max_freq, frequencies, beam_list, multistatic)
{
}
