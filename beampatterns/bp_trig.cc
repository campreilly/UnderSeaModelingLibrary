/**
 * @file bp_trig.cc
 * Models frequency independent sonobuoy sine and cosine beampatterns.
 */

#include <usml/beampatterns/bp_trig.h>

#include <boost/numeric/ublas/detail/definitions.hpp>

using namespace usml::beampatterns;

/**
 * Computes the beam level gain for an arrival vector in the body coordinates
 * of the array,
 */
void bp_trig::beam_level(const bvector& arrival, const seq_vector::csptr& frequencies,
                         vector<double>* level, const bvector& /*steering*/,
                         double /*sound_speed*/) const {
    const double dot =
        (_type == bp_trig_type::sine) ? arrival.right() : arrival.front();
    const double L = _null + _gain * dot * dot;
    noalias(*level) = scalar_vector<double>(frequencies->size(), L);
}

/**
 * Computes the directivity gain for this beam pattern.
 */
void bp_trig::directivity(const seq_vector::csptr& frequencies, vector<double>* level,
                          const bvector& /*steering*/,
                          double /*sound_speed*/) const {
    noalias(*level) = scalar_vector<double>(frequencies->size(), _directivity);
}
