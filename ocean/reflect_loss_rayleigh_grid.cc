/**
 * @file reflect_loss_rayleigh_grid.cc
 * Models plane wave reflection from a bottom types profile .
 */
#include <usml/ocean/reflect_loss_rayleigh_grid.h>
#include <exception>

using namespace usml::ocean ;

/** Constructor - Creates a reflection_loss_rayleigh object for each rayleigh
 * bottom type value (nine total). Also loads bottom type data based on location
 * via a data_grid for later computation on calls to reflect_loss().
 *
 * @param bottom_type_grid data_grid of bottom_type for a location
 *
 * The information stored in "data" is set to a double with the value from 1 to
 * the number of different rayleigh bottom types for the profile.
 */
reflect_loss_rayleigh_grid::reflect_loss_rayleigh_grid(data_grid<double, 2>* bottom_type_grid)
    :   _bottom_grid(bottom_type_grid)
{
    size_t n_types;

    n_types = reflect_loss_rayleigh::BASALT+1; // number of elements of reflect_loss_rayleigh.bottom_type_enum

    /** Set the interpolation type to the nearest neighbor and restrict extrapolation */
    for(int i=0; i<2; i++){
        _bottom_grid->interp_type(i, GRID_INTERP_NEAREST);
        _bottom_grid->edge_limit(i, true);
    }

    /** Builds a vector of reflect_loss_rayleigh values for all bottom types */
    for( size_t i=0; i < n_types; i++ ) {
        _rayleigh.push_back( new reflect_loss_rayleigh( i ) );
    }

}

/** Gets a rayleigh bottom type value at a specific location then
 *  computes the broadband reflection loss and phase change.
 *
 * @param location      Location at which to compute attenuation.
 * @param frequencies   Frequencies over which to compute loss. (Hz)
 * @param angle         Reflection angle relative to the normal (radians).
 * @param amplitude     Change in ray strength in dB (output).
 * @param phase         Change in ray phase in radians (output).
 *                      Phase change not computed if this is NULL.
 */
void reflect_loss_rayleigh_grid::reflect_loss(
    const wposition1& location,
    const seq_vector& frequencies, double angle,
    vector<double>* amplitude, vector<double>* phase) {

    double loc[2];
    loc[0] = location.latitude();
    loc[1] = location.longitude();

    size_t type = _bottom_grid->interpolate(loc);
    _rayleigh[type]->reflect_loss(location, frequencies, angle, amplitude, phase );
}

/** Destructor - Iterates over the rayleigh reflection loss values
 * and deletes them.
 */
reflect_loss_rayleigh_grid::~reflect_loss_rayleigh_grid() {
	for(std::vector<reflect_loss_rayleigh*>::iterator iter=_rayleigh.begin(); iter != _rayleigh.end(); iter++) {
        delete *iter;
	}
}
