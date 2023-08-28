/**
 * @file rvbenv_model.cc
 * Computes the reverberation envelope time series for a single combination of
 * receiver azimuth, source beam number, receiver beam number.
 */

#include <usml/rvbenv/rvbenv_model.h>
#include <usml/types/seq_vector.h>
#include <usml/ublas/math_traits.h>
#include <usml/ublas/matrix_math.h>
#include <usml/ublas/vector_math.h>

#include <boost/numeric/ublas/fwd.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_expression.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_expression.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <cmath>
#include <complex>
#include <cstddef>
#include <ctime>
#include <iostream>

using namespace usml::rvbenv;
using namespace usml::types;
using namespace usml::ublas;

//#define DEBUG_ENVELOPE

/**
 * Initialize model from arguments passed to rvbenv_collection.
 */
rvbenv_model::rvbenv_model(const seq_vector::csptr& envelope_freq,
                           const seq_vector::csptr& travel_time)
    : _envelope_freq(envelope_freq),
      _travel_time(travel_time),
      _level(travel_time->size()),
      _power(envelope_freq->size()),
      _intensity(envelope_freq->size(), travel_time->size()) {}

/**
 * Computes the intensity for a single combination of source and receiver
 * eigenverbs.
 */
bool rvbenv_model::add_intensity(const eigenverb_model::csptr& src_verb,
                                     const eigenverb_model::csptr& rcv_verb,
                                     const vector<double>& scatter, double xs2,
                                     double ys2) {
    compute_overlap(src_verb, rcv_verb, scatter, xs2, ys2);
    compute_time_series(src_verb->time, rcv_verb->time);
}

/**
 * Compute the total power of the overlap between two eigenverbs.
 */
bool rvbenv_model::compute_overlap(const eigenverb_model::csptr& src_verb,
                                   const eigenverb_model::csptr& rcv_verb,
                                   const vector<double>& scatter, double xs2,
                                   double ys2) {
#ifdef DEBUG_ENVELOPE
    cout << "wave_queue::compute_overlap() " << endl
         << "\txs2=" << xs2 << " ys2=" << ys2 << " scatter=" << scatter << endl
         << "\tsrc_verb"
         << " t=" << src_verb->time << " de=" << to_degrees(src_verb->source_de)
         << " az=" << to_degrees(src_verb->source_az)
         << " direction=" << to_degrees(src_verb->direction)
         << " grazing=" << to_degrees(src_verb->grazing) << endl
         << "\tpower=" << 10.0 * log10(src_verb->power)
         << " length=" << src_verb->length << " width=" << src_verb->width
         << endl
         << "\tsurface=" << src_verb->surface << " bottom=" << src_verb->bottom
         << " caustic=" << src_verb->caustic << endl
         << "\trcv_verb"
         << " t=" << rcv_verb->time << " de=" << to_degrees(rcv_verb->source_de)
         << " az=" << to_degrees(rcv_verb->source_az)
         << " direction=" << to_degrees(rcv_verb->direction)
         << " grazing=" << to_degrees(rcv_verb->grazing) << endl
         << "\tpower=" << 10.0 * log10(rcv_verb->power)
         << " length=" << rcv_verb->length << " width=" << rcv_verb->width
         << endl
         << "\tsurface=" << rcv_verb->surface << " bottom=" << rcv_verb->bottom
         << " caustic=" << rcv_verb->caustic << endl;
#endif

    // determine the relative tilt between the projected Gaussians

    const double alpha = src_verb->direction - rcv_verb->direction;
    const double cos2alpha = cos(2.0 * alpha);
    const double sin2alpha = sin(2.0 * alpha);

    // compute commonly used terms in the intersection of the Gaussian profiles

    auto src_length2 = src_verb->length * src_verb->length;
    auto src_width2 = src_verb->width * src_verb->width;
    const double src_sum = src_length2 + src_width2;
    const double src_diff = src_length2 - src_width2;
    const double src_prod = src_length2 * src_width2;

    auto rcv_length2 = rcv_verb->length * rcv_verb->length;
    auto rcv_width2 = rcv_verb->width * rcv_verb->width;
    const double rcv_sum = rcv_length2 + rcv_width2;
    const double rcv_diff = rcv_length2 - rcv_width2;
    const double rcv_prod = rcv_length2 * rcv_width2;

    // compute the scaling of the exponential
    // equations (26) and (28) from the paper

    double det_sr = 0.5 * (2.0 * (src_prod + rcv_prod) + (src_sum * rcv_sum) -
                           (src_diff * rcv_diff) * cos2alpha);
    _power = 0.25 * 0.5 * src_verb->power * rcv_verb->power * scatter;

    // compute the power of the exponential
    // equation (28) from the paper

    const double new_prod = src_diff * cos2alpha;
    const double kappa = -0.25 *
                         (xs2 * (src_sum + new_prod + 2.0 * rcv_length2) +
                          ys2 * (src_sum - new_prod + 2.0 * rcv_width2) -
                          2.0 * sqrt(xs2 * ys2) * src_diff * sin2alpha) /
                         det_sr;
#ifdef DEBUG_ENVELOPE
    cout << "\tsrc_verb->power=" << src_verb->power
         << " rcv_verb->power=" << rcv_verb->power << endl
         << "\tdet_sr=" << det_sr << " kappa=" << kappa
         << " power=" << (10.0 * log10(_power)) << endl;
#endif
    _power *= exp(kappa) / sqrt(det_sr);

    // compute the square of the duration of the overlap
    // equation (41) from the paper

    det_sr = det_sr / (src_prod * rcv_prod);
    _duration = 0.5 *
                ((1.0 / src_width2 + 1.0 / src_length2) +
                 (1.0 / src_width2 - 1.0 / src_length2) * cos2alpha +
                 2.0 / rcv_width2) /
                det_sr;

    // compute duration of the overlap
    // equation (33) from the paper

    const double factor = cos(rcv_verb->grazing) / rcv_verb->sound_speed;
    _duration = 0.5 * factor * sqrt(_duration);
#ifdef DEBUG_ENVELOPE
    cout << "\tcontribution"
         << " duration=" << _duration << " power=" << (10.0 * log10(_power))
         << endl;
#endif
}

/**
 * Computes Gaussian time series contribution given delay, duration, and
 * total power.
 */
void rvbenv_model::compute_time_series(double src_verb_time,
                                       double rcv_verb_time) {
    _intensity.clear();

    for (size_t f = 0; f < _envelope_freq->size(); ++f) {
        // compute the peak time and intensity

        const double delay = src_verb_time + rcv_verb_time + _duration;
        const double scale = _power[f] / _duration;

        // use uBLAS vector proxy for current frequency row

        matrix_row<matrix<double> > intensity(_intensity, f);

        // use uBLAS vector proxies to only compute the portion of the
        // time series within +/- five (5) times the duration
        // speeds up the computation by over a factor of 3

        _level.clear();
        size_t first = _travel_time->find_index(delay - 5.0 * _duration);
        size_t last = _travel_time->find_index(delay + 5.0 * _duration) + 1;
        range window(first, last);
        vector<double> time_data = _travel_time->data();
        vector_range<vector<double> > time(time_data, window);
        vector_range<vector<double> > portion(_level, window);
        portion = scale * exp(-0.5 * abs2((time - delay) / _duration));
        intensity = _level;
    }
}
