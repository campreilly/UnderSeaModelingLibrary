/**
 * @file data_grid_fast_3d.h
 * Wrapper for a data_grid in 3d that uses the fast non-recursive
 * interpolation algorithm.
 */
#ifndef USML_TYPES_DATA_GRID_FAST_3D_H
#define USML_TYPES_DATA_GRID_FAST_3D_H

#include <usml/types/data_grid.h>

namespace usml {
namespace types {
/// @ingroup data_grid
/// @{

/**
* Implements fast calculations for data_grids using a non-recursive
* engine on interpolation. Takes an existing data_grid and wraps it
* into a new data_grid_fast and overrides the interpolate function
* to implement the non-recursive algorithm.
*
* Assumes that both axes of the passed data_grid both have the
* same interp_type.
*
* WARNING: This wrapper is specific to 3-dimensional grids
*          only. For 2-dimensional grids, the user should
*          implement the data_grid_fast_2d wrapper instead.
*
* Since the data is passed in and referenced by this wrapper,
* the data is taken control of and destroyed at the end of its
* use cycle.
*/


class USML_DECLSPEC data_grid_fast_3d : public data_grid<double,3>
{
    private:

        /**
        * A data_grid to do interpolation on using the non-recursive
        * algorithm
        */

        data_grid<double,3>* _grid ;

    public:

        /**
        * Creates a fast interpolation grid from an existing
        * data_grid.
        *
        * @param _grid      The data_grid that is to be wrapped.
        */

        data_grid_fast_3d( data_grid<double,3>* grid ) :
            _grid(grid) {}

        /**
        * Overrides the interpolate function within data_grid using the
        * non-recursive formula. Determines which interpolate function to
        * based on the interp_type enumeral stored within the 0th dimensional
        * axis.
        *
        * Interpolate at a series of locations.
        *
        * @param location   Locations to do the interpolation at
        * @param rho        Surface height in spherical earth coords (output)
        * @param normal     Unit normal relative to location (output)
        */

        double interpolate(const wposition& location, matrix<double>* rho,
            wvector* normal = NULL) {
            switch(_grid->interp_type(0)) {

                ///****nearest****
            case -1:

                break;

                ///****linear****
            case 0:

                break;

                ///****pchip****
            case 1:

                break;
            }
            return 0;
        }

        /**
        * Overrides the interpolate function within data_grid using the
        * non-recursive formula. Determines which interpolate function to
        * based on the interp_type enumeral stored within the 0th dimensional
        * axis.
        *
        * Interpolate at a single location.
        *
        * @param location   Location to do the interpolation at
        * @param rho        Surface height in spherical earth coords (output)
        * @param normal     Unit normal relative to location (output)
        */

        double interpolate(double* location, double* rho,
            wvector1* normal = NULL) {
            switch(_grid->interp_type(0)) {

                ///****nearest****
            case -1:

                break;

                ///****linear****
            case 0:

                break;

                ///****pchip****
            case 1:

                break;
            }
            return 0;
        }

        /**
        * Destructor
        */
       ~data_grid_fast_3d() {
            delete _grid;
        }

};

} // end of namespace types
} // end of namespace usml

#endif
