/**
 * @file data_grid_svp.h
 * Wrapper for a data_grid in 3d that uses the fast non-recursive
 * interpolation algorithm.
 */
#ifndef USML_TYPES_DATA_GRID_SVP_H
#define USML_TYPES_DATA_GRID_SVP_H

#include <usml/types/data_grid.h>

//#define FAST_GRID_DEBUG
//#define FAST_PCHIP_GRID_DEBUG
//#define FAST_NaN_GRID_DEBUG

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
*          implement the data_grid_bathy wrapper instead.
*
* Since the data is passed in and referenced by this wrapper,
* the data is taken control of and destroyed at the end of its
* use cycle.
*/


class USML_DECLSPEC data_grid_svp : public data_grid<double,3>
{
    private:

        /**
        * Create all variables needed for each calculation once
        * to same time and memory.
        */

        unsigned _offset[3];
        unsigned kmin ;                 //minimum value on an axis
        unsigned kzmax, kxmax, kymax ;  //max index on z-axis (depth)
        unsigned k0, k1, k2 ;           //indecies of he offset data
        double result ;                 //interpolated value
            //bi-linear variables
        double f11, f21, f12, f22, x_diff, y_diff ;
        double x, x1, x2, y, y1, y2 ;
        c_matrix<double,2,2> interp_plane ;
            //pchip variables
        c_matrix<double,2,2> dz ;
        double v0, v1, v2, v3 ;
        double inc0, inc1, inc2 ;
        double d0, d1, d2 ;
        double t, t_2, t_3 ;
        double w0, w1, w2, w3 ;
        double slope_1, slope_2 ;
        double h00, h10, h01, h11 ;
        double*** derv_z ;

