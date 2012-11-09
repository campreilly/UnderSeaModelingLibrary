/**
 * @file reflect_loss_netcdf.h
 * Builds rayleigh models for an imported netcdf bottom province file.
 * Includes LaTEX documentation of formula for processing by Doxygen (not yet).
 */

#ifndef USML_OCEAN_REFLECTION_LOSS_NETCDF_H
#define USML_OCEAN_REFLECTION_LOSS_NETCDF_H

#include <usml/ocean/reflect_loss_model.h>
#include <usml/ocean/reflect_loss_rayleigh.h>
#include <vector>
#include <netcdfcpp.h>

namespace usml {
namespace ocean {

class USML_DECLSPEC reflect_loss_netcdf : public reflect_loss_model {

    std::vector<reflect_loss_rayleigh*> rayleigh;
    data_grid<double, 2>* province;

	public:

	 /**
	  * Load bottom province data from disk. Format of the bottom
	  * province file is currently restricted to the following:
	  *
	  *		Dimensions
	  *	@dim lat 	: latitude in degrees
	  *	@dim lon 	: longitude in degrees
	  *	@dim num_types  : number of different bottom provinces
	  *
	  *		Variables
	  *	@var speed_ratio	: speed ratio of the province		    @dim num_types
	  *	@var density_ratio	: density ratio of the province		    @dim num_types
	  *	@var atten		    : attenuation value for the province	@dim num_types
	  *	@var shear_speed	: shear speed of the province		    @dim num_types
	  *	@var shear_atten	: shear attenuation of the province 	@dim num_types
	  *	@var lat		    : latitude in degrees		            @dim lat
	  *	@var long		    : longitude in degress		    	    @dim lon
	  *	@var type		    : bottom province map		    	    @dim (lat,lon)
	  *
	  * The information stored in type is set to a double with the value from 1 to
	  * the number of different bottom provinces for the profile.
	  *
	  */
		reflect_loss_netcdf(const char *filename);

	/**
	 * Computes the broadband reflection loss and phase change.
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

} ;

/// @}
}  // end of namespace ocean
}  // end of namespace usml

#endif
