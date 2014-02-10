/**
 * @file reflect_loss_rayleigh_grid.h
 * Builds rayleigh models for an imported data_grid bottom types.
 * Includes LaTEX documentation of formula for processing by Doxygen (not yet).
 */

#ifndef USML_OCEAN_REFLECT_LOSS_RAYLEIGH_GRID_H
#define USML_OCEAN_REFLECT_LOSS_RAYLEIGH_GRID_H

#include <usml/ocean/reflect_loss_model.h>
#include <usml/ocean/reflect_loss_rayleigh.h>
#include <vector>

namespace usml {
namespace ocean {

using boost::numeric::ublas::vector;

/// @ingroup boundaries
/// @{

/**
 * The reflect_loss_rayleigh_grid object ingests a data_grid of
 * rayleigh bottom type data and creates a reflect_loss_rayleigh object to
 * create a rayleigh reflection loss value for the bottom type
 * number at a specific location and returns broadband reflection loss and phase change.
 */
class USML_DECLSPEC reflect_loss_rayleigh_grid : public reflect_loss_model {

public:

    /**
	  * Constructor - Loads bottom type data via a data_grid
	  *
	  * @param bottom_type_grid data_grid of bottom_types for locations
	  *
	  * The information stored in "data" member of the data_grid is set
	  * to a double with the value from 0 to 8 representing different bottom types.
	  *
	  */

    reflect_loss_rayleigh_grid(data_grid<double, 2>* bottom_type_grid) ;

	/**
	 * Gets a rayleigh bottom type value at a specific location then
	 * computes the broadband reflection loss and phase change.
	 *
	 * @param location      Location at which to compute attenuation.
	 * @param frequencies   Frequencies over which to compute loss. (Hz)
	 * @param angle         Reflection angle relative to the normal (radians).
	 * @param amplitude     Change in ray strength in dB (output).
	 * @param phase         Change in ray phase in radians (output).
	 *                      Phase change not computed if this is NULL.
	 */
	virtual void reflect_loss(
	        const wposition1& location,
	        const seq_vector& frequencies, double angle,
	        vector<double>* amplitude, vector<double>* phase=NULL ) ;

    // Destructor
	virtual ~reflect_loss_rayleigh_grid();

private:

    std::vector<reflect_loss_rayleigh*> _rayleigh;  ///< _rayleigh : vector of reflect_loss_rayleigh objects

    data_grid<double, 2>* _bottom_grid;             ///< _bottom_grid : data_grid2 object

} ;

/// @}
}  // end of namespace ocean
}  // end of namespace usml

#endif