        /** Utility accessor function for data grid values */
        inline double data_3d(unsigned dim0, unsigned dim1, unsigned dim2) {
            unsigned _grid_index[3] ;
            _grid_index[0] = dim0 ;
            _grid_index[1] = dim1 ;
            _grid_index[2] = dim2 ;
            return data(_grid_index) ;
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

        data_grid_svp( const data_grid<double,3>& grid, bool copy_data = true) :
                data_grid(grid,copy_data), kmin(-1), kzmax(_axis[0]->size()-1u),
                kxmax(_axis[1]->size()-1u), kymax(_axis[2]->size()-1u)
        {
            if( interp_type(0)!=GRID_INTERP_PCHIP ) { interp_type(0, GRID_INTERP_PCHIP) ; }
            if( (interp_type(1)!=GRID_INTERP_LINEAR) || (interp_type(1)!=GRID_INTERP_LINEAR) ) {
                interp_type(1, GRID_INTERP_LINEAR) ;
                interp_type(2, GRID_INTERP_LINEAR) ;
            }
            derv_z = new double**[kzmax+1u] ;
            for(int i=0; i<kzmax+1u; ++i) {
                derv_z[i] = new double*[kxmax+1u] ;
                for(int j=0; j<kxmax+1u; ++j) {
                    derv_z[i][j] = new double[kymax+1u] ;
                }
            }
            for(int i=0; i<kzmax+1u; ++i) {
                for(int j=0; j<kxmax+1u; ++j) {
                    for(int k=0; k<kymax+1u; ++k) {
                        if(i==0) {
                            inc1 = _axis[0]->increment(i) ;
                            inc2 = _axis[0]->increment(i+1) ;
                            slope_1 = ( data_3d(i+1,j,k) - data_3d(i,j,k) ) /
                                      inc1 ;
                            slope_2 = ( data_3d(i+2,j,k) - data_3d(i+1,j,k) ) /
                                      inc2 ;
                            result = ( (2.0*inc1 + inc2)*slope_1 - inc1*slope_2 ) /
                                    ( inc1 + inc2 ) ;
                            derv_z[i][j][k] = result ;
                            if(result*slope_1 <= 0.0) {
                                derv_z[i][j][k] = 0.0 ;
                            } else
                            if( (slope_1*slope_2 <=0.0) && (abs(result) > abs(3.0*slope_1)) ) {
                                derv_z[i][j][k] = 3.0*slope_1 ;
                            }
                            #ifdef FAST_NaN_GRID_DEBUG
                                if(abs(derv_z[i][j][k])>=20.0) {
                                    cout << "***Warning: bogus derivative in the z-direction***" << endl;
                                    cout << "---i==0 condition---" << endl;
                                    cout << "inc1: " << inc1 << "\tinc2: " << inc2 << endl;
                                    cout << "data(" << i << "," << j << "," << k << "): " << data_3d(i,j,k)
                                         << "\tdata(" << i+1 << "," << j << "," << k << "): " << data_3d(i+1,j,k)
                                         << "\tdata(" << i+2 << "," << j << "," << k << "): " << data_3d(i+2,j,k)
                                         << endl;
                                    cout << "slope1: " << slope_1 << "\tslope2: " << slope_2 << endl;
                                    cout << "result: " << result << endl;
                                    cout << "derv_z[" << i << "][" << j << "][" << k << "]: "
                                         << derv_z[i][j][k] << endl;
                                }
                            #endif
                        } else
                        if(i==kzmax) {
                            inc1 = _axis[0]->increment(i-1) ;
                            inc2 = _axis[0]->increment(i) ;
                            slope_1 = ( data_3d(i-1,j,k) - data_3d(i-2,j,k) ) /
                                      inc1 ;
                            slope_2 = ( data_3d(i,j,k) - data_3d(i-1,j,k) ) /
                                      inc2 ;
                            result = ( (2.0*inc1 + inc2)*slope_2 - inc1*slope_1 ) /
                                    ( inc1 + inc2 ) ;
                            derv_z[i][j][k] = result ;
                            if(result*slope_1 <= 0.0) {
                                derv_z[i][j][k] = 0.0 ;
                            } else
                            if( (slope_1*slope_2 <=0.0) && (abs(result) > abs(3.0*slope_1)) ) {
                                derv_z[i][j][k] = 3.0*slope_1 ;
                            }
                            #ifdef FAST_NaN_GRID_DEBUG
                                if(abs(derv_z[i][j][k])>=20.0) {
                                    cout << "***Warning: bogus derivative in the z-direction***" << endl;
                                    cout << "---i==kzmax condition---" << endl;
                                    cout << "inc1: " << inc1 << "\tinc2: " << inc2 << endl;
                                    cout << "data(" << i-2 << "," << j << "," << k << "): " << data_3d(i-2,j,k)
                                         << "\tdata(" << i-1 << "," << j << "," << k << "): " << data_3d(i-1,j,k)
                                         << "\tdata(" << i << "," << j << "," << k << "): " << data_3d(i,j,k)
                                         << endl;
                                    cout << "slope1: " << slope_1 << "\tslope2: " << slope_2 << endl;
                                    cout << "result: " << result << endl;
                                    cout << "derv_z[" << i << "][" << j << "][" << k << "]: "
                                         << derv_z[i][j][k] << endl;
                                }
                            #endif
                        } else {
                            inc1 = _axis[0]->increment(i-1) ;
                            inc2 = _axis[0]->increment(i) ;
                            w1 = 2.0*inc2 + inc1 ;
                            w2 = inc2 + 2.0*inc1 ;
                            slope_1 = ( data_3d(i,j,k) - data_3d(i-1,j,k) ) /
                                      inc1 ;
                            slope_2 = ( data_3d(i+1,j,k) - data_3d(i,j,k) ) /
                                      inc2 ;
                            if(slope_1*slope_2 <= 0.0 ) {
                                derv_z[i][j][k] = 0.0 ;
                            } else {
                                derv_z[i][j][k] = (w1 + w2) /
                                                  ( (w1 / slope_1) + (w2 / slope_2) ) ;
                            }
                            #ifdef FAST_NaN_GRID_DEBUG
                                if(abs(derv_z[i][j][k])>=20.0) {
                                    cout << "***Warning: bogus derivative in the z-direction***" << endl;
                                    cout << "---else condition---" << endl;
                                    cout << "inc1: " << inc1 << "\tinc2: " << inc2 << endl;
                                    cout << "data(" << i-1 << "," << j << "," << k << "): " << data_3d(i-1,j,k)
                                         << "\tdata(" << i << "," << j << "," << k << "): " << data_3d(i,j,k)
                                         << "\tdata(" << i+1 << "," << j << "," << k << "): " << data_3d(i+1,j,k)
                                         << endl;
                                    cout << "slope1: " << slope_1 << "\tslope2: " << slope_2 << endl;
                                    cout << "w1: " << w1 << "\tw2: " << w2 << endl;
                                    cout << "derv_z[" << i << "][" << j << "][" << k << "]: "
                                         << derv_z[i][j][k] << endl;
                                }
                            #endif
                        }
                    } //end for-loop in k
                } //end for-loop in j
            } //end for-loop in i
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
                cout << "axis[0]: " ;
                ( (*_axis[0])(_offset[0]) >= 1e6 ) ? (cout << (*_axis[0])(_offset[0])-wposition::earth_radius)
                            : cout << (*_axis[0])(_offset[0]) ;
                cout << "\taxis[1]: " << (*_axis[1])(_offset[1])
                     << "\taxis[2]: " << (*_axis[2])(_offset[2]) << endl;
                cout << "derv_z: (" << derv_z[_offset[0]][_offset[1]][_offset[2]]
                     << ", " << derv_z[_offset[0]+1][_offset[1]][_offset[2]]
                     << ", " << derv_z[_offset[0]+2][_offset[1]][_offset[2]]
                     << ", " << derv_z[_offset[0]+3][_offset[1]][_offset[2]] << ")" <<  endl;
            #endif

                /** PCHIP contribution in zeroth dimension */
            if(derivative) {derivative[0] = 0 ;}
            k0 = _offset[0] ;
            k1 = _offset[1] ;
            k2 = _offset[2] ;

            //construct the interpolated plane to which the final bi-linear
            //interoplation will happen
            for(int i=0; i<2; ++i) {
                for(int j=0; j<2; ++j) {
                        //extract data and take precautions when at boundaries of the axis
                    v1 = data_3d(k0,k1+i,k2+j) ;
                    v2 = data_3d(k0+1,k1+i,k2+j) ;
                    inc1 = _axis[0]->increment(k0) ;

                    t = ( location[0] - (*_axis[0])(k0) ) / inc1 ;
                    t_2 = t*t ;
                    t_3 = t_2*t ;

                        //construct the hermite polynomials
                    h00 = ( 2*t_3 - 3*t_2 + 1 ) ;
                    h10 = ( t_3 - 2*t_2 + t ) ;
                    h01 = ( 3*t_2 - 2*t_3 ) ;
                    h11 = ( t_3 - t_2 ) ;

                    interp_plane(i,j) = h00 * v1 + h10 * derv_z[k0][k1+i][k2+j] +
                                        h01 * v2 + h11 * derv_z[k0+1][k1+i][k2+j] ;

                    #ifdef FAST_PCHIP_GRID_DEBUG
                        cout << "v1: " << v1 << "\tv2: " << v2 << endl;
                        cout << "inc1: " << inc1 << endl;
                        cout << "t: " << t << "\tt_2: " << t_2
                             << "\tt_3: " << t_3 << endl;
                        cout << "slope_1: " << derv_z[k0][k1+i][k2+j]
                             << "\tslope_2: " << derv_z[k0+1][k1+i][k2+j] << endl;
                        cout << "h00: " << h00 << "\th10: " << h10
                             << "\th01: " << h01 << "\th11: " << h11 << endl;
                        cout << "interp_plane(" << i << ", " << j << "): "
                             << interp_plane(i,j) << endl;
                    #endif

                    if(derivative) {
                        dz(i,j) = ( 6*t_2 - 6*t ) * v1/inc1 +
                                  ( 3*t_2 - 4*t + 1 ) * derv_z[k0][k1+i][k2+j]/inc1 +
                                  ( 6*t - 6*t_2 ) * v2/inc1 +
                                  ( 3*t_2 - 2*t ) * derv_z[k0+1][k1+i][k2+j]/inc1 ;
                    }
                }
            }

                /** Bi-Linear contributions from first/second dimensions */
                //extract data around field point
            x = location[1];
            x1 = (*_axis[1])(k1) ;
            x2 = (*_axis[1])(k1+1) ;
            y = location[2];
            y1 = (*_axis[2])(k2) ;
            y2 = (*_axis[2])(k2+1) ;
            f11 = interp_plane(0,0) ;
            f21 = interp_plane(1,0) ;
            f12 = interp_plane(0,1) ;
            f22 = interp_plane(1,1) ;
            x_diff = x2 - x1 ;
            y_diff = y2 - y1 ;

            #ifdef FAST_GRID_DEBUG
                cout << "k_indecies: (" << k0 << ", " << k1 << ", " << k2 << ")" << endl;
                cout << "x: " << x << "\tx1: " << x1 << "\tx2: " << x2 << endl;
                cout << "y: " << y << "\ty1: " << y1 << "\ty2: " << y2 << endl;
                cout << "f11: " << f11 << "\tf21: " << f21 << "\tf12: " << f12 << "\tf22: " << f22 << endl;
            #endif

            result = ( f11*(x2-x)*(y2-y) +
                       f21*(x-x1)*(y2-y) +
                       f12*(x2-x)*(y-y1) +
                       f22*(x-x1)*(y-y1) ) / (x_diff*y_diff);

            if(derivative) {
                derivative[0] = ( dz(0,0)*(x2-x)*(y2-y) + dz(1,0)*(x-x1)*(y2-y) +
                                  dz(0,1)*(x2-x)*(y-y1) + dz(1,1)*(x-x1)*(y-y1) )
                                / (x_diff*y_diff) ;
                derivative[1] = ( -f11*(y2-y) + f21*(y2-y) - f12*(y-y1) + f22*(y-y1) )
                                / ( x_diff * y_diff ) ;
                derivative[2] = ( -f11*(x2-x) - f21*(x-x1) + f12*(x2-x) + f22*(x-x1) )
                                / ( x_diff * y_diff ) ;
            }

            return result;
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
