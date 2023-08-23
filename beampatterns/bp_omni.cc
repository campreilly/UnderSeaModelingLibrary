/**
 * @file bp_omni.cc
 * Omni-directional beam pattern.
 */
#include <usml/beampatterns/bp_omni.h>

using namespace usml::beampatterns;

/**
 * Computes the beam level gain for an arrival vector in the body coordinates
 * of the array,
 */
void bp_omni::beam_level(const bvector& /*arrival*/,
                         const seq_vector::csptr& frequencies, vector<double>* level,
                         const bvector& /*steering*/,
                         double /*sound_speed*/) const {
    noalias(*level) = scalar_vector<double>(frequencies->size(), 1.0);
}

/**
 * Computes the directivity gain for uniform beam pattern.
 */
void bp_omni::directivity(const seq_vector::csptr& frequencies, vector<double>* level,
                          const bvector& /*steering*/,
                          double /*sound_speed*/) const {
    noalias(*level) = scalar_vector<double>(frequencies->size(), 1.0);
}
