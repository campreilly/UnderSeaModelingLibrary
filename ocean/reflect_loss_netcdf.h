/**
 * @file reflect_loss_netcdf.h
 * Builds rayleigh models for an imported netcdf bottom province file.
 * Includes LaTEX documentation of formula for processing by Doxygen (not yet).
 */

#ifndef USML_OCEAN_REFLECT_LOSS_NETCDF_H
#define USML_OCEAN_REFLECT_LOSS_NETCDF_H

#include <usml/ocean/reflect_loss_model.h>
#include <usml/ocean/reflect_loss_rayleigh.h>
#include <vector>
#include <netcdfcpp.h>

namespace usml {
namespace ocean {

/// @ingroup boundaries
/// @{

/**
 * The reflect_loss_netcdf object ingests a netCDF formatted file of
 * bottom province data and creates a reflect_loss_rayleigh object to
 * create a rayleigh reflection loss value for the bottom province
 * number at a specific location and returns broadband reflection loss and phase change.
 */
class USML_DECLSPEC reflect_loss_netcdf : public reflect_loss_model {

    std::vector<reflect_loss_rayleigh*> rayleigh;   ///< rayleigh : reflect_loss_rayleigh object
    data_grid<double, 2>* province;                ///< province : data_grid2 object

	public:

	 /**
	  * Loads bottom province data from a netCDF formatted file.
	  *
	  * @param filename     Filename of the NetCDF file to ingest
	  *
	  * The information stored in "type" is set to a double with the value from 1 to
	  * the number of different bottom provinces for the profile.
	  *
	  */
		reflect_loss_netcdf(const char *filename);

	/**
	 * Creates a rayleigh reflection loss value for the bottom province number
	 * at a specific location then computes the broadband reflection loss and phase change.
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
			boost::numeric::ublas::vector<double>* amplitude, boost::numeric::ublas::vector<double>* phase=NULL ) ;


    ///Destuctor
        virtual ~reflect_loss_netcdf();
} ;

/// @}
}  // end of namespace ocean
}  // end of namespace usml

#endif
