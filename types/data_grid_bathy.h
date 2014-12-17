/**
 * @file data_grid_bathy.h
 * Wrapper for a data_grid in 2d that uses the fast non-recursive
 * interpolation algorithm.
 */
#ifndef USML_TYPES_DATA_GRID_BATHY_H
#define USML_TYPES_DATA_GRID_BATHY_H

#include <usml/types/data_grid.h>
#include <usml/types/seq_vector.h>

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
 * WARNING: This wrapper is specific to 2-dimensional grids
 *          only. For 3-dimensional grids, the user should
 *          implement the data_grid_svp wrapper instead.
 *
 * Since the data is passed in and referenced by this wrapper,
 * the data is taken control of and destroyed at the end of its
 * use cycle.
 */

class USML_DECLSPEC data_grid_bathy: public data_grid<double, 2> {

    public:
        /**
         * Constructor - Creates a fast interpolation grid from an existing data_grid.
         * Also constructs the inverse bicubic coefficient matrix to be
         * used at a later time during pchip calculations.
         *
         * @param grid      The data_grid that is to be wrapped.
         * @param copy_data If true, copies the data grids data
         *                  fields as well as the axises.
         */

        data_grid_bathy(const data_grid<double, 2>* grid, bool copy_data = true) :
                data_grid<double, 2>(*grid, copy_data), _bicubic_coeff(16, 1),
                _field(16, 1), _xyloc(1, 16), _result_pchip(1, 1), _value(4, 4),
                _kmin(0u), _k0max(_axis[0]->size() - 1u), _k1max(_axis[1]->size() - 1u)
        {
            // Construct the inverse bicubic interpolation coefficient matrix
            _inv_bicubic_coeff = zero_matrix<double>(16, 16);
            _inv_bicubic_coeff(0, 0) = 1;
            _inv_bicubic_coeff(1, 8) = 1;
            _inv_bicubic_coeff(2, 0) = -3;
            _inv_bicubic_coeff(2, 1) = 3;
            _inv_bicubic_coeff(2, 8) = -2;
            _inv_bicubic_coeff(2, 9) = -1;
            _inv_bicubic_coeff(3, 0) = 2;
            _inv_bicubic_coeff(3, 1) = -2;
            _inv_bicubic_coeff(3, 8) = _inv_bicubic_coeff(3, 9) = 1;
            _inv_bicubic_coeff(4, 4) = 1;
            _inv_bicubic_coeff(5, 12) = 1;
            _inv_bicubic_coeff(6, 4) = -3;
            _inv_bicubic_coeff(6, 5) = 3;
            _inv_bicubic_coeff(6, 12) = -2;
            _inv_bicubic_coeff(6, 13) = -1;
            _inv_bicubic_coeff(7, 4) = 2;
            _inv_bicubic_coeff(7, 5) = -2;
            _inv_bicubic_coeff(7, 12) = _inv_bicubic_coeff(7, 13) = 1;
            _inv_bicubic_coeff(8, 0) = -3;
            _inv_bicubic_coeff(8, 2) = 3;
            _inv_bicubic_coeff(8, 4) = -2;
            _inv_bicubic_coeff(8, 6) = -1;
            _inv_bicubic_coeff(9, 8) = -3;
            _inv_bicubic_coeff(9, 10) = 3;
            _inv_bicubic_coeff(9, 12) = -2;
            _inv_bicubic_coeff(9, 14) = -1;
            _inv_bicubic_coeff(10, 0) = _inv_bicubic_coeff(10, 3) = 9;
            _inv_bicubic_coeff(10, 1) = _inv_bicubic_coeff(10, 2) = -9;
            _inv_bicubic_coeff(10, 4) = _inv_bicubic_coeff(10, 8) = 6;
            _inv_bicubic_coeff(10, 5) = _inv_bicubic_coeff(10, 10) = -6;
            _inv_bicubic_coeff(10, 6) = _inv_bicubic_coeff(10, 9) = 3;
            _inv_bicubic_coeff(10, 7) = _inv_bicubic_coeff(10, 11) = -3;
            _inv_bicubic_coeff(10, 12) = 4;
            _inv_bicubic_coeff(10, 13) = _inv_bicubic_coeff(10, 14) = 2;
            _inv_bicubic_coeff(10, 15) = 1;
            _inv_bicubic_coeff(11, 0) = _inv_bicubic_coeff(11, 3) = -6;
            _inv_bicubic_coeff(11, 1) = _inv_bicubic_coeff(11, 2) = 6;
            _inv_bicubic_coeff(11, 6) = _inv_bicubic_coeff(11, 12) =
                        _inv_bicubic_coeff(11, 13) = -2;
            _inv_bicubic_coeff(11, 4) = -4;
            _inv_bicubic_coeff(11, 5) = 4;
            _inv_bicubic_coeff(11, 7) = 2;
            _inv_bicubic_coeff(11, 8) = _inv_bicubic_coeff(11, 9) = -3;
            _inv_bicubic_coeff(11, 10) = _inv_bicubic_coeff(11, 11) = 3;
            _inv_bicubic_coeff(11, 14) = _inv_bicubic_coeff(11, 15) = -1;
            _inv_bicubic_coeff(12, 0) = 2;
            _inv_bicubic_coeff(12, 2) = -2;
            _inv_bicubic_coeff(12, 4) = _inv_bicubic_coeff(12, 6) = 1;
            _inv_bicubic_coeff(13, 8) = 2;
            _inv_bicubic_coeff(13, 10) = -2;
            _inv_bicubic_coeff(13, 12) = _inv_bicubic_coeff(13, 14) = 1;
            _inv_bicubic_coeff(14, 0) = _inv_bicubic_coeff(14, 3) = -6;
            _inv_bicubic_coeff(14, 1) = _inv_bicubic_coeff(14, 2) = 6;
            _inv_bicubic_coeff(14, 4) = _inv_bicubic_coeff(14, 6) = -3;
            _inv_bicubic_coeff(14, 5) = _inv_bicubic_coeff(14, 7) = 3;
            _inv_bicubic_coeff(14, 8) = -4;
            _inv_bicubic_coeff(14, 10) = 4;
            _inv_bicubic_coeff(14, 9) = _inv_bicubic_coeff(14, 12) =
                    _inv_bicubic_coeff(14, 14) = -2;
            _inv_bicubic_coeff(14, 11) = 2;
            _inv_bicubic_coeff(14, 13) = _inv_bicubic_coeff(14, 15) = -1;
            _inv_bicubic_coeff(15, 0) = _inv_bicubic_coeff(15, 3) = 4;
            _inv_bicubic_coeff(15, 1) = _inv_bicubic_coeff(15, 2) = -4;
            _inv_bicubic_coeff(15, 4) = _inv_bicubic_coeff(15, 6) =
                    _inv_bicubic_coeff(15, 8) = _inv_bicubic_coeff(15, 9) = 2;
            _inv_bicubic_coeff(15, 5) = _inv_bicubic_coeff(15, 7) =
                    _inv_bicubic_coeff(15, 10) = _inv_bicubic_coeff(15, 11) = -2;
            _inv_bicubic_coeff(15, 12) = _inv_bicubic_coeff(15, 13) =
                    _inv_bicubic_coeff(15, 14) = _inv_bicubic_coeff(15, 15) = 1;

            _fast_index[0] = 0;
            _fast_index[1] = 0;

            //Pre-construct increments for all intervals once to save time
            matrix<double> inc_x(_k0max + 1u, 1);
            for (unsigned i = 0; i < _k0max + 1u; ++i) {
                if (i == 0 || i == _k0max) {
                    inc_x(i, 0) = 2;
                } else {
                    inc_x(i, 0) = (_axis[0]->increment(i - 1)
                            + _axis[0]->increment(i + 1)) / _axis[0]->increment(i);
                }
            }
            matrix<double> inc_y(_k1max + 1u, 1);
            for (unsigned i = 0; i < _k1max + 1u; ++i) {
                if (i == 0 || i == _k1max) {
                    inc_y(i, 0) = 2;
                } else {
                    inc_y(i, 0) = (_axis[1]->increment(i - 1)
                            + _axis[1]->increment(i + 1)) / _axis[1]->increment(i);
                }
            }

            // Pre-construct all derivatives and cross-dervs once to save time
            _derv_x = matrix<double>(_k0max + 1u, _k1max + 1u);
            _derv_y = matrix<double>(_k0max + 1u, _k1max + 1u);
            _derv_x_y = matrix<double>(_k0max + 1u, _k1max + 1u);
            for (unsigned i = 0; i < _k0max + 1u; ++i) {
                for (unsigned j = 0; j < _k1max + 1u; ++j) {
                    if (i < 1 && j < 1) {                      //top-left corner
                        _derv_x(i, j) = (data_2d(i + 1, j) - data_2d(i, j))
                                / inc_x(i, 0);
                        _derv_y(i, j) = (data_2d(i, j + 1) - data_2d(i, j))
                                / inc_y(j, 0);
                        _derv_x_y(i, j) = (data_2d(i + 1, j + 1) - data_2d(i + 1, j)
                                - data_2d(i, j + 1) + data_2d(i, j))
                                / (inc_x(i, 0) * inc_y(j, 0));
                    } else if (i == _k0max && j == _k1max) {     //bottom-right corner
                        _derv_x(i, j) = (data_2d(i, j) - data_2d(i - 1, j))
                                / inc_x(i, 0);
                        _derv_y(i, j) = (data_2d(i, j) - data_2d(i, j - 1))
                                / inc_y(j, 0);
                        _derv_x_y(i, j) = (data_2d(i, j) - data_2d(i, j - 1)
                                - data_2d(i - 1, j) + data_2d(i - 1, j - 1))
                                / (inc_x(i, 0) * inc_y(j, 0));
                    } else if (i < 1 && j == _k1max) {             //top-right corner
                        _derv_x(i, j) = (data_2d(i + 1, j) - data_2d(i, j))
                                / inc_x(i, 0);
                        _derv_y(i, j) = (data_2d(i, j) - data_2d(i, j - 1))
                                / inc_y(j, 0);
                        _derv_x_y(i, j) = (data_2d(i + 1, j) - data_2d(i + 1, j - 1)
                                - data_2d(i, j) + data_2d(i, j - 1))
                                / (inc_x(i, 0) * inc_y(j, 0));
                    } else if (j < 1 && i == _k0max) {           //bottom-left corner
                        _derv_x(i, j) = (data_2d(i, j) - data_2d(i - 1, j))
                                / inc_x(i, 0);
                        _derv_y(i, j) = (data_2d(i, j + 1) - data_2d(i, j))
                                / inc_y(j, 0);
                        _derv_x_y(i, j) = (data_2d(i, j + 1) - data_2d(i, j)
                                - data_2d(i - 1, j + 1) + data_2d(i - 1, j))
                                / (inc_x(i, 0) * inc_y(j, 0));
                    } else if (i < 1 && (1 <= j && j < _k1max)) {       //top row
                        _derv_x(i, j) = (data_2d(i + 1, j) - data_2d(i, j))
                                / inc_x(i, 0);
                        _derv_y(i, j) = (data_2d(i, j + 1) - data_2d(i, j - 1))
                                / inc_y(j, 0);
                        _derv_x_y(i, j) = (data_2d(i + 1, j + 1)
                                - data_2d(i + 1, j - 1) - data_2d(i, j + 1)
                                + data_2d(i, j - 1)) / (inc_x(i, 0) * inc_y(j, 0));
                    } else if (j < 1 && (1 <= i && i < _k0max)) {  //left most column
                        _derv_x(i, j) = (data_2d(i + 1, j) - data_2d(i - 1, j))
                                / inc_x(i, 0);
                        _derv_y(i, j) = (data_2d(i, j + 1) - data_2d(i, j))
                                / inc_y(j, 0);
                        _derv_x_y(i, j) = (data_2d(i + 1, j + 1) - data_2d(i + 1, j)
                                - data_2d(i - 1, j + 1) + data_2d(i - 1, j))
                                / (inc_x(i, 0) * inc_y(j, 0));
                    } else if (j == _k1max && (1 <= i && i < _k0max)) { //right most column
                        _derv_x(i, j) = (data_2d(i + 1, j) - data_2d(i - 1, j))
                                / inc_x(i, 0);
                        _derv_y(i, j) = (data_2d(i, j) - data_2d(i, j - 1))
                                / inc_y(j, 0);
                        _derv_x_y(i, j) = (data_2d(i + 1, j) - data_2d(i + 1, j - 1)
                                - data_2d(i - 1, j) + data_2d(i - 1, j - 1))
                                / (inc_x(i, 0) * inc_y(j, 0));
                    } else if (i == _k0max && (1 <= j && j < _k1max)) {   //bottom row
                        _derv_x(i, j) = (data_2d(i, j) - data_2d(i - 1, j))
                                / inc_x(i, 0);
                        _derv_y(i, j) = (data_2d(i, j + 1) - data_2d(i, j - 1))
                                / inc_y(j, 0);
                        _derv_x_y(i, j) = (data_2d(i, j + 1) - data_2d(i, j - 1)
                                - data_2d(i - 1, j + 1) + data_2d(i - 1, j - 1))
                                / (inc_x(i, 0) * inc_y(j, 0));
                    } else {                                //inside, restrictive
                        _derv_x(i, j) = (data_2d(i + 1, j) - data_2d(i - 1, j))
                                / inc_x(i, 0);
                        _derv_y(i, j) = (data_2d(i, j + 1) - data_2d(i, j - 1))
                                / inc_y(j, 0);
                        _derv_x_y(i, j) = (data_2d(i + 1, j + 1)
                                - data_2d(i + 1, j - 1) - data_2d(i - 1, j + 1)
                                + data_2d(i - 1, j - 1))
                                / (inc_x(i, 0) * inc_y(j, 0));
                    }
                } //end for-loop in j
            } //end for-loop in i
            delete grid ;
        }// end constructor

