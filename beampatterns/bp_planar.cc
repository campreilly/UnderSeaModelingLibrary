/**
 * @file bp_planar.cc
 * Models the closed form solution of a planar array.
 */

#include <usml/beampatterns/bp_planar.h>
#include <usml/types/bvector.h>
#include <usml/types/seq_vector.h>
#include <usml/ublas/vector_math.h>

#include <boost/numeric/ublas/expression_types.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_expression.hpp>
#include <cmath>

using namespace usml::beampatterns;

/**
 * Constructs a vertical or horizontal beam pattern.
 */
bp_planar::bp_planar(unsigned num_elem_up, double spacing_up,
                     unsigned num_elem_right, double spacing_right,
                     bool back_baffle)
    : _num_elem_up(num_elem_up),
      _spacing_up(spacing_up),
      _num_elem_right(num_elem_right),
      _spacing_right(spacing_right),
      _directivity(2.0 / 3.0 * M_PI * M_PI * _spacing_right * _spacing_up *
                   sqrt((_num_elem_right * _num_elem_right - 1) *
                        (_num_elem_up * _num_elem_up - 1))),
      _back_baffle(back_baffle) {}

/**
 * Computes the beam level gain for an arrival vector in the body coordinates
 * of the array,
 */
void bp_planar::beam_level(const bvector& arrival,
                           const seq_vector::csptr& frequencies, vector<double>* level,
                           const bvector& steering, double sound_speed) const {
    // set gain to zero in backplane when baffle is on
    if (_back_baffle) {
        if (arrival.front() <= 0.0) {
            *level *= 0.0;
            return;
        }
    }

    // compute line array in up direction
    double dot = arrival.up() - steering.up();
    vector<double> kd = frequencies->data();
    kd *= M_PI * _spacing_up / sound_speed * dot;
    *level = abs2(element_div(sin(_num_elem_up * kd) + 1e-200,
                              sin(kd) * _num_elem_up + 1e-200));

    // compute line array in right direction
    dot = arrival.right() - steering.right();
    kd = frequencies->data();
    kd *= M_PI * _spacing_right / sound_speed * dot;
    *level = element_prod(
        *level, abs2(element_div(sin(_num_elem_right * kd) + 1e-200,
                                 sin(kd) * _num_elem_right + 1e-200)));
}

/**
 * Computes the directivity gain for planar array.
 */
void bp_planar::directivity(const seq_vector::csptr& frequencies,
                            vector<double>* level, const bvector& steering,
                            double sound_speed) const {
    const double dot = steering.front();
    *level = frequencies->data();
    *level = sound_speed / (*level);
    *level = abs2(*level) / (dot * _directivity);
    if (_back_baffle) {
        *level *= 0.5;
    }
}
