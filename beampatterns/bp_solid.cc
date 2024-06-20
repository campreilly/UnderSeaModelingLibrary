/**
 * @file bp_solid.cc
 * Solid angle (aka cookie-cutter) beam pattern.
 */

#include <usml/beampatterns/bp_solid.h>
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
void bp_solid::beam_level(const bvector& arrival,
                          const seq_vector::csptr& frequencies,
                          vector<double>* level, const bvector& steering,
                          double /*sound_speed*/) const {
    double de = asin(steering.up());
    double az = atan2(steering.front(), steering.right());
    de = abs(to_degrees(asin(arrival.up()) - de));
    az = to_degrees(atan2(arrival.front(), arrival.right()) - az);
    az = abs(fmod(az + 180.0, 360.0) - 180);
    const double value = (de <= _vert_half && az <= _horz_half) ? 1.0 : 0.0;
    noalias(*level) = scalar_vector<double>(frequencies->size(), value);
}

/**
 * Computes the directivity gain for uniform a solid angle beam pattern.
 */
void bp_solid::directivity(const seq_vector::csptr& frequencies,
                           vector<double>* level, const bvector& steering,
                           double /*sound_speed*/) const {
    const double de = to_degrees(asin(steering.up()));
    const double diff =
        min(90.0, de + _vert_half) - max(-90.0, de - _vert_half);
    const double value = diff * _horz_half / (180.0 * 360.0 / 2.0);
    noalias(*level) = scalar_vector<double>(frequencies->size(), value);
}
