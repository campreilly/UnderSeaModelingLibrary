/**
 * @file reflect_loss_netcdf.h
 * Builds rayleigh models for an imported netcdf bottom province file.
 */
#pragma once

#include <netcdfcpp.h>
#include <usml/ocean/reflect_loss_model.h>
#include <usml/ocean/reflect_loss_rayleigh.h>
#include <usml/types/gen_grid.h>

#include <vector>

namespace usml {
namespace ocean {

using boost::numeric::ublas::vector;

/// @ingroup boundaries
/// @{

/**
 * The reflect_loss_netcdf object ingests a netCDF formatted file of
 * bottom type data and creates a reflect_loss_rayleigh object to
 * create a rayleigh reflection loss value for the bottom type
 * number at a specific location and returns broadband reflection
 * loss and phase change. An example of this format is provided below.
 * <pre>
 *  netcdf sediment_test {
 *  dimensions:
 *      speed_ratio = 2 ;
 *      density_ratio = 2 ;
 *      atten = 2 ;
 *      shear_speed = 2 ;
 *      shear_atten = 2 ;
 *      longitude = 10 ;
 *      latitude = 10 ;
 *  variables:
 *      double speed_ratio(speed_ratio) ;
 *      double density_ratio(density_ratio) ;
 *      double atten(atten) ;
 *      double shear_speed(shear_speed) ;
 *      double shear_atten(shear_atten) ;
 *      double longitude(longitude) ;
 *          longitude:units = "degrees" ;
 *      double latitude(latitude) ;
 *          latitude:units = "degrees" ;
 *      double type(longitude, latitude) ;
 *          type:units = "basic" ;
 *  data:
 *   speed_ratio = 1.11733, 2 ;
 *   density_ratio = 1.7, 2.4 ;
 *   atten = 0.01, 0.1 ;
 *   shear_speed = 0, 0.953333 ;
 *   shear_atten = 0, 0.2 ;
 *   longitude = -80.4, -81.029, -82.192, -83.738, -84.1, -85, 2039, -86.029,
 *      -87.0029, -88.2938 ;
 *   latitude = 26, 27.03, 28.5, 29.2, 30.019, 31.741, 32.402, 33.9, 34.593,
 *      35.0001 ;
 *   type =
 *    0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
 *    0, 0, 0, 0, 0, 0, 0, 1, 0, 1,
 *    0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
 *    0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
 *    0, 0, 0, 0, 1, 0, 0, 1, 1, 1,
 *    0, 0, 0, 1, 0, 1, 1, 1, 1, 1,
 *    0, 0, 1, 0, 0, 1, 1, 1, 1, 1,
 *    0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
 *    0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 *    0, 1, 1, 1, 1, 1, 1, 1, 1, 1 ;
 *   }
 * </pre>
 */
class USML_DECLSPEC reflect_loss_netcdf : public reflect_loss_model {
   public:
    /**
     * Loads bottom province data from a netCDF formatted file.
     *
     * @param filename     Filename of the NetCDF file to ingest
     *
     * The information stored in "type" is set to a double with the value from 1
     * to the number of different bottom provinces for the profile.
     *
     */
    reflect_loss_netcdf(const char* filename);

    /**
     * Gets a reflection loss value for the bottom type number at a specific
     * location then computes the broadband reflection loss and phase change.
     *
     * @param location      Location at which to compute attenuation.
     * @param frequencies   Frequencies over which to compute loss. (Hz)
     * @param angle         Reflection angle relative to the normal (radians).
     * @param amplitude     Change in ray strength in dB (output).
     * @param phase         Change in ray phase in radians (output).
     *                      Phase change not computed if this is nullptr.
     */
    void reflect_loss(const wposition1& location,
                      const seq_vector::csptr& frequencies, double angle,
                      vector<double>* amplitude,
                      vector<double>* phase = nullptr) const override;

   private:
    /**
     * Stored loss models for bottom reflections
     */
    std::vector<reflect_loss_model::csptr> _loss_model;

    /**
     * Data grid that stores all of the bottom province information.
     */
    data_grid<2>::csptr _bottom_grid;
};

/// @}
}  // end of namespace ocean
}  // end of namespace usml
