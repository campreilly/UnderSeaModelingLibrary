/**
 * @file data_grid_svp.h
 * Fast non-recursive 3D interpolation algorithm for sound profiles.
 */
#pragma once

#include <usml/types/gen_grid.h>
#include <usml/types/seq_vector.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/detail/iterator.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <memory>

namespace usml {
namespace types {

/// @ingroup data_grid
/// @{

/**
 * Implements fast calculations for 3D data_grids using a non-recursive
 * engine on interpolation. Ignores the interp_type of the underlying grid
 * and replaces this with PCHIP in depth and LINEAR in latitude and longitude.
 *
 * Unlike the gen_grid class, this wrapper does not support modification of
 * the underlying data set. It uses const shared pointers to reference the
 * data in the underlying data_grid.
 */
class USML_DECLSPEC data_grid_svp : public gen_grid<3> {
   public:
    /**
     * Creates a fast interpolation grid from an existing profile.
     *
     * @param grid      The data_grid that is to be wrapped.
     */
    data_grid_svp(data_grid<3>::csptr grid)
        : _kzmax(grid->axis(0).size() - 1u),
          _kxmax(grid->axis(1).size() - 1u),
          _kymax(grid->axis(2).size() - 1u) {
        double result = 0.0;

        // copy data from original grid

        cout << "copy data from original grid" << endl;
        for (size_t n = 0; n < 3; ++n) {
            this->_axis[n] = grid->axis_csptr(n);
            this->_edge_limit[n] = grid->edge_limit(n);
        }
        this->_data = grid->data_csptr();

        this->_interp_type[0] = interp_enum::pchip;
        this->_interp_type[1] = interp_enum::linear;
        this->_interp_type[2] = interp_enum::linear;

        // pchip variables

        double inc1, inc2;
        double w1, w2;
        double slope_1, slope_2;

        _derv_z = new double**[_kzmax + 1u];
        for (size_t i = 0; i < _kzmax + 1u; ++i) {
            _derv_z[i] = new double*[_kxmax + 1u];
            for (size_t j = 0; j < _kxmax + 1u; ++j) {
                _derv_z[i][j] = new double[_kymax + 1u];
            }
        }

        for (size_t i = 0; i < _kzmax + 1u; ++i) {
            for (size_t j = 0; j < _kxmax + 1u; ++j) {
                for (size_t k = 0; k < _kymax + 1u; ++k) {
                    if (i == 0) {
                        inc1 = this->axis(0).increment(i);
                        inc2 = this->axis(0).increment(i + 1);
                        slope_1 =
                            (data_3d(i + 1, j, k) - data_3d(i, j, k)) / inc1;
                        slope_2 =
                            (data_3d(i + 2, j, k) - data_3d(i + 1, j, k)) /
                            inc2;
                        result =
                            ((2.0 * inc1 + inc2) * slope_1 - inc1 * slope_2) /
                            (inc1 + inc2);
                        if (result * slope_1 <= 0.0) {
                            result = 0.0;
                        } else if ((slope_1 * slope_2 <= 0.0) &&
                                   (abs(result) > abs(3.0 * slope_1))) {
                            result = 3.0 * slope_1;
                        }
                    } else if (i == _kzmax) {
                        inc1 = this->axis(0).increment(i - 1);
                        inc2 = this->axis(0).increment(i);
                        slope_1 =
                            (data_3d(i - 1, j, k) - data_3d(i - 2, j, k)) /
                            inc1;
                        slope_2 =
                            (data_3d(i, j, k) - data_3d(i - 1, j, k)) / inc2;
                        result =
                            ((2.0 * inc1 + inc2) * slope_2 - inc1 * slope_1) /
                            (inc1 + inc2);
                        if (result * slope_1 <= 0.0) {
                            result = 0.0;
                        } else if ((slope_1 * slope_2 <= 0.0) &&
                                   (abs(result) > abs(3.0 * slope_1))) {
                            result = 3.0 * slope_1;
                        }
                    } else {
                        inc1 = this->axis(0).increment(i - 1);
                        inc2 = this->axis(0).increment(i);
                        w1 = 2.0 * inc2 + inc1;
                        w2 = inc2 + 2.0 * inc1;
                        slope_1 =
                            (data_3d(i, j, k) - data_3d(i - 1, j, k)) / inc1;
                        slope_2 =
                            (data_3d(i + 1, j, k) - data_3d(i, j, k)) / inc2;
                        if (slope_1 * slope_2 <= 0.0) {
                            result = 0.0;
                        } else {
                            result =
                                (w1 + w2) / ((w1 / slope_1) + (w2 / slope_2));
                        }
                    }
                    _derv_z[i][j][k] = result;
                }  // end for-loop in k
            }      // end for-loop in j
        }          // end for-loop in i
    }              // end Constructor

