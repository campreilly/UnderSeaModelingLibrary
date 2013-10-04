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
        * Holds the offsets from each axis
        */

        unsigned _offset[3];

        double pchip_part(const unsigned _k, const double _location,
                          const double* _data, double* _deriv) const
        {

            const unsigned kmin = 1u ;					  // at endpt if k-1 < 0
            const unsigned kmax = _axis[2]->size()-3u ; // at endpt if k+2 > N-1
            double y0 = _data[0], y1 = _data[1], y2 = _data[2], y3 = _data[3];

            // compute difference values used frequently in computation

            double h0 = _axis[0]->increment(_k - 1); 	// interval from k-1 to k
            double h1 = _axis[0]->increment(_k);	   	// interval from k to k+1
            double h2 = _axis[0]->increment(_k + 1); 	// interval from k+1 to k+2
            double h1_2 = h1 * h1; 		   	// k to k+1 interval squared
            double h1_3 = h1_2 * h1;		   	// k to k+1 interval cubed

            double s = _location-(*_axis[0])(_k);	// local variable
            double s_2 = s * s, s_3 = s_2 * s;	// s squared and cubed
            double sh_minus = s - h1;
            double sh_term = 3.0 * h1 * s_2 - 2.0 * s_3;

            // compute first divided differences (forward derivative)
            // for both the values, and their derivatives

            double deriv0 = (y1 - y0) / h0;	// fwd deriv from k-1 to k
            double deriv1 = (y2 - y1) / h1; 	// fwd deriv from k to k+1
            double deriv2 = (y3 - y2) / h2; 	// fwd deriv from k+1 to k+2

            //*************
            // compute weighted harmonic mean of slopes around index k
            // for both the values, and their derivatives
            // set it zero at local maxima or minima
            // deriv0 * deriv1 condition guards against division by zero

            double slope1=0.0;

            // when not at an end-point, slope1 is the harmonic, weighted
            // average of deriv0 and deriv1.

            if ( _k >= kmin ) {
                double w0 = 2.0 * h1 + h0;
                double w1 = h1 + 2.0 * h0;
                if ( deriv0 * deriv1 > 0.0 ) {
                    slope1 = (w0 + w1) / ( w0 / deriv0 + w1 / deriv1 );
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
            }

            //*************
            // compute weighted harmonic mean of slopes around index k+1
            // for both the values, and their derivatives
            // set it zero at local maxima or minima
            // deriv1 * deriv2 condition guards against division by zero

            double slope2=0.0;

            // when not at an end-point, slope2 is the harmonic, weighted
            // average of deriv1 and deriv2.

            if ( _k <= kmax ) {
                double w1 = 2.0 * h1 + h0;
                double w2 = h1 + 2.0 * h0;
                if ( deriv1 * deriv2 > 0.0 ) {
                    slope2 = (w1 + w2) / ( w1 / deriv1 + w2 / deriv2 );
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
            }

            // compute interpolation value in this dimension

            double _result = y2 * sh_term / h1_3
                   + y1 * (h1_3 - sh_term) / h1_3
                   + slope2 * s_2 * sh_minus / h1_2
                   + slope1 * s * sh_minus * sh_minus / h1_2;

            // compute derivative in this dimension
            // assume linear change of slope across interval

            if(_deriv) {
                double u = s / h1;
                _deriv[0] = slope1 * (1.0 - u) + slope2 * u;
            }

            // use results for dim+1 iteration

            return _result;
        }

    public:

        /**
        * Creates a fast interpolation grid from an existing
        * data_grid.
        *
        * @param grid      The data_grid that is to be wrapped.
        * @param copy_data If true, copies the data grids data
        *                  fields as well as the axises.
        */

        data_grid_fast_3d( const data_grid<double,3>& grid, bool copy_data = true) :
            data_grid(grid,copy_data) {}

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

        double interpolate(double* location, double* derivative = NULL) {
            double result = 0;
            unsigned fast_index[3];
            fast_index[0] = 0;
            fast_index[1] = 0;
            fast_index[2] = 0;
            // find the interval index in each dimension

            for (unsigned dim = 0; dim < 3; ++dim) {

            // limit interpolation to axis domain if _edge_limit turned on

                if ( edge_limit(dim) ) {
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

            switch(interp_type(1)) {

                ///****nearest****
            case -1:
                for(int dim = 0 ; dim < 3; ++dim) {
                    double inc = _axis[dim]->increment(0) ;
                    double u = abs(location[dim]-(*_axis[dim])(_offset[dim])) / inc ;
                    if(u < 0.5) {
                        fast_index[dim] = _offset[dim];
                    } else {
                        fast_index[dim] = _offset[dim]+1;
                    }
                }
                if(derivative) derivative[0] = derivative[1] = derivative[2] = 0;
                return data(fast_index);
                break;

                ///****linear****
            case 0:
                double f11, f21, f12, f22, x_diff, y_diff;
                double x, x1, x2, y, y1, y2;
                double interp_values[4];

                for(int i=0; i<4; ++i) {
                    fast_index[0] = _offset[0]+i-1;
                    unsigned temp_index [4];
                    temp_index[0] = fast_index[0];
                    temp_index[1] = _offset[1];
                    temp_index[2] = _offset[2];
                    x = location[1];
                    x1 = (*_axis[1])(_offset[1]+i-1) ;
                    x2 = (*_axis[1])(_offset[1]+i) ;
                    y = location[2];
                    y1 = (*_axis[2])(_offset[2]+i-1) ;
                    y2 = (*_axis[2])(_offset[2]+i) ;
                    f11 = data(temp_index);
                    fast_index[1] = _offset[1]+i ; fast_index[2] = _offset[2]+i-1 ;
                    f21 = data(fast_index);
                    fast_index[1] = _offset[1]+i-1 ; fast_index[2] = _offset[2]+i ;
                    f12 = data(fast_index);
                    fast_index[1] = _offset[1]+i ; fast_index[2] = _offset[2]+i ;
                    f22 = data(fast_index);
                    x_diff = x2 - x1 ;
                    y_diff = y2 - y1 ;
                    result = ( f11*(x2-x)*(y2-y) +
                               f21*(x-x1)*(y2-y) +
                               f12*(x2-x)*(y-y1) +
                               f22*(x-x1)*(y-y1) ) / (x_diff*y_diff);

                    if(derivative) {
                        derivative[1] = (f21 - f11) / _axis[1]->increment(_offset[1]);
                        derivative[2] = (f12 - f11) / _axis[2]->increment(_offset[2]);
                    }
                    interp_values[i] = result;
                }
                return pchip_part(_offset[0],location[0],interp_values,derivative);
                break;

            case 1:
                throw std::invalid_argument("Interpolation of Pchip in 3 dimensions not yet implemented.");
                break;

            default:
                throw std::invalid_argument("Invalid grid type");
                break;

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
            double derivative[3];
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
};

} // end of namespace types
} // end of namespace usml

#endif
