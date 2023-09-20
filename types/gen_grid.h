/**
 * @file gen_grid.h
 * N-dimensional data set and its associated axes.
 */
#pragma once

#include <usml/types/data_grid.h>
#include <usml/types/gen_grid_utils.h>
#include <usml/types/seq_vector.h>
#include <usml/ublas/randgen.h>
#include <usml/usml_config.h>

#include <array>
#include <cstddef>
#include <memory>
#include <stdexcept>

namespace usml {
namespace types {

/// @ingroup data_grid
/// @{

/**
 * Generic N-dimensional data set and its associated axes.
 * Editable class that supports interpolation in any number of dimensions.
 *
 * @param  NUM_DIMS     Number of dimensions in this grid.  Specifying this
 *                      at compile time allows for some loop un-wrapping.
 */
template <size_t NUM_DIMS, class DATA_TYPE = double>
class USML_DLLEXPORT gen_grid : public data_grid<NUM_DIMS, DATA_TYPE> {
   public:
    /**
     * Create data grid from its associated axes.
     * Allocates new memory for the data at each grid point.
     * Initialize all of the interpolation types to interp_enum::linear.
     *
     * @param axis  Axes to use for each dimension of the grid.
     */
    gen_grid(const seq_vector::csptr axis[]) {
        size_t N = 1;
        for (size_t n = 0; n < NUM_DIMS; ++n) {
            this->_axis[n] = axis[n];
            N *= this->_axis[n]->size();
        }
        this->_writeable_data = std::shared_ptr<DATA_TYPE[]>(new DATA_TYPE[N]);
        this->_data = this->_writeable_data;  // read only reference
        initialize<DATA_TYPE>::zero_n(this->_writeable_data.get(), N);
    }

    /**
     * Define a new data value at a specific combination of indices.
     *
     * @param  index            Index number in each dimension.
     * @param  value            Value to insert at this location.
     */
    void setdata(const size_t* index, DATA_TYPE value) {
        const size_t offset =
            data_grid_compute_offset<NUM_DIMS - 1>(this->_axis, index);
        this->_writeable_data.get()[offset] = value;
    }

    /**
     * Multi-dimensional interpolation with the derivative calculation.
     * So many calculations are shared between the determination of an
     * interpolate value and its derivative, that it is computationally
     * efficient to compute them both at the same time.
     *
     * Limit interpolation to axis domain if _edge_limit turned on for that
     * dimension.  Allow extrapolation if _edge_limit turned off.
     *
     * @param   location    Location at which field value is desired. Must
     *                      have the same rank as the data grid or higher.
     *                      WARNING: The contents of the location vector
     *                      may be modified if edge_limit() is true for
     *                      any dimension.
     * @param   derivative  If this is not nullptr, the first derivative
     *                      of the field at this point will also be computed.
     * @return              Value of the field at this point.
     */
    DATA_TYPE interpolate(const double location[],
                          DATA_TYPE* derivative = nullptr) const {
        size_t index[NUM_DIMS];
        double loc[NUM_DIMS];  // create copy to allow updating
        std::copy_n(location, NUM_DIMS, loc);

        for (size_t dim = 0; dim < NUM_DIMS; ++dim) {
            seq_vector::csptr ax = this->_axis[dim];
            assert(!std::isnan(loc[dim]));

            // limit interpolation to axis domain if _edge_limit turned on

            if (this->_edge_limit[dim]) {
                double a = (*ax)(0);
                double b = (*ax)(ax->size() - 1);
                double sign = (ax->increment(0) < 0) ? -1.0 : 1.0;
                double d = loc[dim] * sign;
                if (d <= a * sign) {  // left of the axis
                    loc[dim] = a;
                    index[dim] = 0;
                } else if (d >= b * sign) {  // right of the axis
                    loc[dim] = b;
                    index[dim] = this->_axis[dim]->size() - 2;
                } else {  // between end-points of axis
                    index[dim] = this->_axis[dim]->find_index(loc[dim]);
                }

                // allow extrapolation if _edge_limit turned off

            } else {
                index[dim] = this->_axis[dim]->find_index(loc[dim]);
            }
            assert(index[dim] >= 0 &&
                   index[dim] <= this->_axis[dim]->size() - 2);
        }

        // compute interpolation results for value and derivative

        DATA_TYPE dresult;
        return interp(NUM_DIMS - 1, index, loc, dresult, derivative);
    }

