/** 
 * @file data_grid.h 
 * N-dimensional data set and its associated axes. 
 */
#ifndef USML_TYPES_DATA_GRID_H
#define USML_TYPES_DATA_GRID_H

#include <usml/types/seq_vector.h>
#include <string.h>

namespace usml {
namespace types {
/// @ingroup data_grid
/// @{

/** Type of interpolation used for each axis. */
enum GRID_INTERP_TYPE
{
    GRID_INTERP_NEAREST = -1, GRID_INTERP_LINEAR = 0, // default
    GRID_INTERP_PCHIP = 1
};

/** 
 * @internal
 * Generalized recursive calculation of N-dimensional array index. 
 * Template recursion used to un-wrap loops and increase execution speed.
 * Start recursion with Dim=NUM_DIMS-1. 
 * Template specialization terminates recursion.
 * Designed to only be used internally by the data_grid class.
 *
 * @param  axis             Set of axes that define dataset.
 * @param  index            Index number in each dimension.
 */
template<unsigned Dim> inline size_t data_grid_compute_offset(
        seq_vector *axis[], const unsigned* index)
{
    return index[Dim] + axis[Dim]->size() * data_grid_compute_offset<Dim - 1> (
            axis, index);
}

/**
 * @internal
 * Final term in recursive calculation of N-dimensional array index.
 */
template<> inline size_t data_grid_compute_offset<0> (seq_vector *axis[],
        const unsigned* index)
{
    return index[0];
}

/**
 * N-dimensional data set and its associated axes.
 * Supports interpolation in any number of dimensions.
 *
 * @param  DATA_TYPE    Type of data to be interpolated. Must support +,-,*,/ 
 *                      with itself and double precision scalars.
 * @param  NUM_DIMS     Number of dimensions in this grid.  Specifying this
 *                      at compile time allows for some loop un-wrapping.
 */
template<class DATA_TYPE, unsigned NUM_DIMS> class data_grid
{

    //*************************************************************************
    // Axis and Data properties

protected:

    /** Axis associated with each dimension of the data grid. */
    seq_vector* _axis[NUM_DIMS];

    /** 
     * Multi-dimensional data stored as a linear array in column major order. 
     * This format is used to support an N-dimensional data set
     * with any any number of dimensions.
     * This memory is created in the constructor and deleted in the destructor.
     */
    DATA_TYPE *_data;

public:

    /**
     * Extract a reference to one of the axes.
     */
    inline const seq_vector* axis(int unsigned dim) const
    {
        return _axis[dim];
    }

    /** 
     * Extract a data value at a specific combination of indices.
     *
     * @param  index            Index number in each dimension.
     */
    inline DATA_TYPE data(const unsigned* index) const
    {
        const size_t offset = data_grid_compute_offset<NUM_DIMS - 1> (
                (seq_vector**) _axis, index);
        return _data[offset];
    }

    /** 
     * Define a new data value at a specific combination of indices.
     *
     * @param  index            Index number in each dimension.
     * @param  value            Value to insert at this location.
     */
    inline void data(const unsigned* index, DATA_TYPE value)
    {
        const size_t offset = data_grid_compute_offset<NUM_DIMS - 1> (_axis,
                index);
        _data[offset] = value;
    }

    //*************************************************************************
    // interp_type property

private:

    /** Defines the type of interpolation for each axis. */
    enum GRID_INTERP_TYPE _interp_type[NUM_DIMS];

public:

    /** 
     * Retrieve the type of interpolation for one of the axes.
     */
    inline enum GRID_INTERP_TYPE interp_type(int unsigned dimension) const
    {
        return _interp_type[dimension];
    }

    /**
     * Define the type of interpolation for one of the axes.
     * Modifies the axis buffer size as a side effect.
     *
     * @param  dimension        Dimension number to be modified.
     * @param  type             Type of interpolation for this dimension.
     */
    inline void interp_type(int unsigned dimension, enum GRID_INTERP_TYPE type)
    {
        _interp_type[dimension] = type;
    }

    //*************************************************************************
    // interpolation methods

private:

    /** Used during interpolation to hold the axis offsets. */
    unsigned _offset[NUM_DIMS];

    /**
     * Private recursion engine for multi-dimensional interpolation.  
     * The type of interpolation for each dimension is determined using 
     * the _interp_type[] field. Interpoaltion coefficients are computed on 
     * the fly to make arbitary combinations of interpolation types viable.
     *
     * @param   dim         Index of the dimension currently being processed.
     *                      Recursion starts at dim=NUM_DIMS-1 and reduces to
     *                      element retrieval when dim=-1.
     * @param   index       Position of the corner before the desired field
     *                      point. Must have the same rank as the data grid.
     * @param   location    Location at which field value is desired. Must
     *                      have the same rank as the data grid or higher.
     * @param   deriv	    Derivative for this iteration.
     * @param	deriv_vec   Results vector for derivative.
     *			    Derviative not computed if NULL.
     * @return              Estimate of the field after interpolation.
     */
    DATA_TYPE interp(int dim, const unsigned* index, const double* location,
            DATA_TYPE& deriv, DATA_TYPE* deriv_vec) const;
    // forward reference needed for recursion

    /**
     * Perform a nearest neighbor interpolation on this dimension.
     *
     * @param   dim         Index of the dimension currently being processed.
     * @param   index       Position of the corner before the desired field
     *                      point. Must have the same rank as the data grid.
     * @param   location    Location at which field value is desired. Must
     *                      have the same rank as the data grid or higher.
     * @param   deriv	    Derivative for this iteration. Always zero for 
     *                      nearest neighbor interpolation.
     * @param	deriv_vec   Results vector for derivative.
     *			    Derviative not computed if NULL.
     * @return              Estimate of the field after interpolation.
     */
    DATA_TYPE nearest(int dim, const unsigned* index, const double* location,
            DATA_TYPE& deriv, DATA_TYPE* deriv_vec) const
    {
        DATA_TYPE result, da;

        // compute field value in this dimension

        const unsigned k = index[dim];
        seq_vector* ax = _axis[dim];
        const double u = (location[dim] - (*ax)(k)) / ax->increment(k);
        if (u < 0.5) {
            result = interp(dim - 1, index, location, da, deriv_vec);
        } else {
            unsigned next[NUM_DIMS];
            memcpy(next, index, NUM_DIMS * sizeof(unsigned));
            ++next[dim];
            result = interp(dim - 1, next, location, da, deriv_vec);
        }

        // compute derivative in this dimension

        if (deriv_vec) {
            deriv = 0.0;
            deriv_vec[dim] = deriv;
            if (dim > 0)
                deriv_vec[dim - 1] = da;
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
     * @param   deriv	    Derivative for this iteration. Constant across the 
     *                      interval for linear interpolation.
     * @param	deriv_vec   Results vector for derivative.
     *			    Derviative not computed if NULL.
     * @return              Estimate of the field after interpolation.
     */
    DATA_TYPE linear(int dim, const unsigned* index, const double* location,
            DATA_TYPE& deriv, DATA_TYPE* deriv_vec) const
    {
        DATA_TYPE result, da, db;

        // build interpolation coefficients

        unsigned next[NUM_DIMS];
        memcpy(next, index, NUM_DIMS * sizeof(unsigned));
        ++next[dim];

        const DATA_TYPE a = interp(dim - 1, index, location, da, deriv_vec);
        const DATA_TYPE b = interp(dim - 1, next, location, db, deriv_vec);
        const unsigned k = index[dim];
        seq_vector* ax = _axis[dim];

        // compute field value in this dimension

        const double u = (location[dim] - (*ax)(k)) / ax->increment(k);
        result = a * (1.0 - u) + b * u;

        // compute derivative in this dimension and prior dimension

        if (deriv_vec) {
            deriv = (b - a) / ax->increment(k);
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
     * Interpolation Polynomial (PCHIP) algorithm. The PCHIP algorithm is
     * designed to preserve the shape of the underlying data and avoid 
     * the overshooting issues prevalent in the cubic spline.  Although the
     * the first derivative of the PCHIP result is guaranteed to be continuous,
     * the second derivative has no such guarantee.  This algorithm 
     * is taken from Chapter 3 of the book <b>Numerical Computing in Matlab</b> 
     * by Cleve Moler, available on-line at http://www.mathworks.com/molar.
     * <pre>
     *
     *      p(x) = y[k+1]  * ( 3 h s^2 - 2 s^3 ) / h^3
     *             y[k]    * ( h^3 - 3 h s^2 - 2 s^3 ) / h^3
     *             y'[k+1] * ( s^2 (s-h) ) / h^2
     *             y'[k]   * ( s (s-h)^2 ) / h^2
     *
     * where:
     *
     *      h = x[k+1] - x[k] 
     *      s = x - x[k]
     *      y'[k] = average slope of interpolant
     *
     * </pre>
     * @param   dim         Index of the dimension currently being processed.
     * @param   index       Position of the corner before the desired field
     *                      point. Must have the same rank as the data grid.
     * @param   location    Location at which field value is desired. Must
     *                      have the same rank as the data grid or higher.
     * @param   deriv	    Derivative for this iteration. Constant across the 
     *                      interval for linear interpolation.
     * @param	deriv_vec   Results vector for derivative.
     *			    Derviative not computed if NULL.
     * @return              Estimate of the field after interpolation.
     */
    DATA_TYPE pchip(int dim, const unsigned* index, const double* location,
            DATA_TYPE& deriv, DATA_TYPE* deriv_vec) const
    {
        DATA_TYPE result, dy0, dy1, dy2, dy3;
        DATA_TYPE dderiv0 = 0.0, dderiv1 = 0.0, dderiv2 = 0.0;

        // build function values at k, k-1, k+1, k+2

        const unsigned kmin = 1u;
        const unsigned kmax = _axis[dim]->size() - 3u;

        const unsigned k = index[dim];
        seq_vector* ax = _axis[dim];
        const DATA_TYPE y1 = interp(dim - 1, index, location, dy1, deriv_vec);

        unsigned prev[NUM_DIMS];
        memcpy(prev, index, NUM_DIMS * sizeof(unsigned));
        --prev[dim];
        const DATA_TYPE y0 = (k < kmin) ? y1 : interp(dim - 1, prev, location,
                dy0, deriv_vec);

        unsigned next[NUM_DIMS];
        memcpy(next, index, NUM_DIMS * sizeof(unsigned));
        ++next[dim];
        const DATA_TYPE y2 = interp(dim - 1, next, location, dy2, deriv_vec);

        unsigned last[NUM_DIMS];
        memcpy(last, next, NUM_DIMS * sizeof(unsigned));
        ++last[dim];
        const DATA_TYPE y3 = (k > kmax) ? y2 : interp(dim - 1, last, location,
                dy3, deriv_vec);

        // compute difference values used frequently in computation

        const double h1 = ax->increment(k);
        const double h1_2 = h1 * h1, h1_3 = h1_2 * h1;
        const double h0 = (k < kmin) ? 0.0 : ax->increment(k - 1);
        const double h2 = (k > kmax) ? 0.0 : ax->increment(k + 1);

        const double s = location[dim] - (*ax)(k);
        const double s_2 = s * s, s_3 = s_2 * s;
        const double sh_minus = s - h1;
        const double sh_term = 3.0 * h1 * s_2 - 2.0 * s_3;

        // compute piecewise first derivatives 

        const DATA_TYPE deriv1 = (y2 - y1) / h1;
        const DATA_TYPE deriv0 = (k < kmin) ? deriv1 : (y1 - y0) / h0;
        const DATA_TYPE deriv2 = (k > kmax) ? deriv1 : (y3 - y2) / h2;
        if (deriv_vec) {
            if (k < kmin)
                dy0 = dy1;
            if (k > kmax)
                dy3 = dy2;
            dderiv1 = (dy2 - dy1) / h1;
            dderiv0 = (k < kmin) ? dderiv1 : (dy1 - dy0) / h0;
            dderiv2 = (k > kmax) ? dderiv1 : (dy3 - dy2) / h2;
        }

        // compute weighted harmonic mean of slope
        // assume x axis unevenly spaced
        // reuse end-point algorithm from pchipend.m w/o derviative limit

        DATA_TYPE slope1, dslope1 = 0.0;
        if (k < kmin) {
            slope1 = ((2.0 + h1 + h2) * deriv1 - h1 * deriv2) / (h1 + h2);
            if (slope1 * deriv1 < 0.0) {
                slope1 *= 0.0;
            }
            if (deriv_vec) {
                dslope1 = ((2.0 + h1 + h2) * dderiv1 - h1 * dderiv2)
                        / (h1 + h2);
                if (dslope1 * dderiv1 < 0.0) {
                    dslope1 *= 0.0;
                }
            }
        } else {
            const double w1 = 2.0 * h1 + h0;
            const double w2 = h1 + 2.0 * h0;
            slope1 = (deriv0 * deriv1 <= 0.0) ? (deriv1 * 0.0) : (w1 + w2)
                    / (w1 / deriv0 + w2 / deriv1);
            if (deriv_vec) {
                dslope1 = (dderiv0 * dderiv1 <= 0.0) ? (dderiv1 * 0.0) : (w1
                        + w2) / (w1 / dderiv0 + w2 / dderiv1);
            }
        }

        DATA_TYPE slope2, dslope2 = 0.0;
        if (k > kmax) {
            slope2 = ((2.0 + h1 + h0) * deriv1 - h1 * deriv0) / (h1 + h0);
            if (slope2 * deriv0 < 0.0) {
                slope2 *= 0.0;
            }
            if (deriv_vec) {
                dslope2 = ((2.0 + h1 + h0) * dderiv1 - h1 * dderiv0)
                        / (h1 + h0);
                if (dslope2 * dderiv0 < 0.0) {
                    dslope2 *= 0.0;
                }
            }
        } else {
            const double w1 = 2.0 * h2 + h1;
            const double w2 = h2 + 2.0 * h1;
            slope2 = (deriv1 * deriv2 <= 0.0) ? (deriv2 * 0.0) : (w1 + w2)
                    / (w1 / deriv1 + w2 / deriv2);
            if (deriv_vec) {
                dslope2 = (dderiv1 * dderiv2 <= 0.0) ? (dderiv2 * 0.0) : (w1
                        + w2) / (w1 / dderiv1 + w2 / dderiv2);
            }
        }

        // compute interpolation value in this dimension

        result = y2 * sh_term / h1_3 + y1 * (h1_3 - sh_term) / h1_3 + slope2
                * s_2 * sh_minus / h1_2 + slope1 * s * sh_minus * sh_minus
                / h1_2;

        // compute derivative in this dimension
        // assume linear change of slope across interval

        if (deriv_vec) {
            const double u = s / h1;
            deriv = slope1 * (1.0 - u) + slope2 * u;
            deriv_vec[dim] = deriv;
            if (dim > 0) {
                deriv_vec[dim - 1] = dy2 * sh_term / h1_3 + dy1 * (h1_3
                        - sh_term) / h1_3 + dslope2 * s_2 * sh_minus / h1_2
                        + dslope1 * s * sh_minus * sh_minus / h1_2;
            }
        }

        // use results for dim+1 iteration

        return result;
    }

public:

    /**
     * Multi-dimensional interpolation with the derivative calculation.
     * So many calculations are shared between the determination of an 
     * interpolate value and its derivative, that it is computationally 
     * efficient to compute them both at the same time.
     *
     * @param   location    Location at which field value is desired. Must
     *                      have the same rank as the data grid or higher.
     * @param   derivative  If this is not null, the first derivative 
     *                      of the field at this point will also be computed.
     * @return              Value of the field at this point.
     */
    DATA_TYPE interpolate(const double* location, DATA_TYPE* derivative = NULL)
    {
        DATA_TYPE dresult;
        for (unsigned dim = 0; dim < NUM_DIMS; ++dim) {
            _offset[dim] = _axis[dim]->find_index(location[dim]);
        }
        return interp(NUM_DIMS - 1, _offset, location, dresult, derivative);
    }

    /**
     * Interpolation 1-D specialization where the arguments, and results,
     * are matrix<double>.  This is used frequently in the WaveQ3D model
     * to interpolate environmental parameters.
     *
     * @param   x           First dimension of location.
     * @param   result      Interpolated values at each location (output).
     * @param   dx          First dimension of derviative (output).
     */
    void interpolate(const matrix<double>& x, matrix<double>* result, matrix<
            double>* dx = NULL)
    {
        double location[1];
        DATA_TYPE derivative[1];
        for (unsigned n = 0; n < x.size1(); ++n) {
            for (unsigned m = 0; m < x.size2(); ++m) {
                location[0] = x(n, m);
                if (dx == NULL) {
                    (*result)(n, m) = (double) interpolate(location);
                } else {
                    (*result)(n, m)
                            = (double) interpolate(location, derivative);
                    (*dx)(n, m) = (double) derivative[0];
                }
            }
        }
    }

    /**
     * Interpolation 2-D specialization where the arguments, and results,
     * are matrix<double>.  This is used frequently in the WaveQ3D model
     * to interpolate environmental parameters.
     *
     * @param   x           First dimension of location.
     * @param   y           Second dimension of location.
     * @param   result      Interpolated values at each location (output).
     * @param   dx          First dimension of derviative (output).
     * @param   dy          Second dimension of derviative (output).
     */
    void interpolate(const matrix<double>& x, const matrix<double>& y, matrix<
            double>* result, matrix<double>* dx = NULL, matrix<double>* dy =
            NULL)
    {
        double location[2];
        DATA_TYPE derivative[2];
        for (unsigned n = 0; n < x.size1(); ++n) {
            for (unsigned m = 0; m < x.size2(); ++m) {
                location[0] = x(n, m);
                location[1] = y(n, m);
                if (dx == NULL || dy == NULL) {
                    (*result)(n, m) = (double) interpolate(location);
                } else {
                    (*result)(n, m)
                            = (double) interpolate(location, derivative);
                    (*dx)(n, m) = (double) derivative[0];
                    (*dy)(n, m) = (double) derivative[1];
                }
            }
        }
    }

    /**
     * Interpolation 3-D specialization where the arguments, and results,
     * are matrix<double>.  This is used frequently in the WaveQ3D model
     * to interpolate environmental parameters.
     *
     * @param   x           First dimension of location.
     * @param   y           Second dimension of location.
     * @param   z           Third dimension of location.
     * @param   result      Interpolated values at each location (output).
     * @param   dx          First dimension of derviative (output).
     * @param   dy          Second dimension of derviative (output).
     * @param   dz          Third dimension of derviative (output).
     */
    void interpolate(const matrix<double>& x, const matrix<double>& y,
            const matrix<double>& z, matrix<double>* result,
            matrix<double>* dx = NULL, matrix<double>* dy = NULL,
            matrix<double>* dz = NULL)
    {
        double location[3];
        DATA_TYPE derivative[3];
        for (unsigned n = 0; n < x.size1(); ++n) {
            for (unsigned m = 0; m < x.size2(); ++m) {
                location[0] = x(n, m);
                location[1] = y(n, m);
                location[2] = z(n, m);
                if (dx == NULL || dy == NULL || dz == NULL) {
                    (*result)(n, m) = (double) interpolate(location);
                } else {
                    (*result)(n, m)
                            = (double) interpolate(location, derivative);
                    (*dx)(n, m) = (double) derivative[0];
                    (*dy)(n, m) = (double) derivative[1];
                    (*dz)(n, m) = (double) derivative[2];
                }
            }
        }
    }

    //*************************************************************************
    // initialization

protected:

    /**
     * Default constructor for sub-classes
     */
    data_grid()
    {
        memset(_interp_type, GRID_INTERP_LINEAR, NUM_DIMS
                * sizeof(enum GRID_INTERP_TYPE));
    }

public:

    /**
     * Create data grid from its associated axes.
     * Allocates new memory for the data at each grid point.
     * Initialize all of the interpolation types to GRID_INTERP_LINEAR.
     *
     * @param axis  Axes to use for each dimension of the grid.
     *              The seq_vector::clone() routine is used to make a 
     *              local copy of each axis within the data grid.
     */
    data_grid(seq_vector *axis[])
    {
        size_t N = 1;
        for (unsigned n = 0; n < NUM_DIMS; ++n) {
            _axis[n] = axis[n]->clone();
            N *= _axis[n]->size();
        }
        _data = new DATA_TYPE[N];
        memset(_data, 0, N * sizeof(DATA_TYPE));
        memset(_interp_type, GRID_INTERP_LINEAR, NUM_DIMS
                * sizeof(enum GRID_INTERP_TYPE));
    }

    /**
     * Create data grid from an existing grid.
     * Allocates new memory for the data at each grid point.
     * Copies interpolation types from the original grid.
     *
     * @param other         Grid to be copied.
     * @param copy_data     Copy both axes and data if true.
     *                      Copy just axes and structure if false.
     */
    data_grid(const data_grid& other, bool copy_data)
    {
        size_t N = 1;
        for (unsigned n = 0; n < NUM_DIMS; ++n) {
            _axis[n] = other._axis[n]->clone();
            N *= _axis[n]->size();
        }
        _data = new DATA_TYPE[N];
        if (copy_data) {
            memcpy(_data, other._data, N * sizeof(DATA_TYPE));
        } else {
            memset(_data, 0, N * sizeof(DATA_TYPE));
        }
        memcpy(_interp_type, other._interp_type, NUM_DIMS
                * sizeof(enum GRID_INTERP_TYPE));
    }

    /**
     * Destroys memory area for field data.
     */
    ~data_grid()
    {
        for (unsigned n = 0; n < NUM_DIMS; ++n)
            delete _axis[n];
        delete[] _data;
    }
};

//*************************************************************************
// implementation of forward methods

/**
 * Recursion engine for multi-dimensional interpolation.
 */
template<class DATA_TYPE, unsigned NUM_DIMS>
DATA_TYPE data_grid<DATA_TYPE, NUM_DIMS>::interp(int dim,
        const unsigned* index, const double* location, DATA_TYPE& deriv,
        DATA_TYPE* deriv_vec) const
{
    DATA_TYPE result;

    if (dim < 0) {
        result = data(index);
        // terminates recursion

    } else {
        if (_interp_type[dim] == GRID_INTERP_LINEAR) {
            result = linear(dim, index, location, deriv, deriv_vec);
        } else if (_interp_type[dim] == GRID_INTERP_PCHIP) {
            result = pchip(dim, index, location, deriv, deriv_vec);
        } else {
            result = nearest(dim, index, location, deriv, deriv_vec);
        }
    }
    return result;
}

} // end of namespace types
} // end of namespace usml

#endif
