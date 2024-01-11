/**
 * @file data_grid_mackenzie.h
 * Mackenzie model for the speed of sound vs. temperature and salinity.
 */
#pragma once

#include <stddef.h>
#include <usml/types/data_grid.h>
#include <usml/types/gen_grid.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition.h>

namespace usml {
namespace ocean {

using namespace usml::types;

/// @ingroup profiles
/// @{

/**
 * Implements the Mackenzie model for estimating the speed of sound
 * from the ocean's temperature and salinity profile. This type of
 * profile is commonly used to estimate the sound speed profile
 * associated with a ocean physical properties database such as
 * World Ocean Atlas.
 *
 * Mackenzie model is defined by the equation:
 * <pre>
 *      c(D,S,T) =
 *          1448.96 + 4.591 T - 5.304e-2 T^2 + 2.374e-4 T^3
 *          + 1.340 (S-35) + 1.630e-2 D
 *          + 1.675e-7 D^2 - 1.025e-2 T (S-35) - 7.139e-13 T D^3
 * </pre>
 * where:
 *      - D = depth (meters)
 *      - S = salinity (ppt)
 *      - T = temperature (degrees C)
 *
 * Uses the interp_enum::pchip interpolation in the depth direction
 * to reduce sudden changes in profile slope.  The latitude and longitude
 * directions use interp_enum::linear (the default).  Values outside of the
 * latitude/longitude axes defined by the data grid at limited to the values
 * at the grid edge.
 *
 * When using a gridded data set, it is recommended that edge_limit be set to
 * TRUE for any dimensional axis that uses the PCHIP interpolation. This is
 * because of PCHIP allowing for extreme values when extrapolating data.
 *
 * NOTE: data_grid_mackenzie takes control of the two data_grids that are
 * passed in and then deletes them before the sound speed data_grid is returned.
 *
 * @xref R.J. Urick, Principles of Underwater Sound, 3rd Edition,
 *       (1983), p. 113.
 *
 * @xref K.V. Mackenzie, "Nine-term Equation for Sound Speed
 *       in the Oceans," J. Acoust. Soc. Am. 70:807 (1981).
 *
 * @xref UK National Physical Laboratory, "Technical Guides -
 *       Speed of Sound in Sea-Water," interactive website at
 *       http://resource.npl.co.uk/acoustics/techguides/soundseawater/
 */
class data_grid_mackenzie : public gen_grid<3> {
   public:
    //**************************************************
    // initialization

    /**
     * Define sound speed profile using temperature and salinity.
     *
     * @param temperature   Ocean temperature profile (degrees C).
     * @param salinity      Ocean salinity profile (ppt).
     */
    data_grid_mackenzie(data_grid<3>::csptr temperature,
                        data_grid<3>::csptr salinity)
        : gen_grid<3>(temperature->axis_list()) {
        // build grid the same size as the temperature grid

        for (int dim = 0; dim < 3; ++dim) {
            this->interp_type(dim, temperature->interp_type(dim));
            this->edge_limit(dim, temperature->edge_limit(dim));
        }

        // compute sound speed for each temperature, salinity, and depth

        size_t index[3];
        for (index[0] = 0; index[0] < temperature->axis(0).size(); ++index[0]) {
            for (index[1] = 0; index[1] < temperature->axis(1).size();
                 ++index[1]) {
                for (index[2] = 0; index[2] < temperature->axis(2).size();
                     ++index[2]) {
                    // extract depth, temperature, and salinity at this point

                    double D = wposition::earth_radius -
                               temperature->axis(0)[index[0]];
                    double T = temperature->data(index);
                    double S = salinity->data(index);

                    // compute sound speed

                    double c = 1448.96 + 4.591 * T - 5.304e-2 * T * T +
                               2.374e-4 * T * T * T +
                               (1.340 - 1.025e-2 * T) * (S - 35.0) +
                               1.630e-2 * D + 1.675e-7 * D * D -
                               7.139e-13 * T * D * D * D;
                    setdata(index, c);
                }
            }
        }
    }
};

/// @}
}  // end of namespace ocean
}  // end of namespace usml