   private:
    //*************************************************************************
    // interpolation methods

    /**
     * Private recursion engine for multi-dimensional interpolation.
     * The type of interpolation for each dimension is determined using
     * the _interp_type[] field. Interpolation coefficients are computed on
     * the fly to make arbitrary combinations of interpolation types viable.
     *
     * @param   dim         Index of the dimension currently being processed.
     *                      Recursion starts at dim=NUM_DIMS-1 and reduces to
     *                      element retrieval when dim=-1.
     * @param   index       Position of the corner before the desired field
     *                      point. Must have the same rank as the data grid.
     * @param   location    Location at which field value is desired. Must
     *                      have the same NUM_DIM as the data grid or higher.
     * @param   deriv       Derivative for this iteration.
     * @param   deriv_vec   Results vector for derivative.
     *                      Derivative not computed if nullptr.
     * @return              Estimate of the field after interpolation.
     */
    DATA_TYPE interp(int dim, const size_t index[], const double location[],
                     DATA_TYPE& deriv, DATA_TYPE deriv_vec[]) const {
        DATA_TYPE result;

        if (dim < 0) {
            const size_t offset =
                data_grid_compute_offset<NUM_DIMS - 1>(this->_axis, index);
            result = this->_data.get()[offset];
            // terminates recursion

        } else {
            switch (this->_interp_type[dim]) {
                case interp_enum::nearest:
                    result = nearest(dim, index, location, deriv, deriv_vec);
                    break;
                case interp_enum::linear:
                    result = linear(dim, index, location, deriv, deriv_vec);
                    break;
                case interp_enum::pchip:
                    result = pchip(dim, index, location, deriv, deriv_vec);
                    break;
                default:
                    throw std::invalid_argument("bad interp type");
            }
        }
        return result;
    }

    /**
     * Perform a nearest neighbor interpolation on this dimension.
     *
     * @param   dim         Index of the dimension currently being processed.
     * @param   index       Position of the corner before the desired field
     *                      point. Must have the same NUM_DIM as the data grid.
     * @param   location    Location at which field value is desired. Must
     *                      have the same NUM_DIM as the data grid or higher.
     * @param   deriv       Derivative for this iteration. Always zero for
     *                      nearest neighbor interpolation.
     * @param   deriv_vec   Results vector for derivative.
     *                      Derivative not computed if nullptr.
     * @return              Estimate of the field after interpolation.
     */
    DATA_TYPE nearest(int dim, const size_t index[], const double location[],
                      DATA_TYPE& deriv, DATA_TYPE deriv_vec[]) const {
        DATA_TYPE result, da;

        // compute field value in this dimension

        const size_t k = index[dim];
        seq_vector::csptr ax = this->_axis[dim];
        const double u = (location[dim] - (*ax)(k)) / ax->increment(k);
        if (u < 0.5) {
            result = interp(dim - 1, index, location, da, deriv_vec);
        } else {
            size_t next[NUM_DIMS];
            std::copy_n(index, NUM_DIMS, next);
            ++next[dim];
            result = interp(dim - 1, next, location, da, deriv_vec);
        }

        // compute derivative in this dimension

        if (deriv_vec) {
            initialize<DATA_TYPE>::zero(deriv, result);
            deriv_vec[dim] = deriv;
            if (dim > 0) deriv_vec[dim - 1] = da;
        }

        // use results for dim+1 iteration

        return result;
    }

