/**
 * @file transmit_cw.cc
 * Pulsed continuous wave (CW) transmission.
 */

#include <usml/transmit/transmit_cw.h>
#include <usml/types/seq_linear.h>
#include <usml/types/seq_vector.h>
#include <usml/ublas/math_traits.h>
#include <usml/ublas/vector_math.h>

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_expression.hpp>
#include <cmath>

using namespace usml::transmit;

/**
 * Waveform parameters for a pulsed continuous wave pulse.
 */
transmit_cw::transmit_cw(const std::string& type, double duration,
                         double fcenter, double delay, double source_level,
                         window::type window, double window_param,
                         double orderedAZ, double orderedDE, int transmit_mode)
    : transmit_model(type, duration, fcenter, 1.0 / duration, delay,
                     source_level, window, window_param, orderedAZ, orderedDE,
                     transmit_mode) {}

/**
 * Creates a complex analytic signal for this waveform.
 */
cdvector transmit_cw::asignal(double fsample, double fband, double inphase,
                              double* outphase) {
    const int N = int(round(duration * fsample));
    const double T = N / fsample;
    const double omega = TWO_PI * (fcenter - fband);
    if (outphase != nullptr) {
        *outphase = fmod(omega * T + inphase, TWO_PI);
    }
    vector<double> t = seq_linear(0.0, 1.0 / fsample, N).data();
    cdvector signal = usml::ublas::asignal(omega * t + inphase);
    add_window(signal);
    return signal;
}
