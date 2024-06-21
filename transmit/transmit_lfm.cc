/**
 * @file transmit_lfm.cc
 * Linear frequency modulated (LFM) transmission.
 */

#include <usml/transmit/transmit_lfm.h>
#include <usml/types/seq_linear.h>
#include <usml/types/seq_vector.h>
#include <usml/ublas/math_traits.h>
#include <usml/ublas/vector_math.h>

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_expression.hpp>
#include <cmath>

using namespace usml::transmit;

/**
 * Waveform parameters for a linear frequency modulated pulse.
 */
transmit_lfm::transmit_lfm(const std::string& type, double duration,
                           double bandwidth, double fcenter, double delay,
                           double source_level, window::type window,
                           double window_param, double orderedAZ,
                           double orderedDE, int transmit_mode)
    : transmit_model(type, duration, bandwidth, fcenter, delay, source_level,
                     window, window_param, orderedAZ, orderedDE,
                     transmit_mode) {}

/**
 * Creates a complex analytic signal for this waveform.
 */
cdvector transmit_lfm::asignal(double fsample, double fband, double inphase,
                               double* outphase) {
    const int N = int(round(duration * fsample));
    const double T = N / fsample;
    const double omega = TWO_PI * (fcenter - 0.5 * bandwidth - fband);
    const double slope = 0.5 * TWO_PI * bandwidth / T;
    if (outphase != nullptr) {
        *outphase = fmod(slope * T * T + omega * T + inphase, TWO_PI);
    }
    vector<double> t = seq_linear(0.0, 1.0 / fsample, N).data();
    cdvector signal = usml::ublas::asignal(slope * usml::ublas::abs2(t) +
                                           omega * t + inphase);
    add_window(signal);
    return signal;
}
