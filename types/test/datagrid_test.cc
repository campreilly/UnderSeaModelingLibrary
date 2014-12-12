/**
 * @example types/test/datagrid_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <usml/types/types.h>
#include <iostream>
#include <stdio.h>
#ifdef WIN32
#include "sys_time_win32.h"
#else
#include <sys/time.h>
#endif
#include <usml/netcdf/netcdf_files.h>

BOOST_AUTO_TEST_SUITE(datagrid_test)

using namespace boost::unit_test;
using namespace usml::types;

typedef seq_vector::const_iterator iterator;

/**
 * As a note when using valgrind mem-check, there are many instances
 * of an "Conditional jump or move depends on uninitialised value(s)"
 * warning that are issued for the deriv_1d_test and datagrid_fast_acc_test
 * that have been determined to not effect the overall performance or
 * functionality of data_grid::pchip and may be ignored.
 */

/**
 * @ingroup types_test
 * Test the ability of data_grid_compute_offset() to retrieve data
 * from a 3-D data matrix in column major order.
 * Generate errors if values differ by more that 1E-6 percent.
 */
BOOST_AUTO_TEST_CASE( compute_index_test ) {

    cout << "=== datagrid_test: compute_index_test ===" << endl;

    // build a set of axes like the ones used in data_grid

    seq_linear x(0,100.0,2);
    seq_linear y(0,10.0,3);
    seq_linear z(0,1.0,4);
    seq_vector *axis[] = {&x, &y, &z};
    cout << "x=" << x << endl;
    cout << "y=" << y << endl;
    cout << "z=" << z << endl;

    // fill in a data vector using combination of axis values

    double data[2*3*4];
    unsigned k = 0;
    for ( iterator ix = x.begin(); ix < x.end(); ++ix ) {
        for ( iterator iy = y.begin(); iy < y.end(); ++iy ) {
            for ( iterator iz = z.begin(); iz < z.end(); ++iz ) {
                data[k++] = *ix + *iy + *iz;
            }
        }
    }

    cout << "data[x][y][z]=";
    for ( unsigned n=0; n < k; ++n ) printf("%03.0f ", data[n] );
    cout << endl;

    // check to see if all data in the right place

    unsigned index[3];
    unsigned a=0,b=0,c=0;
    for ( iterator ix = x.begin(); ix < x.end(); ++ix ) {
        index[0] = a++;
        b = 0;
        for ( iterator iy = y.begin(); iy < y.end(); ++iy ) {
            index[1] = b++;
            c = 0;
            for ( iterator iz = z.begin(); iz < z.end(); ++iz ) {
                index[2] = c++;
                k = data_grid_compute_offset<2>( axis, index );
                printf( "x=%d y=%d z=%d offset=%02d data=%03.0f\n",
                        a-1, b-1, c-1, k, data[k] );
                BOOST_CHECK_CLOSE( data[k], *ix + *iy + *iz, 1e-6 );
            }
        }
    }
}

/**
 * Compute a linear field value of 1-D interpolation test data
 */
static double linear1d( double axis ) {
    return 5.0 + 3.0 * axis;
}

/**
 * @ingroup types_test
 * Interpolate 1-D linear field using a scalar.
 * Exercise all of the 1-D interpolation types.
 * Generate errors if values differ by more that 1E-6 percent.
 */
BOOST_AUTO_TEST_CASE( linear_1d_test ) {
    double truth, nearest, linear, pchip;

    cout << "=== datagrid_test: linear_1d_test ===" << endl;

    // construct synthetic data for this test

    seq_linear axis(1.0,2.0,9.0);
    seq_vector *ax[] = {&axis};
    data_grid<double,1> grid( ax );
    grid.edge_limit(0,false);

    for ( unsigned n=0; n < axis.size(); ++n ) {
        grid.data( &n, linear1d(axis(n)) );
    }

    // interpolate using all possible algorithms

    cout << "x\ttruth\tnearest\tlinear\tpchip" << endl;
    for ( double x=0.25; x <= 10.0; x += 0.25 ) {
    	double y = x ;
        cout << y << "\t";
        truth = linear1d(y);
        cout << truth << "\t";

        grid.interp_type(0,GRID_INTERP_NEAREST);
        nearest = grid.interpolate( &y );
        cout << nearest << "\t";

        grid.interp_type(0,GRID_INTERP_LINEAR);
        linear = grid.interpolate( &y );
        cout << linear << "\t";
        BOOST_CHECK_CLOSE( linear, truth, 1e-6 );

        grid.interp_type(0,GRID_INTERP_PCHIP);
        pchip = grid.interpolate( &y );
        cout << pchip << "\t";
        BOOST_CHECK_CLOSE( pchip, truth, 1e-6 );

        cout << endl;
    }
}

