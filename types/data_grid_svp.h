/**
 * @file data_grid_svp.h
 * Wrapper for a data_grid in 3d that uses the fast non-recursive
 * interpolation algorithm.
 */
#pragma once

#include <usml/types/data_grid.h>

namespace usml {
namespace types {
/// @ingroup data_grid
/// @{

/**
 * Implements fast calculations for data_grids using a non-recursive
 * engine on interpolation. Takes an existing data_grid and wraps it
 * into a new data_grid_svp and overrides the interpolate function
 * to implement the non-recursive algorithm.
 *
 * Assumes that both axes of the passed data_grid both have the
 * same interp_type.
 *
 * WARNING: This wrapper is specific to 3-dimensional grids
 *          only. For 2-dimensional grids, the user should
 *          implement the data_grid_bathy wrapper instead.
 *
 * Since the data is passed in and referenced by this wrapper,
 * the data is taken control of and destroyed at the end of its
 * use cycle.
 */

class USML_DECLSPEC data_grid_svp: public data_grid<double, 3> {

    public:

        /**
         * Constructor - Creates a fast interpolation grid from an existing
         * data_grid.
         *
         * @param grid      The data_grid that is to be wrapped.
         * @param copy_data If true, copies the data grids data
         *                  fields as well as the axises.
         */

        data_grid_svp( const data_grid<double, 3>* grid )
            :   data_grid<double, 3>(*grid, true),
                _kzmax(_axis[0]->size() - 1u),
                _kxmax(_axis[1]->size() - 1u),
                _kymax(_axis[2]->size() - 1u)
        {
            double result = 0.0;

            //pchip variables
            double inc1, inc2;
            double w1, w2;
            double slope_1, slope_2;


            if (interp_type(0) != GRID_INTERP_PCHIP) {
                interp_type(0, GRID_INTERP_PCHIP);
            }
            if ((interp_type(1) != GRID_INTERP_LINEAR)
                    || (interp_type(1) != GRID_INTERP_LINEAR)) {
                interp_type(1, GRID_INTERP_LINEAR);
                interp_type(2, GRID_INTERP_LINEAR);
            }
            derv_z = new double**[_kzmax + 1u];
            for (int i = 0; i < _kzmax + 1u; ++i) {
                derv_z[i] = new double*[_kxmax + 1u];
                for (int j = 0; j < _kxmax + 1u; ++j) {
                    derv_z[i][j] = new double[_kymax + 1u];
                }
            }
            for (int i = 0; i < _kzmax + 1u; ++i) {
                for (int j = 0; j < _kxmax + 1u; ++j) {
                    for (int k = 0; k < _kymax + 1u; ++k) {
                        if (i == 0) {
                            inc1 = _axis[0]->increment(i);
                            inc2 = _axis[0]->increment(i + 1);
                            slope_1 = (data_3d(i + 1, j, k) - data_3d(i, j, k))
                                    / inc1;
                            slope_2 = (data_3d(i + 2, j, k) - data_3d(i + 1, j, k))
                                    / inc2;
                            result = ( (2.0 * inc1 + inc2) * slope_1 - inc1 * slope_2 )
                                        / (inc1 + inc2) ;
                            if ( result * slope_1 <= 0.0 ) {
                                result = 0.0 ;
                            } else if ( (slope_1 * slope_2 <= 0.0)
                                    && (abs(result) > abs(3.0 * slope_1)) ) {
                                result = 3.0 * slope_1 ;
                            }
                        } else if (i == _kzmax) {
                            inc1 = _axis[0]->increment(i - 1);
                            inc2 = _axis[0]->increment(i);
                            slope_1 = (data_3d(i - 1, j, k) - data_3d(i - 2, j, k))
                                    / inc1;
                            slope_2 = (data_3d(i, j, k) - data_3d(i - 1, j, k))
                                    / inc2;
                            result = ( (2.0 * inc1 + inc2) * slope_2 - inc1 * slope_1 )
                                       / (inc1 + inc2) ;
                            if (result * slope_1 <= 0.0) {
                                result = 0.0;
                            } else if ((slope_1 * slope_2 <= 0.0)
                                    && (abs(result) > abs(3.0 * slope_1))) {
                                result = 3.0 * slope_1;
                            }
                        } else {
                            inc1 = _axis[0]->increment(i - 1);
                            inc2 = _axis[0]->increment(i);
                            w1 = 2.0 * inc2 + inc1;
                            w2 = inc2 + 2.0 * inc1;
                            slope_1 = (data_3d(i, j, k) - data_3d(i - 1, j, k))
                                    / inc1;
                            slope_2 = (data_3d(i + 1, j, k) - data_3d(i, j, k))
                                    / inc2;
                            if (slope_1 * slope_2 <= 0.0) {
                                result = 0.0;
                            } else {
                                result = (w1 + w2)
                                        / ((w1 / slope_1) + (w2 / slope_2));
                            }
                        }
                        derv_z[i][j][k] = result ;
                    } //end for-loop in k
                } //end for-loop in j
            } //end for-loop in i
            delete grid ;
        } // end Constructor

