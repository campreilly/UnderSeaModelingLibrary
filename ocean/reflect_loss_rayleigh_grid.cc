/**
 * @file reflect_loss_rayleigh_grid.cc
 * Models plane wave reflection from a bottom types profile .
 */

#include <usml/ocean/reflect_loss_rayleigh_grid.h>

#include <memory>
#include <utility>

using namespace usml::ocean;

/**
 * Creates a reflection_loss_rayleigh object for each rayleigh
 * bottom type value (nine total). Also loads bottom type data based on location
 * via a data_grid for later computation on calls to reflect_loss().
 *
 * The information stored in "data" is set to a double with the value from 1 to
 * the number of different rayleigh bottom types for the profile.
 */
reflect_loss_rayleigh_grid::reflect_loss_rayleigh_grid(
    const data_grid<2>::sptr& type_grid)
    : _bottom_grid(type_grid) {
    // set the interpolation type to the nearest neighbor
    // and restrict extrapolation

    for (size_t i = 0; i < 2; i++) {
        type_grid->interp_type(i, interp_enum::nearest);
        type_grid->edge_limit(i, true);
    }

    // builds a vector of reflect_loss_rayleigh values for all bottom types

    auto num_types = size_t(bottom_type_enum::basalt);
    for (size_t i = 0; i <= num_types; i++) {
        auto model = reflect_loss_rayleigh::csptr(new reflect_loss_rayleigh(i));
        _rayleigh.push_back(model);
    }
}

/**
 * Gets a Rayleigh bottom type value at a specific location then
 * computes the broadband reflection loss and phase change.
 *
 * @param location      Location at which to compute attenuation.
 * @param frequencies   Frequencies over which to compute loss. (Hz)
 * @param angle         Reflection angle relative to the normal (radians).
 * @param amplitude     Change in ray strength in dB (output).
 * @param phase         Change in ray phase in radians (output).
 *                      Phase change not computed if this is nullptr.
 */
void reflect_loss_rayleigh_grid::reflect_loss(
    const wposition1& location, const seq_vector::csptr& frequencies,
    double angle, vector<double>* amplitude, vector<double>* phase) const {
    double loc[2];
    loc[0] = location.latitude();
    loc[1] = location.longitude();
    auto type = (size_t)_bottom_grid->interpolate(loc);
    _rayleigh[type]->reflect_loss(location, frequencies, angle, amplitude,
                                  phase);
}
