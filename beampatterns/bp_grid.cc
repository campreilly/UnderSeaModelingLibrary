/**
 * @file bp_grid.cc
 * Constructs a beam pattern from a data grid.
 */

#include <usml/beampatterns/bp_grid.h>
#include <usml/ublas/math_traits.h>

#include <cmath>
#include <memory>

using namespace usml::beampatterns;

/**
 * Computes the beam level gain for an arrival vector in the body coordinates
 * of the array.
 */
void bp_grid::beam_level(const bvector& arrival, const seq_vector::csptr& frequencies,
                         vector<double>* level, const bvector& /* steering */,
                         double /* sound_speed */) const {
    double location[3];
    location[1] = to_degrees(asin(arrival.up()));
    location[2] = to_degrees(atan2(arrival.right(), arrival.front()));
    for (int f = 0; f < frequencies->size(); ++f) {
        location[0] = (*frequencies)[f];
        (*level)[f] = _data->interpolate(location);
    }
}