    /**
     * Perform a linear interpolation on this dimension.
     *
     * @param   dim         Index of the dimension currently being processed.
     * @param   index       Position of the corner before the desired field
     *                      point. Must have the same rank as the data grid.
     * @param   location    Location at which field value is desired. Must
     *                      have the same rank as the data grid or higher.
     * @param   deriv       Derivative for this iteration. Constant across the
     *                      interval for linear interpolation.
     * @param   deriv_vec   Results vector for derivative.
     *                      Derivative not computed if nullptr.
     * @return              Estimate of the field after interpolation.
     */
    DATA_TYPE linear(int dim, const size_t index[], const double location[],
                     DATA_TYPE& deriv, DATA_TYPE deriv_vec[]) const {
        DATA_TYPE result, da, db;

        // build interpolation coefficients

        const DATA_TYPE a = interp(dim - 1, index, location, da, deriv_vec);
        size_t next[NUM_DIMS];
        std::copy_n(index, NUM_DIMS, next);
        ++next[dim];
        const DATA_TYPE b = interp(dim - 1, next, location, db, deriv_vec);
        const size_t k = index[dim];
        seq_vector::csptr ax = this->_axis[dim];

        // compute field value in this dimension

        const double h = (double)ax->increment(k);
        const double u = (location[dim] - (*ax)(k)) / h;
        result = a * (1.0 - u) + b * u;

        // compute derivative in this dimension and prior dimension

        if (deriv_vec) {
            deriv = (b - a) / h;
            deriv_vec[dim] = deriv;
            if (dim > 0) {
                deriv_vec[dim - 1] = da * (1.0 - u) + db * u;
            }
        }

        // use results for dim+1 iteration

        return result;
    }

