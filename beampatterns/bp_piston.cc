/**
 * @file bp_piston.cc
 * Piston hydrophone model oriented along the front/forward axis
 */

#include <usml/beampatterns/bp_piston.h>
#include <usml/types/bvector.h>
#include <usml/types/seq_vector.h>

#include <boost/numeric/ublas/vector.hpp>
#include <cmath>
#include <memory>

using namespace usml::beampatterns;

/**
 * Computes the beam level gain for an arrival vector in the body coordinates
 * of the array,
 */
void bp_piston::beam_level(const bvector& arrival,
                           const seq_vector::csptr& frequencies,
                           vector<double>* level, const bvector& /* steering */,
                           double sound_speed) const {
    // set gain to zero in backplane when baffle is on
    if (_back_baffle) {
        if (arrival.front() <= 0.0) {
            *level *= 0.0;
            return;
        }
    }

    // analytic solution for beam pattern
    const double sinA = sqrt(1.0 - arrival.front() * arrival.front());
    for (int i = 0; i < frequencies->size(); i++) {
        const double f = (*frequencies)(i);
        const double P1 = M_PI * _diameter / (sound_speed / f) * sinA + 1e-17;
        const double P2 = 2.0 * std::cyl_bessel_j(1.0, P1);
        (*level)(i) = pow(P2 / P1, 2);
    }
}

/**
 * Computes the directivity gain for piston array.
 */
void bp_piston::directivity(const seq_vector::csptr& frequencies,
                            vector<double>* level, const bvector& /*steering*/,
                            double sound_speed) const {
    *level = frequencies->data();
    *level /= (sound_speed * M_PI * _diameter);
    if (_back_baffle) {
        *level *= 0.5;
    }
}
