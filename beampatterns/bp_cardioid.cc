/**
 * @file bp_cardioid.cc
 * A frequency independent cardioid beam pattern, .
 */

#include <usml/beampatterns/bp_cardioid.h>

#include <boost/numeric/ublas/detail/definitions.hpp>

using namespace usml::beampatterns;

/**
 * Computes the beam level gain for an arrival vector in the body coordinates
 * of the array,
 */
void bp_cardioid::beam_level(const bvector& arrival,
                             const seq_vector::csptr& frequencies,
                             vector<double>* level, const bvector& /*steering*/,
                             double /*sound_speed*/) const {
    const double P = (1.0 + _factor * arrival.front()) / (1.0 + _factor);
    noalias(*level) = scalar_vector<double>(frequencies->size(), P * P);
}

/**
 * Computes the directivity gain for this beam pattern.
 */
void bp_cardioid::directivity(const seq_vector::csptr& frequencies,
                              vector<double>* level,
                              const bvector& /*steering*/,
                              double /*sound_speed*/) const {
    noalias(*level) = scalar_vector<double>(frequencies->size(), _directivity);
}
