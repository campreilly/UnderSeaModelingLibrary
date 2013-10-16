/**
 * @file data_grid_fast_3d.h
 * Wrapper for a data_grid in 3d that uses the fast non-recursive
 * interpolation algorithm.
 */
#ifndef USML_TYPES_DATA_GRID_FAST_3D_H
#define USML_TYPES_DATA_GRID_FAST_3D_H

#include <usml/types/data_grid.h>

//#define FAST_GRID_DEBUG
//#define FAST_PCHIP_GRID_DEBUG

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
        unsigned fast_index[3];

        double pchip_part(const unsigned _k, const double _location,
                          const double* _data, double* _deriv) const
        {
            double y0 = _data[0], y1 = _data[1], y2 = _data[2], y3 = _data[3];

            double inc0 = _axis[0]->increment(_k-1) ;
            double inc1 = _axis[0]->increment(_k) ;
            double inc2 = _axis[0]->increment(_k+1) ;
            double slope_1 = (y2 - y1)/(2*inc1) + (y1 - y0)/(2*inc0);
            double slope_2 = (y3 - y2)/(2*inc2) + (y2 - y1)/(2*inc1);

            double t = ( _location - (*_axis[0])(_k) ) / inc1 ;
            double t_2 = t*t ;
            double t_3 = t_2*t ;

            double h00 = ( 2*t_3 - 3*t_2 + 1 ) ;
            double h10 = ( t_3 - 2*t_2 + t ) ;
            double h01 = ( 3*t_2 - 2*t_3 ) ;
            double h11 = ( t_3 - t_2 ) ;

            double _result = h00 * y1 + h10 * slope_1 +
                             h01 * y2 + h11 * slope_2 ;

            #ifdef FAST_PCHIP_GRID_DEBUG
                cout << "y0: " << y0 << "\ty1: " << y1
                     << "\ty2: " << y2 << "\ty3: " << y3 << endl;
                cout << "t: " << t << "\tt_2: " << t_2
                     << "\tt_3: " << t_3 << endl;
                cout << "inc0: " << inc0 << "\tinc1: " << inc1
                     << "\tinc2: " << inc2 << endl;
                cout << "slope_1: " << slope_1
                     << "\tslope_2: " << slope_2 << endl;
                cout << "h00: " << h00 << "\th10: " << h10
                     << "\th01: " << h01 << "\th11: " << h11 << endl;
                cout << "_result is: " << _result << endl;
            #endif

            if(_deriv) {
                _deriv[0] = ( 6*t_2 - 6*t ) * y1 +
                            ( 3*t_2 - 4*t + 1 ) * slope_1 +
                            ( 6*t - 6*t_2 ) * y2 +
                            ( 3*t_2 - 2*t ) * slope_2 ;
            }

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
                data_grid(grid,copy_data)
        {
            fast_index[0] = 0;
            fast_index[1] = 0;
            fast_index[2] = 0;
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

        double interpolate(double* location, double* derivative = NULL) {
            double result = 0;
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

            #ifdef FAST_GRID_DEBUG
                cout << "offset: (" << _offset[0] << "," << _offset[1] << "," << _offset[2] << ")" << endl;
                cout << "axis[0]: " << (*_axis[0])(_offset[0]) << "\taxis[1]: " << (*_axis[1])(_offset[1])
                     << "\taxis[2]: " << (*_axis[2])(_offset[2]) << endl;
            #endif
            double v = location[0] - (*_axis[0])(_offset[0]) ;
            if(derivative) {derivative[1] = derivative[2] = 0;}

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
                unsigned temp_index [3];
                double s ;

                for(int i=0; i<4; ++i) {
                    fast_index[0] = _offset[0]+i-1;
                    temp_index[0] = fast_index[0];
                    temp_index[1] = _offset[1];
                    temp_index[2] = _offset[2];
                    x = location[1];
                    x1 = (*_axis[1])(temp_index[1]) ;
                    x2 = (*_axis[1])(temp_index[1]+1) ;
                    y = location[2];
                    y1 = (*_axis[2])(temp_index[2]) ;
                    y2 = (*_axis[2])(temp_index[2]+1) ;
                    f11 = data(temp_index);
                    fast_index[1] = temp_index[1]+1 ; fast_index[2] = temp_index[2] ;
                    f21 = data(fast_index);
                    fast_index[1] = temp_index[1] ; fast_index[2] = temp_index[2]+1 ;
                    f12 = data(fast_index);
                    fast_index[1] = temp_index[1]+1 ; fast_index[2] = temp_index[2]+1 ;
                    f22 = data(fast_index);
                    x_diff = x2 - x1 ;
                    y_diff = y2 - y1 ;
                    #ifdef FAST_GRID_DEBUG
                        cout << "iteration: " << i << endl;
                        cout << "temp_index: (" << temp_index[0] << ", " << temp_index[1] << ", " << temp_index[2] << ")" << endl;
                        cout << "x: " << x << "\tx1: " << x1 << "\tx2: " << x2 << endl;
                        cout << "y: " << y << "\ty1: " << y1 << "\ty2: " << y2 << endl;
                        cout << "f11: " << f11 << "\tf21: " << f21 << "\tf12: " << f12 << "\tf22: " << f22 << endl;
                    #endif
                    result = ( f11*(x2-x)*(y2-y) +
                               f21*(x-x1)*(y2-y) +
                               f12*(x2-x)*(y-y1) +
                               f22*(x-x1)*(y-y1) ) / (x_diff*y_diff);
                    if(derivative) {
                        if(i==1) {s = (1 - v);}
                        else if(i==2) {s = v;}
                        else {s = 0;}
                        derivative[1] += s * ( -f11*(y2-y) + f21*(y2-y) - f12*(y-y1) + f22*(y-y1) )
                                        / ( x_diff * y_diff );
                        derivative[2] += s * ( -f11*(x2-x) - f21*(x-x1) + f12*(x2-x) + f22*(x-x1) )
                                        / ( x_diff * y_diff );
                    }
                    interp_values[i] = result;
                }
                #ifdef FAST_GRID_DEBUG
                    cout << "_offset[0]: " << _offset[0] << "\tlocation[0]: " << location[0] << endl;
                    cout << "interp_values: (" << interp_values[0] << "," << interp_values[1] << "," <<
                        interp_values[2] << "," << interp_values[3] << ")" << endl;
                #endif
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
                        cout << "result in void interp: " << (*result)(n, m) << endl;
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
