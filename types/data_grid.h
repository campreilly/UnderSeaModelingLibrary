/**
 * @file data_grid.h
 * N-dimensional data set and its associated axes.
 */
#pragma once

#include <string.h>
#include <typeinfo>
#include <sstream>
#include <netcdfcpp.h>
#include <usml/types/wposition.h>
#include <usml/types/wvector.h>
#include <usml/types/seq_vector.h>

using namespace usml::ublas;

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
    template<size_t Dim> inline size_t data_grid_compute_offset(
            seq_vector *axis[], const size_t* index)
    {
        return index[Dim] + axis[Dim]->size() * data_grid_compute_offset<Dim - 1> (
                axis, index);
    }

    /**
     * @internal
     * Final term in recursive calculation of N-dimensional array index.
     */
    template<> inline size_t data_grid_compute_offset<0> (seq_vector *axis[],
            const size_t* index)
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
template<class DATA_TYPE, size_t NUM_DIMS>
class USML_DLLEXPORT data_grid {

    typedef data_grid<DATA_TYPE,NUM_DIMS>       self_type ;
    typedef size_t                              size_type ;

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
        	size_type N = 1 ;
            for (unsigned n = 0; n < NUM_DIMS; ++n) {
                _axis[n] = axis[n]->clone();
                N *= _axis[n]->size();
                interp_type( n, GRID_INTERP_LINEAR ) ;
            }
            _data = new DATA_TYPE[N];
            memset(_data, 0, N * sizeof(DATA_TYPE));
            memset(_edge_limit, true, NUM_DIMS * sizeof(bool));
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
        	size_type N = 1 ;
            for (unsigned n = 0; n < NUM_DIMS; ++n) {
                _axis[n] = other._axis[n]->clone() ;
                N *= _axis[n]->size();
            }
            _data = new DATA_TYPE[N] ;
            if (copy_data) {
                memcpy(_data, other._data, N * sizeof(DATA_TYPE)) ;
            } else {
                memset(_data, 0, N * sizeof(DATA_TYPE)) ;
            }
            memcpy(_interp_type, other._interp_type, NUM_DIMS
                    * sizeof(enum GRID_INTERP_TYPE)) ;
            memset(_edge_limit, true, NUM_DIMS * sizeof(bool)) ;
        }

        /**
         * Copy operator
         * The data and axes are copied from rhs to this.
         */
        void copy(const self_type& rhs) {
        	size_type N = 1 ;
            for(size_type n = 0; n < NUM_DIMS; ++n) {
                if(_axis[n] != NULL) {
                    delete _axis[n] ;
                }
                _axis[n] = rhs._axis[n]->clone() ;
                N *= _axis[n]->size();
            }
            if(_data != NULL) {
                delete[] _data ;
            }
            _data = new DATA_TYPE[N] ;
        	memcpy( _data, rhs._data, N * sizeof(DATA_TYPE) ) ;
        }

        /**
         * Destroys memory area for field data.
         */
        virtual ~data_grid()
        {
            for (size_t n = 0; n < NUM_DIMS; ++n) {
                if (_axis[n] != NULL) {
                    delete _axis[n];
                }
            }
            if (_data != NULL) {
                delete[] _data;
            }
        }

        /**
         * Extract a constant reference to one of the axes.
         */
        inline const seq_vector* axis(size_t dim) const
        {
            return _axis[dim];
        }

        /**
         * Extract a reference to one of the axes.
         */
        inline seq_vector* axis(size_t dim)
        {
            return _axis[dim];
        }

        /**
         * Extract a data value at a specific combination of indices.
         *
         * @param  index            Index number in each dimension.
         */
        inline DATA_TYPE data(const size_t* index) const
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
        inline void data(const size_t* index, DATA_TYPE value)
        {
            const size_t offset = data_grid_compute_offset<NUM_DIMS - 1> (_axis,
                    index);
            _data[offset] = value;
        }

        /**
         * Output data_grid to netcdf file.
         * @param filename      name of the netcdf file to output to
         */
        void write_netcdf( const char* filename ) {

            NcFile* _file = new NcFile( filename, NcFile::Replace ) ;

            vector<const NcDim*> axis_dim(NUM_DIMS) ;
            vector<NcVar*> axis_var(NUM_DIMS) ;
            const NcDim** axis_size = new const NcDim*[NUM_DIMS] ;
            long* data_size = new long[NUM_DIMS] ; 
				// Note: Using size_type instead of long for this variable
			    // doesn't work on VC++ 32 bit, data_var->put() requires long.
            NcType type ;
            const std::type_info& dt = typeid(DATA_TYPE) ;
            const char* dtype = dt.name() ;
            if( dtype == typeid(int).name() ) {
                type = ncInt ;
            }else if( dtype == typeid(float).name() ) {
                type = ncFloat ;
            }else if( dtype == typeid(double).name() ) {
                type = ncDouble ;
            }

            NcVar* earth_radius = _file->add_var( "earth_radius", ncDouble, 0 ) ;
            for(long i=0; i < NUM_DIMS; ++i) {
                std::stringstream ss ;
                ss << "axis" << i << "\0" ;
                axis_dim[i] = _file->add_dim( (ss.str()).c_str(), _axis[i]->size() ) ;
                axis_size[i] = axis_dim[i] ;
                axis_var[i] = _file->add_var( (ss.str()).c_str(), type, axis_dim[i] ) ;
                data_size[i] = _axis[i]->size() ;
            }
            NcVar* data_var = _file->add_var( "data", type, NUM_DIMS, &*axis_size ) ;

            earth_radius->put( &wposition::earth_radius, 1 ) ;
            for(long i=0; i < NUM_DIMS; ++i) {
            	axis_var[i]->put( boost::numeric::ublas::vector<double>(*_axis[i]).data().begin(),
					data_size[i] ) ;
            }
            data_var->put( _data, data_size ) ;

            // clean up after file completion
            delete _file ;
            delete[] axis_size ;
            delete[] data_size ;
            _file = NULL ;
            data_size = NULL ;
        }

        /**
         * Retrieve the type of interpolation for one of the axes.
         */
        inline enum GRID_INTERP_TYPE interp_type(size_t dimension) const
        {
            return _interp_type[dimension];
        }

        /**
         * Define the type of interpolation for one of the axes.
         * Modifies the axis buffer size as a side effect.
         * Note that linear interpolation requires a minimum of 2 points;
         * pchip requires a minimum of 4 points.
         *
         * @param  dimension        Dimension number to be modified.
         * @param  type             Type of interpolation for this dimension.
         */
        inline void interp_type(size_t dimension, enum GRID_INTERP_TYPE type)
        {
            const size_t size = _axis[dimension]->size() ;
            if ( type > GRID_INTERP_NEAREST && size < 2 ) {
                type = GRID_INTERP_NEAREST ;
            } else if ( type > GRID_INTERP_LINEAR && size < 4 ) {
                type = GRID_INTERP_LINEAR ;
            } else {
                _interp_type[dimension] = type;
            }
        }

        /**
         * Returns the edge_limit flag for the requested dimension.
         */
        inline bool edge_limit(size_t dimension) const
        {
            return _edge_limit[dimension];
        }

        /**
         * Set the edge_limit flag for the requested dimension.
         * Note: Default is true
         *
         * @param  dimension        Dimension number to be modified.
         * @param  flag             Limits locations when true.
         */
        inline void edge_limit(size_t dimension, bool flag)
        {
            _edge_limit[dimension] = flag;
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
         * @param   derivative  If this is not null, the first derivative
         *                      of the field at this point will also be computed.
         * @return              Value of the field at this point.
         */
        DATA_TYPE interpolate(double* location, DATA_TYPE* derivative = NULL)
        {
            // find the "interval index" in each dimension

            for (size_t dim = 0; dim < NUM_DIMS; ++dim) {

                // limit interpolation to axis domain if _edge_limit turned on

                if ( _edge_limit[dim] ) {
                    double a = *(_axis[dim]->begin()) ;
                    double b = *(_axis[dim]->rbegin()) ;
                    double inc = _axis[dim]->increment(0);
                    if ( inc < 0) {                                                     // a > b
                        if ( location[dim] >= a ) {                                     //left of the axis
                            location[dim] = a ;
                            _offset[dim] = 0 ;
                        } else if ( location[dim] <= b ) {                              //right of the axis
                            location[dim] = b ;
                            _offset[dim] = _axis[dim]->size()-2 ;
                        } else {
                            _offset[dim] = _axis[dim]->find_index(location[dim]);       //somewhere in-between the endpoints of the axis
                        }
                    }
                    if (inc > 0 ) {                                                     // a < b
                        if ( location[dim] <= a ) {                                     //left of the axis
                            location[dim] = a ;
                            _offset[dim] = 0 ;
                        } else if ( location[dim] >= b ) {                              //right of the axis
                            location[dim] = b ;
                            _offset[dim] = _axis[dim]->size()-2 ;
                        } else {
                            _offset[dim] = _axis[dim]->find_index(location[dim]);       //somewhere in-between the endpoints of the axis
                        }
                    }

                // allow extrapolation if _edge_limit turned off

                } else {
                    _offset[dim] = _axis[dim]->find_index(location[dim]);
                }
            }

            // compute interpolation results for value and derivative

            DATA_TYPE dresult;
            return interp(NUM_DIMS - 1, _offset, location, dresult, derivative);
        }

        /**
         * Interpolation 1-D specialization where the arguments, and results,
         * are matrix<double>.  This is used frequently in the WaveQ3D model
         * to interpolate environmental parameters.
         *
         * @param   x           First dimension of location.
         * @param   result      Interpolated values at each location (output).
         * @param   dx          First dimension of derivative (output).
         */
        void interpolate(const matrix<double>& x, matrix<double>* result, matrix<
                double>* dx = NULL)
        {
            double location[1];
            DATA_TYPE derivative[1];
            for (size_t n = 0; n < x.size1(); ++n) {
                for (size_t m = 0; m < x.size2(); ++m) {
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
         * @param   dx          First dimension of derivative (output).
         * @param   dy          Second dimension of derivative (output).
         */
        void interpolate(const matrix<double>& x, const matrix<double>& y, matrix<
                double>* result, matrix<double>* dx = NULL, matrix<double>* dy =
                NULL)
        {
            double location[2];
            DATA_TYPE derivative[2];
            for (size_t n = 0; n < x.size1(); ++n) {
                for (size_t m = 0; m < x.size2(); ++m) {
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
            DATA_TYPE derivative[3];
            for (size_t n = 0; n < x.size1(); ++n) {
                for (size_t m = 0; m < x.size2(); ++m) {
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

    protected:

        /** Axis associated with each dimension of the data grid. */
        seq_vector* _axis[NUM_DIMS] ;

        /** Defines the type of interpolation for each axis. */
        enum GRID_INTERP_TYPE _interp_type[NUM_DIMS];

        /** Limits locations to values inside axis when true. */
        bool _edge_limit[NUM_DIMS];

        /** Used during interpolation to hold the axis offsets. */
        size_t _offset[NUM_DIMS] ;

        /**
         * Multi-dimensional data stored as a linear array in column major order.
         * This format is used to support an N-dimensional data set
         * with any number of dimensions.
         * This memory is created in the constructor and deleted in the destructor.
         */
        DATA_TYPE* _data ;

        /**
         * Default constructor for sub-classes
         */
        data_grid() {

            for (unsigned n = 0; n < NUM_DIMS; ++n) {
                _axis[n] = NULL ;
            }
            _data = NULL ;

            memset(_interp_type, GRID_INTERP_LINEAR, NUM_DIMS * sizeof(enum GRID_INTERP_TYPE)) ;
            memset(_edge_limit, true, NUM_DIMS * sizeof(bool)) ;
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
         * @param   deriv	    Derivative for this iteration.
         * @param	deriv_vec   Results vector for derivative.
         *			            Derivative not computed if NULL.
         * @return              Estimate of the field after interpolation.
         */
        DATA_TYPE interp(int dim, const size_t* index, const double* location,
                DATA_TYPE& deriv, DATA_TYPE* deriv_vec) const;
        // forward reference needed for recursion

        /**
         * Perform a nearest neighbor interpolation on this dimension.
         *
         * @param   dim         Index of the dimension currently being processed.
         * @param   index       Position of the corner before the desired field
         *                      point. Must have the same NUM_DIM as the data grid.
         * @param   location    Location at which field value is desired. Must
         *                      have the same NUM_DIM as the data grid or higher.
         * @param   deriv	    Derivative for this iteration. Always zero for
         *                      nearest neighbor interpolation.
         * @param	deriv_vec   Results vector for derivative.
         *			            Derivative not computed if NULL.
         * @return              Estimate of the field after interpolation.
         */
        DATA_TYPE nearest(int dim, const size_t* index, const double* location,
                DATA_TYPE& deriv, DATA_TYPE* deriv_vec) const
        {
            DATA_TYPE result, da;

            // compute field value in this dimension

            const size_t k = index[dim];
            seq_vector* ax = _axis[dim];
            const double u = (location[dim] - (*ax)(k)) / ax->increment(k);
            if (u < 0.5) {
                result = interp(dim - 1, index, location, da, deriv_vec);
            } else {
                size_t next[NUM_DIMS];
                memcpy(next, index, NUM_DIMS * sizeof(size_t));
                ++next[dim];
                result = interp(dim - 1, next, location, da, deriv_vec);
            }

            // compute derivative in this dimension

            if (deriv_vec) {
                deriv = 0.0;
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
         * @param   deriv	    Derivative for this iteration. Constant across the
         *                      interval for linear interpolation.
         * @param	deriv_vec   Results vector for derivative.
         *			    		Derivative not computed if NULL.
         * @return              Estimate of the field after interpolation.
         */
        DATA_TYPE linear(int dim, const size_t* index, const double* location,
                DATA_TYPE& deriv, DATA_TYPE* deriv_vec) const
        {
            DATA_TYPE result, da, db;

            // build interpolation coefficients

			size_t* next = new size_t[NUM_DIMS];	// dynamic creation works around Win64 problem
            memcpy(next, index, NUM_DIMS * sizeof(size_t));
            ++next[dim];

            const DATA_TYPE a = interp(dim - 1, index, location, da, deriv_vec);
            const DATA_TYPE b = interp(dim - 1, next, location, db, deriv_vec);
            const size_t k = index[dim];
            seq_vector* ax = _axis[dim];
			delete[] next;

            // compute field value in this dimension

            const DATA_TYPE h = (DATA_TYPE) ax->increment(k) ;
            const DATA_TYPE u = (location[dim] - (*ax)(k)) / h ;
            result = a * (1.0 - u) + b * u;

            // compute derivative in this dimension and prior dimension

            if (deriv_vec) {
                deriv = (b - a) / h ;
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
         * When using a gridded data set, it is recommended that edge_limit be set to
         * TRUE for any dimensional axis that uses the PCHIP interpolation. This is
         * because of PCHIP allowing for extreme values when extrapolating data.
         *
         * @xref Cleve Moler, Numerical Computing in Matlab, Chapter 3 Interpolation,
         * http://www.mathworks.com/moler/chapters.html accessed 5/15/2012.
         * @xref F. N. Fritsch and R. E. Carlson, Monotone Piecewise Cubic Interpolation,
         * SIAM Journal on Numerical Analysis, 17 (1980), pp. 238-246.
         * @xref D. Kahaner, C. Moler, and S. Nash, Numerical Methods and Software,
         * Prentice{Hall, Englewood Cli®s, NJ, 1989.
         *
         * The basic algorithm assumes that the interpolation location is
         * in the interval [ x[k], x[k+1] ), where "k" is known as the
         * "interval index".  The result is then calculated from four unevenly
         * spaced points, and their forward (one-sided) derivatives.
         *
         * <pre>
         * 		y0 = y[k-1]		h0 = x[k]-x[k-1]	deriv0 = (y1-y0)/h0
         * 		y1 = y[k]		h1 = x[k+1]-x[k]	deriv1 = (y2-y1)/h1
         * 		y2 = y[k+1]		h2 = x[k+2]-x[k+1]	deriv2 = (y3-y2)/h2
         * 		y3 = y[k+2]		s = x - x[k]
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
         *		slope[k] = weighted harmonic average of deriv0, deriv1, deriv2 terms
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
         * @param   deriv	    Derivative for this iteration. Constant across the
         *                      interval for linear interpolation.
         * @param	deriv_vec   Results vector for derivative.
         *			            Derivative not computed if NULL.
         * @return              Estimate of the field after interpolation.
         */
        DATA_TYPE pchip(int dim, const size_t* index, const double* location,
                DATA_TYPE& deriv, DATA_TYPE* deriv_vec) const
        {
            DATA_TYPE result ;
            DATA_TYPE y0, y1, y2, y3 ; 			// dim-1 values at k-1, k, k+1, k+2
            DATA_TYPE dy0=0, dy1=0, dy2=0, dy3=0 ;		// dim-1 derivs at k-1, k, k+1, k+2
            const size_t kmin = 1u ;					  // at endpt if k-1 < 0
            const size_t kmax = _axis[dim]->size()-3u ; // at endpt if k+2 > N-1

            // interpolate in dim-1 dimension to find values and derivs at k, k-1

            const size_t k = index[dim];
            seq_vector* ax = _axis[dim];
            y1 = interp( dim-1, index, location, dy1, deriv_vec );

            if ( k >= kmin ) {
                size_t prev[NUM_DIMS];
                memcpy(prev, index, NUM_DIMS * sizeof(size_t));
                --prev[dim];
                y0 = interp( dim-1, prev, location, dy0, deriv_vec );
            } else {	// use harmless values at left end-point
                y0 = y1 ;
                dy0 = dy1 ;
            }

            // interpolate in dim-1 dimension to find values and derivs at k+1, k+2

            size_t next[NUM_DIMS];
            memcpy(next, index, NUM_DIMS * sizeof(size_t));
            ++next[dim];
            y2 = interp( dim-1, next, location, dy2, deriv_vec );

            if ( k <= kmax ) {
                size_t last[NUM_DIMS];
                memcpy(last, next, NUM_DIMS * sizeof(size_t));
                ++last[dim];
                y3 = interp(dim - 1, last, location, dy3, deriv_vec);
            } else {	// use harmless values at right end-point
                y3 = y2 ;
                dy3 = dy2 ;
            }

            // compute difference values used frequently in computation

            const DATA_TYPE h0 = (DATA_TYPE) ax->increment(k - 1); 	// interval from k-1 to k
            const DATA_TYPE h1 = (DATA_TYPE) ax->increment(k);	   	// interval from k to k+1
            const DATA_TYPE h2 = (DATA_TYPE) ax->increment(k + 1); 	// interval from k+1 to k+2
            const DATA_TYPE h1_2 = h1 * h1; 		   	// k to k+1 interval squared
            const DATA_TYPE h1_3 = h1_2 * h1;		   	// k to k+1 interval cubed

            const DATA_TYPE s = location[dim]-(*ax)(k);	// local variable
            const DATA_TYPE s_2 = s * s, s_3 = s_2 * s;	// s squared and cubed
            const DATA_TYPE sh_minus = s - h1;
            const DATA_TYPE sh_term = 3.0 * h1 * s_2 - 2.0 * s_3;

            // compute first divided differences (forward derivative)
            // for both the values, and their derivatives

            const DATA_TYPE deriv0 = (y1 - y0) / h0;	// fwd deriv from k-1 to k
            const DATA_TYPE deriv1 = (y2 - y1) / h1; 	// fwd deriv from k to k+1
            const DATA_TYPE deriv2 = (y3 - y2) / h2; 	// fwd deriv from k+1 to k+2

            DATA_TYPE dderiv0=0.0, dderiv1=0.0, dderiv2=0.0 ;
            if (deriv_vec) {				// fwd deriv of dim-1 derivatives
                dderiv0 = (dy1 - dy0) / h0;
                dderiv1 = (dy2 - dy1) / h1;
                dderiv2 = (dy3 - dy2) / h2;
            }

            //*************
            // compute weighted harmonic mean of slopes around index k
            // for both the values, and their derivatives
            // set it zero at local maxima or minima
            // deriv0 * deriv1 condition guards against division by zero

            DATA_TYPE slope1=0.0, dslope1=0.0;

            // when not at an end-point, slope1 is the harmonic, weighted
            // average of deriv0 and deriv1.

            if ( k >= kmin ) {
                const DATA_TYPE w0 = 2.0 * h1 + h0;
                const DATA_TYPE w1 = h1 + 2.0 * h0;
                if ( deriv0 * deriv1 > 0.0 ) {
                    slope1 = (w0 + w1) / ( w0 / deriv0 + w1 / deriv1 );
                }
                if ( deriv_vec != NULL && dderiv0 * dderiv1 > 0.0 ) {
                    dslope1 = (w0 + w1) / ( w0 / dderiv0 + w1 / dderiv1 );
                }

            // at left end-point, use Matlab end-point formula with slope limits
            // note that the deriv0 value is bogus values when this is true

            } else {
                slope1 = ( (2.0+h1+h2) * deriv1 - h1 * deriv2 ) / (h1+h2) ;
                if ( slope1 * deriv1 < 0.0 ) {
                    slope1 = 0.0 ;
                } else if ( (deriv1*deriv2 < 0.0) && (abs(slope1) > abs(3.0*deriv1)) ) {
                    slope1 = 3.0*deriv1 ;
                }
                if ( deriv_vec ) {
                    dslope1 = ( (2.0+h1+h2) * dderiv1 - h1 * dderiv2 ) / (h1+h2) ;
                    if ( dslope1 * dderiv1 < 0.0 ) {
                        dslope1 = 0.0 ;
                    } else if ( (dderiv1*dderiv2 < 0.0) && (abs(dslope1) > abs(3.0*dderiv1)) ) {
                        dslope1 = 3.0*dderiv1 ;
                    }
                }
            }

            //*************
            // compute weighted harmonic mean of slopes around index k+1
            // for both the values, and their derivatives
            // set it zero at local maxima or minima
            // deriv1 * deriv2 condition guards against division by zero

            DATA_TYPE slope2=0.0, dslope2=0.0;

            // when not at an end-point, slope2 is the harmonic, weighted
            // average of deriv1 and deriv2.

            if ( k <= kmax ) {
                const DATA_TYPE w1 = 2.0 * h1 + h0;
                const DATA_TYPE w2 = h1 + 2.0 * h0;
                if ( deriv1 * deriv2 > 0.0 ) {
                    slope2 = (w1 + w2) / ( w1 / deriv1 + w2 / deriv2 );
                }
                if ( deriv_vec != NULL && dderiv1 * dderiv2 > 0.0 ) {
                    dslope2 = (w1 + w2) / ( w1 / dderiv1 + w2 / dderiv2 );
                }

            // at right end-point, use Matlab end-point formula with slope limits
            // note that the deriv2 value is bogus values when this is true

            } else {		// otherwise, compute harmonic weighted average
                slope2 = ( (2.0+h1+h2) * deriv1 - h1 * deriv0 ) / (h1+h0) ;
                if ( slope2 * deriv1 < 0.0 ) {
                    slope2 = 0.0 ;
                } else if ( (deriv1*deriv0 < 0.0) && (abs(slope2) > abs(3.0*deriv1)) ) {
                    slope2 = 3.0*deriv1 ;
                }
                if ( deriv_vec ) {
                    dslope2 = ( (2.0+h1+h2) * dderiv1 - h1 * dderiv0 ) / (h1+h0) ;
                    if ( dslope2 * dderiv1 < 0.0 ) {
                        dslope2 = 0.0 ;
                    } else if ( (dderiv1*dderiv0 < 0.0) && (abs(dslope2) > abs(3.0*dderiv1)) ) {
                        dslope2 = 3.0*dderiv1 ;
                    }
                }
            }

            // compute interpolation value in this dimension

            result = y2 * sh_term / h1_3
                   + y1 * (h1_3 - sh_term) / h1_3
                   + slope2 * s_2 * sh_minus / h1_2
                   + slope1 * s * sh_minus * sh_minus / h1_2;

            // compute derivative in this dimension
            // assume linear change of slope across interval

            if (deriv_vec) {
                const DATA_TYPE u = s / h1;
                deriv = slope1 * (1.0 - u) + slope2 * u;
				deriv_vec[dim] = deriv ;
                if (dim > 0) {
                    deriv_vec[dim-1] = dy2 * sh_term / h1_3
									 + dy1 * (h1_3 - sh_term) / h1_3
									 + dslope2 * s_2 * sh_minus / h1_2
									 + dslope1 * s * sh_minus * sh_minus / h1_2;
                }
            }

            // use results for dim+1 iteration

            return result;
        }

}; // end data_grid class

//*************************************************************************
// implementation of forward methods

/**
 * Recursion engine for multi-dimensional interpolation.
 */
template<class DATA_TYPE, size_t NUM_DIMS>
DATA_TYPE data_grid<DATA_TYPE, NUM_DIMS>::interp(int dim,
        const size_t* index, const double* location, DATA_TYPE& deriv,
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
