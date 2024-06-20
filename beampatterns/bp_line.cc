/**
 * @file bp_line.cc
 * Vertical and horizontal line arrays in closed form.
 */
#include <usml/beampatterns/bp_line.h>

using namespace usml::beampatterns;

/**
 * Computes the beam level gain for an arrival vector in the body coordinates
 * of the array,
 */
void bp_line::beam_level(const bvector& arrival,
                         const seq_vector::csptr& frequencies,
                         vector<double>* level, const bvector& steering,
                         double sound_speed) const {
    const double dot = (_type == bp_line_type::HLA)
                           ? arrival.front() - steering.front()
                           : arrival.up() - steering.up();
    *level = frequencies->data();
    *level *= M_PI * _spacing / sound_speed * dot;
    *level = abs2(element_div(sin(_num_elements * (*level)) + 1e-200,
                              sin(*level) * _num_elements + 1e-200));
}

/**
 * Estimates the directivity gain for line array.
 */
void bp_line::directivity(const seq_vector::csptr& frequencies,
                          vector<double>* level, const bvector& steering,
                          double sound_speed) const {
    const scalar_vector<double> one(frequencies->size(), 1.0);

    // steering relative to first element
    const double str = (_type == HLA) ? steering.front() : steering.up();

    // collect constant terms related to element spacing
    vector<double> alpha = frequencies->data();
    alpha *= (M_PI * _spacing / sound_speed * 2.0);

    // compute double summation over number of elements
    noalias(*level) = scalar_vector<double>(frequencies->size(), 0.0);
    for (double n = 1.0; n <= _num_elements; ++n) {
        for (double m = 1.0; m <= _num_elements; ++m) {
            if (n == m) {
                *level += one;
            } else {
                const vector<double> anm(alpha * (n - m));
                *level +=
                    element_div(element_prod(cos(anm * str), sin(anm)), anm);
            }
        }
    }

    // normalize to number elements
    *level /= (_num_elements * _num_elements);
}