        /**
         * Destructor
         */
        virtual ~data_grid_svp() {
            for(size_t i=0; i<=_kzmax; ++i) {
                for(size_t j=0; j<=_kxmax; ++j) {
                    delete[] derv_z[i][j] ;
                }
                delete[] derv_z[i] ;
            }
            delete[] derv_z ;
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
         * @param derivative Calculates first derivative if not NULL
         */

        double interpolate(double* location, double* derivative = NULL)
        {
            double result = 0.0;
            unsigned k0, k1, k2;           //indices of he offset data

            //bi-linear variables
            double f11, f21, f12, f22, x_diff, y_diff;
            double x, x1, x2, y, y1, y2;

            //pchip variables
            double v1, v2;
            double inc1;
            double t, t_2, t_3;
            double h00, h10, h01, h11;

            // find the interval index in each dimension

            for (unsigned dim = 0; dim < 3; ++dim) {

                // limit interpolation to axis domain if _edge_limit turned on

                if (edge_limit(dim)) {
                    double a = *(_axis[dim]->begin());
                    double b = *(_axis[dim]->rbegin());
                    double inc = _axis[dim]->increment(0);
                    if (inc < 0) {                                // a > b
                        if (location[dim] >= a) {                 //left of the axis
                            location[dim] = a;
                            _offset[dim] = 0;
                        } else if (location[dim] <= b) {         //right of the axis
                            location[dim] = b;
                            _offset[dim] = _axis[dim]->size() - 2;
                        } else {
                            _offset[dim] = _axis[dim]->find_index(location[dim]); //somewhere in-between the endpoints of the axis
                        }
                    }
                    if (inc > 0) {                                // a < b
                        if (location[dim] <= a) {                 //left of the axis
                            location[dim] = a;
                            _offset[dim] = 0;
                        } else if (location[dim] >= b) {         //right of the axis
                            location[dim] = b;
                            _offset[dim] = _axis[dim]->size() - 2;
                        } else {
                            _offset[dim] = _axis[dim]->find_index(location[dim]); //somewhere in-between the endpoints of the axis
                        }
                    }

                    // allow extrapolation if _edge_limit turned off

                } else {
                    _offset[dim] = _axis[dim]->find_index(location[dim]);
                }
            }

            //** PCHIP contribution in zeroth dimension */
            if (derivative) {
                derivative[0] = 0;
            }
            k0 = _offset[0];
            k1 = _offset[1];
            k2 = _offset[2];

            // construct the interpolated plane to which the final bi-linear
            // interpolation will happen
            for (int i = 0; i < 2; ++i) {
                for (int j = 0; j < 2; ++j) {
                    //extract data and take precautions when at boundaries of the axis
                    v1 = data_3d(k0, k1 + i, k2 + j);
                    v2 = data_3d(k0 + 1, k1 + i, k2 + j);
                    inc1 = _axis[0]->increment(k0);

                    t = (location[0] - (*_axis[0])(k0)) / inc1;
                    t_2 = t * t;
                    t_3 = t_2 * t;

                    //construct the hermite polynomials
                    h00 = (2 * t_3 - 3 * t_2 + 1);
                    h10 = (t_3 - 2 * t_2 + t);
                    h01 = (3 * t_2 - 2 * t_3);
                    h11 = (t_3 - t_2);

                    _interp_plane(i, j) = h00 * v1 + h10 * derv_z[k0][k1 + i][k2 + j]
                            + h01 * v2 + h11 * derv_z[k0 + 1][k1 + i][k2 + j];

                    if (derivative) {
                        _dz(i, j) = (6 * t_2 - 6 * t) * v1 / inc1
                                + (3 * t_2 - 4 * t + 1) * derv_z[k0][k1 + i][k2 + j] / inc1
								+ (6 * t - 6 * t_2) * v2 / inc1
                                + (3 * t_2 - 2 * t) * derv_z[k0 + 1][k1 + i][k2 + j] / inc1 ;
                    }
                }
            }

            //** Bi-Linear contributions from first/second dimensions */
            //extract data around field point
            x = location[1];
            x1 = (*_axis[1])(k1);
            x2 = (*_axis[1])(k1 + 1);
            y = location[2];
            y1 = (*_axis[2])(k2);
            y2 = (*_axis[2])(k2 + 1);
            f11 = _interp_plane(0, 0);
            f21 = _interp_plane(1, 0);
            f12 = _interp_plane(0, 1);
            f22 = _interp_plane(1, 1);
            x_diff = x2 - x1;
            y_diff = y2 - y1;

            result = (f11 * (x2 - x) * (y2 - y) + f21 * (x - x1) * (y2 - y)
                    + f12 * (x2 - x) * (y - y1) + f22 * (x - x1) * (y - y1))
                    / (x_diff * y_diff);

            if (derivative) {
                derivative[0] = (_dz(0, 0) * (x2 - x) * (y2 - y)
                        + _dz(1, 0) * (x - x1) * (y2 - y)
                        + _dz(0, 1) * (x2 - x) * (y - y1)
                        + _dz(1, 1) * (x - x1) * (y - y1)) / (x_diff * y_diff);
                derivative[1] = (-f11 * (y2 - y) + f21 * (y2 - y) - f12 * (y - y1)
                        + f22 * (y - y1)) / (x_diff * y_diff);
                derivative[2] = (-f11 * (x2 - x) - f21 * (x - x1) + f12 * (x2 - x)
                        + f22 * (x - x1)) / (x_diff * y_diff);
            }

            return result;

        } // end interpolate at a single location.

        /**
         * Interpolation 3-D specialization where the arguments, and results,
         * are matrix<double>.  This is used frequently in the WaveQ3D model
         * to interpolate environmental parameters.
         *
         * @param   x           First dimension of location.
         * @param   y           Second dimension of location.
         * @param   z           Third dimension of location.
         * @param   result      Interpolated values at each location (output).
         * @param   dx          First dimension of derivative (output).
         * @param   dy          Second dimension of derivative (output).
         * @param   dz          Third dimension of derivative (output).
         */
        void interpolate(const matrix<double>& x, const matrix<double>& y,
                const matrix<double>& z, matrix<double>* result,
                matrix<double>* dx = NULL, matrix<double>* dy = NULL,
                matrix<double>* dz = NULL)
        {
            double location[3];
            double derivative[3];
            for (unsigned n = 0; n < x.size1(); ++n) {
                for (unsigned m = 0; m < x.size2(); ++m) {
                    location[0] = x(n, m);
                    location[1] = y(n, m);
                    location[2] = z(n, m);
                    if (dx == NULL || dy == NULL || dz == NULL) {
                        (*result)(n, m) = (double) interpolate(location);
                    } else {
                        (*result)(n, m) = (double) interpolate(location,
                                derivative);
                        (*dx)(n, m) = (double) derivative[0];
                        (*dy)(n, m) = (double) derivative[1];
                        (*dz)(n, m) = (double) derivative[2];
                    }
                }
            }

        } // end interpolate

    private:

        /** Utility accessor function for data grid values */
        inline double data_3d(unsigned dim0, unsigned dim1, unsigned dim2)
        {
            unsigned grid_index[3];
            grid_index[0] = dim0;
            grid_index[1] = dim1;
            grid_index[2] = dim2;
            return data(grid_index);

        } // end data_3d

        /**
         * Create all variables needed for each calculation once
         * to same time and memory.
         */
        unsigned _kzmax, _kxmax, _kymax;  //max index on z-axis (depth)

        //bi-linear variable
        c_matrix<double, 2, 2> _interp_plane;

        //pchip variables
        c_matrix<double, 2, 2> _dz;
        double*** derv_z;

}; // end data_grid_svp class

} // end of namespace types
} // end of namespace usml