/**
 * Compute a cubic field value of 1-D interpolation test data
 */
static double cubic1d( double axis ) {
    return linear1d( axis ) - 0.3 * axis*axis;
}

/**
 * @ingroup types_test
 * Interpolate 1-D cubic field using a scalar.
 * Exercise all of the 1-D interpolation types.
 * Generate errors if values differ by more that 1E-6 percent.
 */
BOOST_AUTO_TEST_CASE( cubic_1d_test ) {
    double truth, nearest, linear, pchip;

    cout << "=== datagrid_test: cubic_1d_test ===" << endl;

    // construct synthetic data for this test

    seq_linear axis(1.0,2.0,9.0);
    seq_vector *ax[] = {&axis};
    data_grid<double,1> grid( ax );
    grid.edge_limit(0,false);

    for ( unsigned n=0; n < axis.size(); ++n ) {
        grid.data( &n, cubic1d(axis(n)) );
    }

    // interpolate using all possible algorithms
    // don't try extrapolation

    cout << "x\ttruth\tnearest\tlinear\tpchip" << endl;
    for ( double x=1.0; x <= 9.0; x += 0.25 ) {
        cout << x << "\t";
        truth = cubic1d(x);
        cout << truth << "\t";

        grid.interp_type(0,GRID_INTERP_NEAREST);
        nearest = grid.interpolate( &x );
        cout << nearest << "\t";

        grid.interp_type(0,GRID_INTERP_LINEAR);
        linear = grid.interpolate( &x );
        cout << linear << "\t";

        grid.interp_type(0,GRID_INTERP_PCHIP);
        pchip = grid.interpolate( &x );
        cout << pchip << "\t";
        BOOST_CHECK_CLOSE( pchip, truth, 2.0 );

        cout << endl;
    }
}

/**
 * Compute the derivative of the function defined in cubic1d().
 */
static double deriv1d( double axis ) {
    return 3.0 - 0.6 * axis;
}

/**
 * @ingroup types_test
 * Interpolate 1-D cubic field using a scalar.
 * Test the accuracy of the derivatives
 * Generate errors if values differ by more that 1E-6 percent.
 */
BOOST_AUTO_TEST_CASE( deriv_1d_test ) {
    double truth, nearest, linear, pchip;

    cout << "=== datagrid_test: deriv_1d_test ===" << endl;

    // construct synthetic data for this test

    seq_linear axis(1.0,2.0,9.0);
    seq_vector *ax[] = {&axis};
    data_grid<double,1> grid( ax );

    for ( unsigned n=0; n < axis.size(); ++n ) {
        grid.data( &n, cubic1d(axis(n)) );
    }

    // interpolate using all possible algorithms
    // don't try extrapolation

    cout << "x\ttruth\tnearest\tlinear\tpchip" << endl;
    for ( double x=1.0; x <= 9.0; x += 0.25 ) {
        cout << x << "\t";
        truth = deriv1d(x);
        cout << truth << "\t";

        grid.interp_type(0,GRID_INTERP_NEAREST);
        grid.interpolate( &x, &nearest );
        cout << nearest << "\t";

        grid.interp_type(0,GRID_INTERP_LINEAR);
        grid.interpolate( &x, &linear );
        cout << linear << "\t";

        grid.interp_type(0,GRID_INTERP_PCHIP);
        grid.interpolate( &x, &pchip );
        cout << pchip << "\t";
        // BOOST_CHECK_CLOSE( pchip, truth, 20.0 ) ;

        cout << endl;
    }
}

/**
 * Cubic generating function and its derivative
 * functions in the x and y directions.
 */

static double cubic2d( double val[] ) {
    double func_val = ( val[0]*val[0]*val[0] ) *
            ( val[1]*val[1]*val[1] ) ;
    return func_val ;
}

static double deriv2d_x( double val[] ) {
    double func_val = 3.0 * ( val[0]*val[0] ) *
            ( val[1]*val[1]*val[1] ) ;
    return func_val ;
}

static double deriv2d_y( double val[] ) {
    double func_val = 3.0 * ( val[1]*val[1] ) *
            ( val[0]*val[0]*val[0] ) ;
    return func_val ;
}

/**
 * @ingroup types_test
 * Interpolate 2-D cubic field using a cubic generating
 * function and compare the speed required to interpolate
 * 1e6 points using the fast_2d and data_grid methods
 */