        /**
         * Destructor
         */
        virtual ~data_grid_bathy() {}

        /**
         * Overrides the interpolate function within data_grid using the
         * non-recursive formula. Determines which interpolate function to
         * based on the interp_type enumeral stored within the 0th dimensional
         * axis.
         *
         * Interpolate at a single location.
         *
         * @param location   Location to do the interpolation at
         * @param derivative Derivative at the location (output)
         * @return           Returns the value at the field location
         */

        double interpolate(double* location, double* derivative = NULL) {

            double result = 0;
            // find the interval index in each dimension

            for (unsigned dim = 0; dim < 2; ++dim) {

                // limit interpolation to axis domain if _edge_limit turned on

                if (edge_limit(dim)) {
                    double a = *(_axis[dim]->begin());
                    double b = *(_axis[dim]->rbegin());
                    double inc = _axis[dim]->increment(0);
                    if (inc < 0) {                                          // a > b
                        if (location[dim] >= a) {                //left of the axis
                            location[dim] = a;
                            _offset[dim] = 0;
                        } else if (location[dim] <= b) {         //right of the axis
                            location[dim] = b;
                            _offset[dim] = _axis[dim]->size() - 2;
                        } else {
                            _offset[dim] = _axis[dim]->find_index(location[dim]); //somewhere in-between the endpoints of the axis
                        }
                    }
                    if (inc > 0) {                                          // a < b
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

            switch (interp_type(0)) {

            //****nearest****
            case -1:
                for (int dim = 0; dim < 2; ++dim) {
                    double inc = _axis[dim]->increment(0);
                    double u = abs(location[dim] - (*_axis[dim])(_offset[dim]))
                            / inc;
                    if (u < 0.5) {
                        _fast_index[dim] = _offset[dim];
                    } else {
                        _fast_index[dim] = _offset[dim] + 1;
                    }
                }
                if (derivative)
                    derivative[0] = derivative[1] = 0;
                return data(_fast_index);
                break;

                //****linear****
            case 0:
                double f11, f21, f12, f22, x_diff, y_diff;
                double x, x1, x2, y, y1, y2;

                x = location[0];
                x1 = (*_axis[0])(_offset[0]);
                x2 = (*_axis[0])(_offset[0] + 1);
                y = location[1];
                y1 = (*_axis[1])(_offset[1]);
                y2 = (*_axis[1])(_offset[1] + 1);
                f11 = data(_offset);
                _fast_index[0] = _offset[0] + 1;
                _fast_index[1] = _offset[1];
                f21 = data(_fast_index);
                _fast_index[0] = _offset[0];
                _fast_index[1] = _offset[1] + 1;
                f12 = data(_fast_index);
                _fast_index[0] = _offset[0] + 1;
                _fast_index[1] = _offset[1] + 1;
                f22 = data(_fast_index);
                x_diff = x2 - x1;
                y_diff = y2 - y1;
                result = (f11 * (x2 - x) * (y2 - y) + f21 * (x - x1) * (y2 - y)
                        + f12 * (x2 - x) * (y - y1) + f22 * (x - x1) * (y - y1))
                        / (x_diff * y_diff);
                if (derivative) {
                    derivative[0] = (f21 * (y2 - y) - f11 * (y2 - y)
                            + f22 * (y - y1) - f12 * (y - y1)) / (x_diff * y_diff);
                    derivative[1] = (f12 * (x2 - x) - f11 * (x2 - x)
                            + f22 * (x - x1) - f21 * (x - x1)) / (x_diff * y_diff);
                }
                return result;
                break;

                //****pchip****
            case 1:
                result = fast_pchip(_offset, location, derivative);
                return result;
                break;

            default:
                throw std::invalid_argument(
                        "Interp must be NEAREST, LINEAR, or PCHIP");
                break;
            }
        } // end interpolate at a single location

        /**
         * Overrides the interpolate function within data_grid using the
         * non-recursive formula.
         *
         * Interpolate at a series of locations.
         *
         * @param   x           First dimension of location.
         * @param   y           Second dimension of location.
         * @param   result      Interpolated values at each location (output).
         * @param   dx          First dimension of derivative (output).
         * @param   dy          Second dimension of derivative (output).
         */

        void interpolate(const matrix<double>& x, const matrix<double>& y,
                matrix<double>* result, matrix<double>* dx = NULL,
                matrix<double>* dy = NULL) {
            double location[2];
            double derivative[2];
            for (unsigned n = 0; n < x.size1(); ++n) {
                for (unsigned m = 0; m < x.size2(); ++m) {
                    location[0] = x(n, m);
                    location[1] = y(n, m);
                    if (dx == NULL || dy == NULL) {
                        (*result)(n, m) = (double) interpolate(location);
                    } else {
                        (*result)(n, m) = (double) interpolate(location,
                                derivative);
                        (*dx)(n, m) = (double) derivative[0];
                        (*dy)(n, m) = (double) derivative[1];
                    }
                }
            }
        } // end Interpolate at a series of locations.

    private:

        /** Utility accessor function for data grid values */
        inline double data_2d(unsigned row, unsigned col) {
            unsigned grid_index[2];
            grid_index[0] = row;
            grid_index[1] = col;
            return data(grid_index);
        }

        /**
         * A non-recursive version of the Piecewise Cubic Hermite
         * polynomial (PCHIP) specific to the 2-dimensional grid of
         * data. This algorithm was generated from the below formula and
         * by performing a linear transformation from the data set to the
         * interval [0,1] x [0,1]:
         *
         *  g(x,y) = sum [i,0,3] (
         *                sum [j,0,3] (
         *                    a_ij * x^i * y^j
         *                )
         *           )
         *
         * where (x,y) is the point of interpolation and a_ij are known
         * as the bicubic interpolation coefficients. An inverse matrix
         * is then constructed from the 16 equations that are generated.
         * Using this inverse matrix and the 4 surround data points, their
         * respective derivatives with respect to x and y, and mixed xy
         * xy derivatives, we can construct each a_ij.
         *
         * The partial and mixed derivates are computed using a centered
         * differencing approximation, so that,
         *
         *      f_x(i,j) = [ f(i+1,j) - f(i-1,j) ] / [ x_(i+1) - x(i-1) ]
         *      f_y(i,j) = [ f(i,j+1) - f(i,j-1) ] / [ y_(i+1) - y(i-1) ]
         *      f_xy(i,j) = { f(i+1,j+1) - f(i+1,j-1) - f(i-1,j+1) +
         *                  f(i-1,j-1) } / [ x_(i+1) - x(i-1) ] *
         *                  [ y_(i+1) - y(i-1) ]
         *
         * @xref http://en.wikipedia.org/wiki/Bicubic_interpolation
         * @xref http://en.wikipedia.org/wiki/Finite_difference
         *
         * Below is a representation of the data extracted from the field
         * and how it is stored within the matrix:
         *
         *              * field(1,0)                  * field(3,0)
         *                      * (interp point)
         *
         *              * field(0,0)                  * field(2,0)
         *
         * field(0,0) to field(3,0) are the data points that surrounding the
         * interpolation point. field(4,0) to field(7,0) are the derivatives
         * with respect to x of the extracted data points. field(8,0) to
         * field(11,0) are the derivatives with respect to y of the extracted
         * data points. field(12,0) to field(15,0) are the derivatives with
         * respect to x and y of the extracted data points.
         *
         * @param interp_index  index on the grid for the closest data point
         * @param location      Location of the field calculation
         * @param derivative    Generate the derivative at the location (output)
         * @return              Returns the value at the field location
         */
        double fast_pchip(const unsigned* interp_index, double* location,
                double* derivative = NULL) {
            int k0 = interp_index[0];
            int k1 = interp_index[1];
            double norm0, norm1;

            // Checks for boundaries of the axes
            norm0 = (*_axis[0])(k0 + 1) - (*_axis[0])(k0);
            norm1 = (*_axis[1])(k1 + 1) - (*_axis[1])(k1);
            for (int i = -1; i < 3; ++i) {
                for (int j = -1; j < 3; ++j) {
                    //get appropriate data when at boundaries
                    if ((k0 + i) >= _k0max) {
                        _fast_index[0] = _k0max;
                    } else if ((k0 + i) <= _kmin) {
                        _fast_index[0] = _kmin;
                    } else {
                        _fast_index[0] = k0 + i;
                    }
                    //get appropriate data when at boundaries
                    if ((k1 + j) >= _k1max) {
                        _fast_index[1] = _k1max;
                    } else if ((k1 + j) <= _kmin) {
                        _fast_index[1] = _kmin;
                    } else {
                        _fast_index[1] = k1 + j;
                    }
                    _value(i + 1, j + 1) = data(_fast_index);
                }   //end for-loop in j
            }   //end for-loop in i

            // Construct the _field matrix
            _field(0, 0) = _value(1, 1);                  //f(0,0)
            _field(1, 0) = _value(1, 2);                  //f(0,1)
            _field(2, 0) = _value(2, 1);                  //f(1,0)
            _field(3, 0) = _value(2, 2);                  //f(1,1)
            _field(4, 0) = _derv_x(k0, k1);               //f_x(0,0)
            _field(5, 0) = _derv_x(k0, k1 + 1);           //f_x(0,1)
            _field(6, 0) = _derv_x(k0 + 1, k1);           //f_x(1,0)
            _field(7, 0) = _derv_x(k0 + 1, k1 + 1);       //f_x(1,1)
            _field(8, 0) = _derv_y(k0, k1);               //f_y(0,0)
            _field(9, 0) = _derv_y(k0, k1 + 1);           //f_y(0,1)
            _field(10, 0) = _derv_y(k0 + 1, k1);          //f_y(1,0)
            _field(11, 0) = _derv_y(k0 + 1, k1 + 1);      //f_y(1,1)
            _field(12, 0) = _derv_x_y(k0, k1);            //f_x_y(0,0)
            _field(13, 0) = _derv_x_y(k0, k1 + 1);        //f_x_y(0,1)
            _field(14, 0) = _derv_x_y(k0 + 1, k1);        //f_x_y(1,0)
            _field(15, 0) = _derv_x_y(k0 + 1, k1 + 1);    //f_x_y(1,1)

            // Construct the coefficients of the bicubic interpolation
            _bicubic_coeff = prod(_inv_bicubic_coeff, _field);

            // Create the power series of the interpolation formula before hand for speed
            double x_inv = location[0] - (*_axis[0])(k0);
            double y_inv = location[1] - (*_axis[1])(k1);

            _xyloc(0, 0) = 1;
            _xyloc(0, 1) = y_inv / norm1;
            _xyloc(0, 2) = _xyloc(0, 1) * _xyloc(0, 1);
            _xyloc(0, 3) = _xyloc(0, 2) * _xyloc(0, 1);
            _xyloc(0, 4) = x_inv / norm0;
            _xyloc(0, 5) = _xyloc(0, 4) * _xyloc(0, 1);
            _xyloc(0, 6) = _xyloc(0, 4) * _xyloc(0, 2);
            _xyloc(0, 7) = _xyloc(0, 4) * _xyloc(0, 3);
            _xyloc(0, 8) = _xyloc(0, 4) * _xyloc(0, 4);
            _xyloc(0, 9) = _xyloc(0, 8) * _xyloc(0, 1);
            _xyloc(0, 10) = _xyloc(0, 8) * _xyloc(0, 2);
            _xyloc(0, 11) = _xyloc(0, 8) * _xyloc(0, 3);
            _xyloc(0, 12) = _xyloc(0, 8) * _xyloc(0, 4);
            _xyloc(0, 13) = _xyloc(0, 12) * _xyloc(0, 1);
            _xyloc(0, 14) = _xyloc(0, 12) * _xyloc(0, 2);
            _xyloc(0, 15) = _xyloc(0, 12) * _xyloc(0, 3);

            _result_pchip = prod(_xyloc, _bicubic_coeff);
            if (derivative) {
                derivative[0] = 0;
                derivative[1] = 0;
                for (int i = 1; i < 4; ++i) {
                    for (int j = 0; j < 4; ++j) {
                        derivative[0] += i * _bicubic_coeff(i * 4 + j, 0)
                                * _xyloc(0, 4 * (i - 1)) * _xyloc(0, j);
                    }
                }
                for (int i = 0; i < 4; ++i) {
                    for (int j = 1; j < 4; ++j) {
                        derivative[1] += j * _bicubic_coeff(i * 4 + j, 0)
                                * _xyloc(0, 4 * i) * _xyloc(0, j - 1);
                    }
                }
            }
            return _result_pchip(0, 0);
        }

        //***********************************************************/
        // Private data members

        /**
         * Matrix that is the inverse of the bicubic coefficients
         * This matrix will be used to construct the bicubic
         * coefficients. The result will be a 16x1 matrix.
         */
        c_matrix<double, 16, 16> _inv_bicubic_coeff;

        /**
         * Create variables that are used multiple times through out
         * the course of multiple calls per instance. Thus allowing
         * these variables to be created once and used as many times
         * as needed and save memory and time.
         */

        c_matrix<double, 16, 1> _bicubic_coeff;
        c_matrix<double, 16, 1> _field;
        c_matrix<double, 1, 16> _xyloc;
        c_matrix<double, 1, 1> _result_pchip;
        c_matrix<double, 4, 4> _value;
        matrix<double> _derv_x;
        matrix<double> _derv_y;
        matrix<double> _derv_x_y;
        unsigned  _fast_index[2];
        const int _kmin;
        const int _k0max;
        const int _k1max;

}; // end data_grid_bathy

} // end of namespace types
} // end of namespace usml

#endif
