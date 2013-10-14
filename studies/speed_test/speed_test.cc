/**
 * @file folds.cc
 */
#include <usml/waveq3d/waveq3d.h>
#include <usml/netcdf/netcdf_files.h>
#include <fstream>
#include <sys/time.h>

//#define USML_DEBUG

using namespace usml::netcdf ;
using namespace usml::ocean ;

/**
 * Cubic generating function and its derivative
 * functions in the x and y directions.
 */

static double cubic2d( double _val[] ) {
    double func_val = ( _val[0]*_val[0]*_val[0] ) *
            ( _val[1]*_val[1]*_val[1] ) ;
    return func_val ;
}

//static double deriv2d_x( double _val[] ) {
//    double func_val = 3.0 * ( _val[0]*_val[0] ) *
//            ( _val[1]*_val[1]*_val[1] ) ;
//    return func_val ;
//}
//
//static double deriv2d_y( double _val[] ) {
//    double func_val = 3.0 * ( _val[1]*_val[1] ) *
//            ( _val[0]*_val[0]*_val[0] ) ;
//    return func_val ;
//}

/**
 * Command line interface.
 */
int main() {
    cout << "=== datagrid_interpolation_speed_test ===" << endl;

    int num_points = 1e5 ;
    int param = 5 ;
    int counter = 0 ;
    unsigned index[2] ;
    double x_y[2] ;

    seq_vector* ax[2] ;
    ax[0] = new seq_linear(1.0, 1.0, 5) ;
    ax[1] = new seq_linear(1.0, 1.0, 5) ;
    data_grid<double,2>* grid = new data_grid<double,2>(ax);

    for(int i = 0; i<2; ++i) {
//        grid->interp_type(i, GRID_INTERP_NEAREST);
//        grid->interp_type(i, GRID_INTERP_LINEAR);
        grid->interp_type(i, GRID_INTERP_PCHIP);
        grid->edge_limit(i, true);
    }

    for(int i=0; i < (*ax[0]).size(); ++i) {
        for(int j=0; j < (*ax[1]).size(); ++j) {
                index[0] = i ;
                index[1] = j ;
                x_y[0] = i + 1 ;
                x_y[1] = j + 1 ;
                double number = cubic2d(x_y) ;
                grid->data( index, number ) ;
        }
    }

    cout << "==========simple_data grid=============" << endl;
    cout << "axis[0]: " << *ax[0] << endl;
    cout << "axis[1]: " << *ax[1] << endl;
    for(int i=0; i < (*ax[0]).size(); i++ ) {
        for(int j=0; j < (*ax[1]).size(); j++ ) {
            index[0] = i ;
            index[1] = j ;
            cout << grid->data(index) ;
            (j < (*ax[1]).size()-1) ? cout << "\t" :  cout << endl;
        }
    }
    cout << endl;

    double spot[2] ;
    matrix<double*> location (num_points,1) ;
    for(int i=0; i<num_points; ++i) {
        spot[0] = param * randgen::uniform();
        spot[1] = param * randgen::uniform();
        location(i,0) = spot ;
    }

    struct timeval time ;
    struct timezone zone ;
    gettimeofday( &time, &zone ) ;
    double start = time.tv_sec + time.tv_usec * 1e-6 ;
    while ( counter != num_points ) {
        grid->interpolate( location(counter, 0) );
        ++counter;
    }
    gettimeofday( &time, &zone ) ;
    double complete = time.tv_sec + time.tv_usec * 1e-6 ;

	cout << "Time to complete interpolation using data_grid method was "
		 << (complete-start) << " sec." << endl;

//    data_grid_fast_2d* fast_grid = new data_grid_fast_2d(*grid, true);
//    counter = 0 ;
//    gettimeofday( &time, &zone ) ;
//    start = time.tv_sec + time.tv_usec * 1e-6 ;
//    while ( counter != num_points ) {
//        fast_grid->interpolate( location(counter, 0) );
//        ++counter;
//    }
//    gettimeofday( &time, &zone ) ;
//    complete = time.tv_sec + time.tv_usec * 1e-6 ;
//
//	cout << "Time to complete interpolation using fast_grid method was "
//		 << (complete-start) << " sec." << endl;

}