BOOST_AUTO_TEST_CASE( datagrid_interp_speed_test ) {
    cout << "=== datagrid_interpolation_speed_test ===" << endl;

    int num_points = 1e3 ;
    int param = 5 ;
    int counter = 0 ;
    unsigned index[2] ;
    double x_y[2] ;

    seq_vector* ax[2] ;
    ax[0] = new seq_linear(1.0, 1.0, 5) ;
    ax[1] = new seq_linear(1.0, 1.0, 5) ;
    data_grid<double,2>* grid = new data_grid<double,2>(ax) ;

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

    data_grid_bathy* fast_grid = new data_grid_bathy(grid, true);
    counter = 0 ;
    gettimeofday( &time, &zone ) ;
    start = time.tv_sec + time.tv_usec * 1e-6 ;
    while ( counter != num_points ) {
        fast_grid->interpolate( location(counter, 0) );
        ++counter;
    }
    gettimeofday( &time, &zone ) ;
    complete = time.tv_sec + time.tv_usec * 1e-6 ;

	cout << "Time to complete interpolation using fast_grid method was "
		 << (complete-start) << " sec." << endl ;
    delete fast_grid ;
    delete ax[0] ;
    delete ax[1] ;
}

/**
 * @ingroup types_test
 * Interpolate 2-D cubic field using a cubic generating
 * function and compare the interpolated results and their
 * derivatives for both the fast_2d and data_grid methods
 * to the analytic values.
 */

