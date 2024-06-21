/**
 * @file bp_gaussian.cc
 * Solid angle (aka cookie-cutter) beam pattern with a Gaussian taper.
 */

#include <usml/beampatterns/bp_gaussian.h>
#include <usml/types/bvector.h>
#include <usml/types/seq_vector.h>
#include <usml/ublas/math_traits.h>

#include <algorithm>
#include <boost/numeric/ublas/detail/definitions.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <cmath>

using namespace usml::beampatterns;

/**
 * Computes the beam level gain for an arrival vector in the body coordinates
 * of the array,
 */
void bp_gaussian::beam_level(const bvector& arrival,
                             const seq_vector::csptr& frequencies,
                             vector<double>* level, const bvector& steering,
                             double /*sound_speed*/) const {
    double de = 0.0;
    if (_vert_half < 90.0) {
        de = asin(steering.up());
        de = abs(to_degrees(asin(arrival.up()) - de));
        de /= (_vert_half * 2.0);
    }

    double az = 0.0;
    if (_horz_half < 180.0) {
        az = atan2(steering.front(), steering.right());
        az = to_degrees(atan2(arrival.front(), arrival.right()) - az);
        az = abs(fmod(az + 180.0, 360.0) - 180);
        az /= (_horz_half * 2.0);
    }

    const double value = exp(-0.5 * (de * de + az * az));
    noalias(*level) = scalar_vector<double>(frequencies->size(), value);
}