    /**
     * Interpolate this dimension using the Piecewise Cubic Hermite
     * Interpolation Polynomial (PCHIP) algorithm from Matlab.
     * Matlab uses shape preserving, "visually pleasing" version of the
     * cubic interpolant that does not suffer from the overshooting
     * issues prevalent in the cubic spline.  Although the the first
     * derivative of the PCHIP result is guaranteed to be continuous,
     * the second derivative has no such guarantee.
     *
     * This algorithm differs from the Matlab implementation in that
     * is simultaneously interpolates the function value for the current
     * dimension, and interpolates the derivative for the previous dimension.
     *
     * When using a gridded data set, it is recommended that edge_limit be set
     * to TRUE for any dimensional axis that uses the PCHIP interpolation. This
     * is because of PCHIP allowing for extreme values when extrapolating data.
     *
     * @xref Cleve Moler, Numerical Computing in Matlab, Chapter 3
     * Interpolation, http://www.mathworks.com/moler/chapters.html accessed
     * 5/15/2012.
     * @xref F. N. Fritsch and R. E. Carlson, Monotone Piecewise Cubic
     * Interpolation, SIAM Journal on Numerical Analysis, 17 (1980), pp.
     * 238-246.
     * @xref D. Kahaner, C. Moler, and S. Nash, Numerical Methods and Software,
     * Prentice{Hall, Englewood Cli®s, NJ, 1989.
     *
     * The basic algorithm assumes that the interpolation location is
     * in the interval [ x[k], x[k+1] ), where "k" is known as the
     * "interval index".  The result is then calculated from four unevenly
     * spaced points, and their forward (one-sided) derivatives.
     *
     * <pre>
     *         y0 = y[k-1]        h0 = x[k]-x[k-1]    deriv0 = (y1-y0)/h0
     *         y1 = y[k]        h1 = x[k+1]-x[k]    deriv1 = (y2-y1)/h1
     *         y2 = y[k+1]        h2 = x[k+2]-x[k+1]    deriv2 = (y3-y2)/h2
     *         y3 = y[k+2]        s = x - x[k]
     *
     * such that
     *
     *
     *      p(x) = y[k+1]  * ( 3 h1 s^2 - 2 s^3 ) / h1^3
     *             y[k]    * ( h1^3 - 3 h1 s^2 - 2 s^3 ) / h1^3
     *             slope[k+1] * ( s1^2 (s-h) ) / h1^2
     *             slope[k]   * ( s (s-h[k])^2 ) / h1^2
     *
     * where:
     *
     *        slope[k] = weighted harmonic average of deriv0, deriv1, deriv2
     * terms
     *
     * </pre>
     * At the end-points, y'[0] and y'[N-1] must be estimated.  This
     * implementation uses Matlab's non-centered, shape-preserving,
     * three-point formula for the end-point slope.
     *
     * @param   dim         Index of the dimension currently being processed.
     * @param   index       Position of the corner before the desired field
     *                      point. Must have the same rank as the data grid.
     * @param   location    Location at which field value is desired. Must
     *                      have the same rank as the data grid or higher.
     * @param   deriv       Derivative for this iteration. Constant across the
     *                      interval for linear interpolation.
     * @param   deriv_vec   Results vector for derivative.
     *                      Derivative not computed if nullptr.
     * @return              Estimate of the field after interpolation.
     */
    DATA_TYPE pchip(int dim, const size_t index[], const double location[],
                    DATA_TYPE& deriv, DATA_TYPE deriv_vec[]) const {
        DATA_TYPE result;
        seq_vector::csptr ax = this->_axis[dim];
        const size_t kmin = 1u;               // at endpt if k-1 < 0
        const size_t kmax = ax->size() - 3u;  // at endpt if k+2 > N-1

        //NOLINTBEGIN(clang-diagnostic-uninitialized)
        DATA_TYPE y0, y1, y2, y3;      // dim-1 values at k-1, k, k+1, k+2
        DATA_TYPE dy0, dy1, dy2, dy3;  // dim-1 derivs at k-1, k, k+1, k+2
        initialize<DATA_TYPE>::zero(dy0, dy1, dy2, dy3);
        //NOLINTEND(clang-diagnostic-uninitialized)

        // interpolate in dim-1 dimension to find values and derivs at k, k-1

        const size_t k = index[dim];
        y1 = interp(dim - 1, index, location, dy1, deriv_vec);

        if (k >= kmin) {
            size_t prev[NUM_DIMS];
            std::copy_n(index, NUM_DIMS, prev);
            --prev[dim];
            y0 = interp(dim - 1, prev, location, dy0, deriv_vec);
        } else {  // use harmless values at left end-point
            y0 = y1;
            dy0 = dy1;
        }

        // interpolate in dim-1 dimension to find values and derivs at k+1, k+2

        size_t next[NUM_DIMS];
        std::copy_n(index, NUM_DIMS, next);
        ++next[dim];
        y2 = interp(dim - 1, next, location, dy2, deriv_vec);

        if (k <= kmax) {
            size_t last[NUM_DIMS];
            std::copy_n(next, NUM_DIMS, last);
            ++last[dim];
            y3 = interp(dim - 1, last, location, dy3, deriv_vec);
        } else {  // use harmless values at right end-point
            y3 = y2;
            dy3 = dy2;
        }

        // compute difference values used frequently in computation

        const double h0 =
            double(ax->increment(k - 1));            // interval from k-1 to k
        const double h1 = double(ax->increment(k));  // interval from k to k+1
        const double h2 =
            double(ax->increment(k + 1));  // interval from k+1 to k+2
        const double h1_2 = h1 * h1;       // k to k+1 interval squared
        const double h1_3 = h1_2 * h1;     // k to k+1 interval cubed

        const double s = location[dim] - (*ax)(k);  // local variable
        const double s_2 = s * s, s_3 = s_2 * s;    // s squared and cubed
        const double sh_minus = s - h1;
        const double sh_term = 3.0 * h1 * s_2 - 2.0 * s_3;

        // compute first divided differences (forward derivative)
        // for both the values, and their derivatives

        const DATA_TYPE deriv0 = (y1 - y0) / h0;  // fwd deriv from k-1 to k
        const DATA_TYPE deriv1 = (y2 - y1) / h1;  // fwd deriv from k to k+1
        const DATA_TYPE deriv2 = (y3 - y2) / h2;  // fwd deriv from k+1 to k+2

        DATA_TYPE dderiv0, dderiv1, dderiv2;
        initialize<DATA_TYPE>::zero(dderiv0, dderiv1, dderiv2,
                                    y1);  // prevent valgrind from complaining
        if (deriv_vec) {                  // fwd deriv of dim-1 derivatives
            dderiv0 = (dy1 - dy0) / h0;
            dderiv1 = (dy2 - dy1) / h1;
            dderiv2 = (dy3 - dy2) / h2;
        }

        //*************
        // compute weighted harmonic mean of slopes around index k
        // for both the values, and their derivatives
        // set it zero at local maxima or minima
        // deriv0 * deriv1 condition guards against division by zero

        DATA_TYPE slope1, dslope1;
        initialize<DATA_TYPE>::zero(slope1, dslope1, y1);

        // when not at an end-point, slope1 is the harmonic, weighted
        // average of deriv0 and deriv1.

        if (k >= kmin) {
            const double w0 = 2.0 * h1 + h0;
            const double w1 = h1 + 2.0 * h0;
            derivative<DATA_TYPE>::compute(deriv0, deriv1, dderiv0, dderiv1, w0,
                                           w1, deriv_vec, slope1, dslope1);

            // at left end-point, use Matlab end-point formula with slope limits
            // note that the deriv0 value is bogus values when this is true

        } else {
            slope1 = ((2.0 + h1 + h2) * deriv1 - h1 * deriv2) / (h1 + h2);
            dslope1 = ((2.0 + h1 + h2) * dderiv1 - h1 * dderiv2) / (h1 + h2);
            end_point_derivative<DATA_TYPE>::compute(
                deriv1, deriv2, dderiv1, dderiv2, deriv_vec, slope1, dslope1);
        }

        //*************
        // compute weighted harmonic mean of slopes around index k+1
        // for both the values, and their derivatives
        // set it zero at local maxima or minima
        // deriv1 * deriv2 condition guards against division by zero

        DATA_TYPE slope2, dslope2;
        initialize<DATA_TYPE>::zero(slope2, dslope2, y1);

        // when not at an end-point, slope2 is the harmonic, weighted
        // average of deriv1 and deriv2.

        if (k <= kmax) {
            const double w1 = 2.0 * h1 + h0;
            const double w2 = h1 + 2.0 * h0;
            derivative<DATA_TYPE>::compute(deriv1, deriv2, dderiv1, dderiv2, w1,
                                           w2, deriv_vec, slope2, dslope2);

            // at right end-point, use Matlab end-point formula with slope
            // limits note that the deriv2 value is bogus values when this is
            // true

        } else {  // otherwise, compute harmonic weighted average
            slope2 = ((2.0 + h1 + h2) * deriv1 - h1 * deriv0) / (h1 + h0);
            dslope2 = ((2.0 + h1 + h2) * dderiv1 - h1 * dderiv0) / (h1 + h0);
            end_point_derivative<DATA_TYPE>::compute(
                deriv1, deriv0, dderiv1, dderiv0, deriv_vec, slope2, dslope2);
        }

        // compute interpolation value in this dimension

        result = y2 * sh_term / h1_3 + y1 * (h1_3 - sh_term) / h1_3 +
                 slope2 * s_2 * sh_minus / h1_2 +
                 slope1 * s * sh_minus * sh_minus / h1_2;

        // compute derivative in this dimension
        // assume linear change of slope across interval

        if (deriv_vec) {
            DATA_TYPE u, one_minus_u;
            double v = s / h1;
            initialize<DATA_TYPE>::value(u, slope1, v);
            initialize<DATA_TYPE>::value(one_minus_u, slope1, (1.0 - v));
            deriv = slope1 * one_minus_u + slope2 * u;
            deriv_vec[dim] = deriv;
            if (dim > 0) {
                deriv_vec[dim - 1] = dy2 * sh_term / h1_3 +
                                     dy1 * (h1_3 - sh_term) / h1_3 +
                                     dslope2 * s_2 * sh_minus / h1_2 +
                                     dslope1 * s * sh_minus * sh_minus / h1_2;
            }
        }

        // use results for dim+1 iteration

        return result;
    }

   protected:
    /// Limit construction to sub-classes.
    gen_grid<NUM_DIMS, DATA_TYPE>() {}

    /**
     * Local copy of data storage to support data editing.
     */
    std::shared_ptr<DATA_TYPE[]> _writeable_data;
};

}  // end of namespace types
}  // end of namespace usml