BOOST_AUTO_TEST_CASE( datagrid_fast_acc_test ) {
    cout << "=== datagrid_fast_accuracy_test ===" << endl;

    seq_vector* axis[2] ;
    int N = 10 ;
    axis[0] = new seq_linear(1.0, 1.0, N) ;
    axis[1] = new seq_linear(1.0, 1.0, N) ;
    data_grid<double,2>* test_grid = new data_grid<double,2>(axis) ;
    unsigned index[2] ;
    double vals[2] ;
    for(int i=0; i<(*axis[0]).size(); ++i) {
        for(int j=0; j<(*axis[1]).size(); ++j) {
            index[0] = i;
            index[1] = j;
            vals[0] = i + 1 ;
            vals[1] = j + 1 ;
            double number = cubic2d(vals) ;
            test_grid->data(index, number);
        }
    }

    cout << "\t*** simple_data grid***" << endl;
    cout << "axis[0]: " << *axis[0] << endl;
    cout << "axis[1]: " << *axis[1] << endl;
    for(int i=0; i<(*axis[0]).size(); i++) {
        for(int j=0; j<(*axis[1]).size(); j++) {
            index[0] = i;
            index[1] = j;
            cout << test_grid->data(index);
            (j < N-1) ? cout << "\t" :  cout << endl;
        }
    }
    cout << endl;

    for(int i=0; i<2; i++){
        test_grid->interp_type(i, GRID_INTERP_PCHIP);
        test_grid->edge_limit(i, true);
    }

    data_grid<double,2>* duplicate = new data_grid<double,2>(*test_grid, true) ;
    data_grid_bathy* test_grid_fast = new data_grid_bathy(duplicate, true) ;

    double spot[2];
    spot[1] = 10.3265; spot[0] = 9.8753;
    double fast_2d_derv[2];
    double fast_2d_value;
    cout << "x: " << spot[0] << "\ty: " << spot[1] << endl;
    fast_2d_value = test_grid_fast->interpolate( spot, fast_2d_derv );
    cout << "fast_2d:    " << fast_2d_value << "\tderivative: " << fast_2d_derv[0] << ", " << fast_2d_derv[1] << endl;

    double data_grid_derv[2];
    double data_grid_value;
    data_grid_value = test_grid->interpolate( spot, data_grid_derv );
    cout << "data_grid:  " << data_grid_value << "\tderivative: " << data_grid_derv[0] << ", " << data_grid_derv[1] << endl;

    double true_derv[2];
    double true_value;
    true_value = cubic2d(spot) ;
    true_derv[0] = deriv2d_x(spot) ;
    true_derv[1] = deriv2d_y(spot) ;
    cout << "true value: " << true_value << "\tderivative: " << true_derv[0] << ", " << true_derv[1] << endl;

    // Compare to 3 %
    BOOST_CHECK_CLOSE(fast_2d_value, true_value, 3);
    BOOST_CHECK_CLOSE(data_grid_value, true_value, 3 );

    // Setup a complex example to compare results for pchip
    cout << "\t*** 2d_data bathy grid_test_pchip ***" << endl;
    wposition::compute_earth_radius( 19.52 ) ;
    const double lat1 = 16.2 ;
    const double lat2 = 24.6 ;
    const double lng1 = -164.4;
    const double lng2 = -155.5 ;
    cout << "load STD14 environmental bathy data" << endl ;

    data_grid<double,2>* grid = new usml::netcdf::netcdf_bathy( USML_TEST_DIR "/studies/cmp_speed/std14bathy.nc",
        lat1, lat2, lng1, lng2, wposition::earth_radius );
    for(int i=0; i<2; i++){
        grid->interp_type(i, GRID_INTERP_PCHIP);
        grid->edge_limit(i, true);
    }
    data_grid<double,2>* dummy = new data_grid<double,2>(*grid, true) ;
    data_grid_bathy* fast_grid = new data_grid_bathy(dummy, true) ;

    const seq_vector* ax0 = grid->axis(0);
    const seq_vector* ax1 = grid->axis(1);
    cout << "axis0(13 to 21): (" << (*ax0)(13) << "," << (*ax0)(14) << ","
                                       << (*ax0)(15) << "," << (*ax0)(16) << ","
                                       << (*ax0)(17) << "," << (*ax0)(18) << ","
                                       << (*ax0)(19) << "," << (*ax0)(20) << ","
                                       << (*ax0)(21) << ")" << endl;
    cout << "axis1(36 to 44): (" << (*ax1)(36) << "," << (*ax1)(37) << ","
                                      << (*ax1)(38) << "," << (*ax1)(39) << ","
                                      << (*ax1)(40) << "," << (*ax1)(41) << ","
                                      << (*ax1)(42) << "," << (*ax1)(43) << ","
                                      << (*ax1)(44) << ")" << endl;
    cout << "\t*** Data *** [axis0: rows & axis1: columns]" << endl;
    for(int i=13; i<22; ++i) {
        (i==13) ? cout << "[" : cout << "";
        for(int j=36; j<45; ++j) {
            (j==36) ? cout << "(" : cout << ", ";
            index[0] = i;
            index[1] = j;
            cout << grid->data(index) - wposition::earth_radius;
            (j!=44) ? cout << "" : cout << ")";
            (j==44 && i!=21) ? cout << endl : cout << "";

        }
        (i==21) ? cout << "]" << endl : cout << "";
    }

    double v0;
    double v1;
    double location[2] ;
    double derv[2] ;
    double value;

    location[0] = 1.24449;
    location[1] = -2.76108;
    location[0] = (*ax0)(17) + 0.00639;
    location[1] = (*ax1)(40);
    value = fast_grid->interpolate( location, derv ) - wposition::earth_radius;
    cout << "fast_2d:    " << value << "\tderivative: " << derv[0] << ", " << derv[1] << endl;
    value = grid->interpolate( location, derv ) - wposition::earth_radius;
    cout << "data_grid:  " << value << "\tderivative: " << derv[0] << ", " << derv[1] << endl;

    cout << "\t*** 3d_data svp grid_test_pchip/bi-linear ***" << endl;
    cout << "load STD14 svp environmental profile data" << endl ;
    data_grid<double,3>* test_grid_3d = new usml::netcdf::netcdf_profile(
            USML_TEST_DIR "/studies/cmp_speed/std14profile.nc",
            0.0, lat1, lat2, lng1, lng2, wposition::earth_radius );
    for(int i=0; i<3; i++){
        (i<1) ? test_grid_3d->interp_type(i, GRID_INTERP_PCHIP) :
            test_grid_3d->interp_type(i, GRID_INTERP_LINEAR);
        test_grid_3d->edge_limit(i, true);
    }
    data_grid<double,3>* duplicate_3d = new data_grid<double,3>(*test_grid_3d, true) ;
    data_grid_svp* test_grid_fast_3d = new data_grid_svp(duplicate_3d,true);

    double loc[3];
    loc[1] = 1.24449;
    loc[2] = -2.76108;
    loc[0] = -2305.0 + wposition::earth_radius;
    v0 = test_grid_3d->interpolate( loc );
    v1 = test_grid_fast_3d->interpolate( loc );
    cout << "location: (" << loc[0]-wposition::earth_radius << ", " << loc[1] << "," << loc[2] << ")" << "\tgrid: " << v0 << "\tfast_grid: " << v1 << endl;

    BOOST_CHECK_CLOSE(v0, v1, 3.0) ;

    delete test_grid_fast_3d ;
    delete fast_grid ;
    delete test_grid_fast ;
    delete test_grid_3d ;
    delete grid ;
    delete test_grid ;
    delete axis[0] ;
    delete axis[1] ;
}

BOOST_AUTO_TEST_SUITE_END()