    /**
     * Destructor
     */
    ~data_grid_svp() {
        for (size_t i = 0; i <= _kzmax; ++i) {
            for (size_t j = 0; j <= _kxmax; ++j) {
                delete[] _derv_z[i][j];
            }
            delete[] _derv_z[i];
        }
    }

    /**
     * Overrides the interpolate function within data_grid using the
     * non-recursive formula. Determines which interpolate function to based
     * on the interp_type enumeration stored within the 0th dimensional axis.
     *
     * Interpolate at a single location.
     *
     * @param location   Location to do the interpolation at
     * @param derivative Calculates first derivative if not nullptr
     */
    double interpolate(double* location, double* derivative = nullptr) const {
        double result = 0.0;
        size_t k0, k1, k2;  // indices of he offset data
        size_t offset[3];
        // bi-linear variables
        double f11, f21, f12, f22, x_diff, y_diff;
        double x, x1, x2, y, y1, y2;

        // pchip variables
        double inc1;
        double t, t_2, t_3;
        double h00, h10, h01, h11;

        // find the interval index in each dimension

        for (size_t dim = 0; dim < 3; ++dim) {
            // limit interpolation to axis domain if _edge_limit turned on

            if (this->edge_limit(dim)) {
                double a = *(this->axis(dim).begin());
                double b = *(this->axis(dim).rbegin());
                double inc = this->axis(dim).increment(0);
                if (inc < 0) {                 // a > b
                    if (location[dim] >= a) {  // left of the axis
                        location[dim] = a;
                        offset[dim] = 0;
                    } else if (location[dim] <= b) {  // right of the axis
                        location[dim] = b;
                        offset[dim] = this->axis(dim).size() - 2;
                    } else {
                        offset[dim] = this->axis(dim).find_index(
                            location[dim]);  // somewhere in-between the
                                             // endpoints of the axis
                    }
                }
                if (inc > 0) {                 // a < b
                    if (location[dim] <= a) {  // left of the axis
                        location[dim] = a;
                        offset[dim] = 0;
                    } else if (location[dim] >= b) {  // right of the axis
                        location[dim] = b;
                        offset[dim] = this->axis(dim).size() - 2;
                    } else {
                        offset[dim] = this->axis(dim).find_index(
                            location[dim]);  // somewhere in-between the
                                             // endpoints of the axis
                    }
                }

                // allow extrapolation if _edge_limit turned off

            } else {
                offset[dim] = this->axis(dim).find_index(location[dim]);
            }
        }

        // PCHIP contribution in zeroth dimension

        if (derivative) {
            derivative[0] = 0;
        }
        k0 = offset[0];
        k1 = offset[1];
        k2 = offset[2];

        // construct the interpolated plane to which the final bi-linear
        // interpolation will happen

        c_matrix<double, 2, 2> interp_plane;
        c_matrix<double, 2, 2> dz;

        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 2; ++j) {
                // extract data and take precautions when at boundaries of the
                // axis
                double v1 = data_3d(k0, k1 + i, k2 + j);
                double v2 = data_3d(k0 + 1, k1 + i, k2 + j);
                inc1 = this->axis(0).increment(k0);

                t = (location[0] - this->axis(0)(k0)) / inc1;
                t_2 = t * t;
                t_3 = t_2 * t;

                // construct the hermite polynomials
                h00 = (2 * t_3 - 3 * t_2 + 1);
                h10 = (t_3 - 2 * t_2 + t);
                h01 = (3 * t_2 - 2 * t_3);
                h11 = (t_3 - t_2);

                interp_plane(i, j) =
                    h00 * v1 + inc1 * h10 * _derv_z[k0][k1 + i][k2 + j] +
                    h01 * v2 + inc1 * h11 * _derv_z[k0 + 1][k1 + i][k2 + j];

                if (derivative) {
                    dz(i, j) =
                        (6 * t_2 - 6 * t) * v1 / inc1 +
                        (3 * t_2 - 4 * t + 1) * _derv_z[k0][k1 + i][k2 + j] +
                        (6 * t - 6 * t_2) * v2 / inc1 +
                        (3 * t_2 - 2 * t) * _derv_z[k0 + 1][k1 + i][k2 + j];
                }
            }
        }

