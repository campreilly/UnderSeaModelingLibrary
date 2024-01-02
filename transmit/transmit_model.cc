/**
 * @file transmit_model.cc
 * Parameters for a single waveform in a transmission schedule.
 */

#include <usml/transmit/transmit_model.h>
#include <usml/ublas/vector_math.h>

#include <boost/numeric/ublas/expression_types.hpp>

using namespace usml::transmit;

/**
 * Parameters for a single waveform in a transmission schedule.
 */
transmit_model::transmit_model(const std::string& type, double duration,
                               double bandwidth, double fcenter, double delay,
                               double source_level, window::type window_type,
                               double window_param, double orderedAZ,
                               double orderedDE, int transmit_mode)
    : type(type),
      duration(duration),
      bandwidth(bandwidth),
      fcenter(fcenter),
      delay(delay),
      source_level(source_level),
      window_type(window_type),
      window_param(window_param),
      orderedAZ(orderedAZ),
      orderedDE(orderedDE),
      transmit_mode(transmit_mode) {}

/**
 * Add window to transmit pulse, in-place.
 */
void transmit_model::add_window(cdvector& signal) const {
    signal = signal * window::any(window_type, signal.size(), window_param);
}
