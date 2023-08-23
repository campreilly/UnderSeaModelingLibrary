/**
 * @file bp_multi.cc
 * Combines the responses of multiple beampattern models into a single model.
 */

#include <usml/beampatterns/bp_multi.h>
#include <usml/types/orientation.h>

using namespace usml::beampatterns;

/**
 * Computes the beam level gain for an arrival vector in the body coordinates
 * of the array,
 */
void bp_multi::beam_level(const bvector& arrival,
                          const seq_vector::csptr& frequencies,
                          vector<double>* level, const bvector& steering,
                          double sound_speed) const {
    vector<double> tmp(frequencies->size(), 1.0);
    *level = vector<double>(frequencies->size(), 1.0);

    for (const auto& pattern : _bp_list) {
        pattern->beam_level(arrival, frequencies, &tmp, steering, sound_speed);
        if (_type == bp_multi_type::sum) {
            *level += tmp;
        } else {
            *level = element_prod(*level, tmp);
        }
    }
}