        //** Bi-Linear contributions from first/second dimensions */
        // extract data around field point
        x = location[1];
        x1 = this->axis(1)(k1);
        x2 = this->axis(1)(k1 + 1);
        y = location[2];
        y1 = this->axis(2)(k2);
        y2 = this->axis(2)(k2 + 1);
        f11 = interp_plane(0, 0);
        f21 = interp_plane(1, 0);
        f12 = interp_plane(0, 1);
        f22 = interp_plane(1, 1);
        x_diff = x2 - x1;
        y_diff = y2 - y1;

        result = (f11 * (x2 - x) * (y2 - y) + f21 * (x - x1) * (y2 - y) +
                  f12 * (x2 - x) * (y - y1) + f22 * (x - x1) * (y - y1)) /
                 (x_diff * y_diff);

        if (derivative) {
            derivative[0] = (dz(0, 0) * (x2 - x) * (y2 - y) +
                             dz(1, 0) * (x - x1) * (y2 - y) +
                             dz(0, 1) * (x2 - x) * (y - y1) +
                             dz(1, 1) * (x - x1) * (y - y1)) /
                            (x_diff * y_diff);
            derivative[1] = (-f11 * (y2 - y) + f21 * (y2 - y) - f12 * (y - y1) +
                             f22 * (y - y1)) /
                            (x_diff * y_diff);
            derivative[2] = (-f11 * (x2 - x) - f21 * (x - x1) + f12 * (x2 - x) +
                             f22 * (x - x1)) /
                            (x_diff * y_diff);
        }

        return result;

    }  // end interpolate at a single location.

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
                     matrix<double>* dx = nullptr, matrix<double>* dy = nullptr,
                     matrix<double>* dz = nullptr) const {
        double location[3];
        double derivative[3];
        for (size_t n = 0; n < x.size1(); ++n) {
            for (size_t m = 0; m < x.size2(); ++m) {
                location[0] = x(n, m);
                location[1] = y(n, m);
                location[2] = z(n, m);
                if (dx == nullptr || dy == nullptr || dz == nullptr) {
                    (*result)(n, m) = (double)interpolate(location);
                } else {
                    (*result)(n, m) = (double)interpolate(location, derivative);
                    (*dx)(n, m) = (double)derivative[0];
                    (*dy)(n, m) = (double)derivative[1];
                    (*dz)(n, m) = (double)derivative[2];
                }
            }
        }

    }  // end interpolate

   private:
    /** Utility accessor function for data grid values */
    inline double data_3d(size_t dim0, size_t dim1, size_t dim2) const {
        size_t grid_index[3];
        grid_index[0] = dim0;
        grid_index[1] = dim1;
        grid_index[2] = dim2;
        return data(grid_index);

    }  // end data_3d

    /**
     * Create all variables needed for each calculation once
     * to same time and memory.
     */
    size_t _kzmax, _kxmax, _kymax;  // max index on z-axis (depth)
    double*** _derv_z;

};  // end data_grid_svp class

}  // end of namespace types
}  // end of namespace usml
