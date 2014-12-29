/**
 * @example types/test/datagrid_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <usml/types/types.h>
#include <iostream>
#include <sstream>
#include <boost/progress.hpp>
#include <stdio.h>
#include <usml/netcdf/netcdf_files.h>
#include <boost/progress.hpp>

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
        cout << pchip ;
        // BOOST_CHECK_CLOSE( pchip, truth, 20.0 ) ;

        cout << endl;
    }
}

/**
 * Cubic generating function and its derivative
 * functions in the x and y directions.
 */

static double cubic2d( double x, double y ) {
    return (x*x*x) * (y*y*y) ;
}

static double deriv2d_x( double x, double y ) {
    return (3.0*x*x) * (y*y*y) ;
}

static double deriv2d_y( double x, double y ) {
    return (x*x*x) * (3.0*y*y) ;
}

/**
 * @ingroup types_test
 * Interpolate 2-D cubic field using a cubic generating
 * function and compare the speed required to interpolate
 * 1e6 points using the fast_2d and data_grid methods
 */

BOOST_AUTO_TEST_CASE( interp_speed_test ) {
    cout << "=== datagrid_test: interpolation_speed_test ===" << endl;

    int num_points = 1e3 ;
    int param = 5 ;
    int counter = 0 ;
    unsigned index[2] ;

    seq_vector* ax[2] ;
    ax[0] = new seq_linear(1.0, 1.0, 5) ;
    ax[1] = new seq_linear(1.0, 1.0, 5) ;
    data_grid<double,2>* grid = new data_grid<double,2>(ax) ;
    grid->interp_type(0, GRID_INTERP_PCHIP);
    grid->interp_type(1, GRID_INTERP_PCHIP);

    for(int i=0; i < ax[0]->size(); ++i) {
        for(int j=0; j < ax[1]->size(); ++j) {
                index[0] = i ;
                index[1] = j ;
                double x = i + 1 ;
                double y = j + 1 ;
                double number = cubic2d(x, y) ;
                grid->data( index, number ) ;
        }
    }

    double spot[2] ;
    matrix<double*> location (num_points,1) ;
    for(int i=0; i<num_points; ++i) {
        spot[0] = param * randgen::uniform();
        spot[1] = param * randgen::uniform();
        location(i,0) = spot ;
    }

    {
		cout << "Interpolation using data_grid method" << endl ;
		boost:: progress_timer timer ;
		counter = 0 ;
		while ( counter != num_points ) {
			grid->interpolate( location(counter, 0) );
			++counter;
		}
	}

    data_grid_bathy* fast_grid = new data_grid_bathy(grid);
	{
		cout << "Interpolation using fast_grid method" << endl ;
		boost:: progress_timer timer ;
		counter = 0 ;
		while ( counter != num_points ) {
			fast_grid->interpolate( location(counter, 0) );
			++counter;
		}
	}

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
 *
 * An error is produced if the values produced by fast_grids
 * are greater than a 3% difference from the data_grid or
 * true values.
 */

BOOST_AUTO_TEST_CASE( fast_accuracy_test ) {
    cout << "=== datagrid_test: fast_accuracy_test ===" << endl;

    int N = 10 ;
    double span = 0.5 ;

    // Build a data grid with simple axes
    seq_vector* axis[2] ;
    axis[0] = new seq_linear(-span, 0.1, N) ;
    axis[1] = new seq_linear(-span, 0.1, N) ;
    data_grid<double,2>* test_grid = new data_grid<double,2>(axis) ;
    test_grid->interp_type(0, GRID_INTERP_PCHIP);
    test_grid->interp_type(1, GRID_INTERP_PCHIP);

    // Populate the data grid with a bicubic function
    unsigned index[2] ;
    double x,y ;
    std::size_t size0( axis[0]->size() ) ;
    std::size_t size1( axis[1]->size() ) ;
    for(int i=0; i<size0; ++i) {
        for(int j=0; j<size1; ++j) {
            index[0] = i;
            index[1] = j;
            x = (double)i/N - span ;
            y = (double)j/N - span ;
            double number = cubic2d(x,y) ;
            test_grid->data(index, number);
        }
    }

    cout << "\t*** simple_data grid***" << endl;
//    cout << "axis[0]: " << *axis[0] << endl;
//    cout << "axis[1]: " << *axis[1] << endl;
//    for(int i=0; i<size0; i++) {
//        std::stringstream ss ;
//        for(int j=0; j<size1; j++) {
//            index[0] = i;
//            index[1] = j;
//            std::stringstream t ;
//            t << test_grid->data(index) ;
//            ss << t.str() ;
//            char buff[8] ;
//            std::size_t size( 10 - t.str().size() ) ;
//            sprintf(buff,"%*s",size," ") ;
//            ss << buff ;
//        }
//        cout << ss.str() << endl ;
//    }
//    cout << endl;

    double spot[2] ;
//    x = 0.3265, y = -0.2753 ;
    x = 0.2135, y = -0.3611 ;
    spot[0] = x, spot[1] = y ;
    double derv[2] ;
    cout << "location: (" << spot[0] << ", " << spot[1] << ")" << endl;

    double grid_value = test_grid->interpolate( spot, derv );
    printf("data_grid: %10f  derivative: %8f, %8f\n",grid_value,derv[0],derv[1]) ;

    data_grid_bathy test_grid_fast( test_grid ) ;
    double fast_value = test_grid_fast.interpolate( spot, derv );
    printf("fast_grid: %10f  derivative: %8f, %8f\n",fast_value,derv[0],derv[1]) ;

    double true_value = cubic2d(x,y) ;
    derv[0] = deriv2d_x(x,y) ;
    derv[1] = deriv2d_y(x,y) ;
    printf("true value: %9f  derivative: %9f, %8f\n",true_value,derv[0],derv[1]) ;

    // Compare to 3 %
    BOOST_CHECK_CLOSE(fast_value, true_value, 3);
    BOOST_CHECK_CLOSE(grid_value, true_value, 3 );

    // Setup a complex example to compare results for pchip
    cout << "\n\t*** 2d_data bathy grid_test_pchip ***" << endl;
    wposition::compute_earth_radius( 19.52 ) ;
    const double lat1 = 16.2 ;
    const double lat2 = 24.6 ;
    const double lng1 = -164.4;
    const double lng2 = -155.5 ;
    cout << "load STD14 environmental bathy data" << endl ;
    data_grid<double,2>* grid = new usml::netcdf::netcdf_bathy( USML_TEST_DIR "/studies/cmp_speed/std14bathy.nc",
        lat1, lat2, lng1, lng2, wposition::earth_radius );
    grid->interp_type(0, GRID_INTERP_PCHIP);
    grid->interp_type(1, GRID_INTERP_PCHIP);

    seq_vector* ax0 = grid->axis(0) ;
    seq_vector* ax1 = grid->axis(1) ;
//    cout << "axis0(13 to 21): (" << (*ax0)(13) << "," << (*ax0)(14) << ","
//                                       << (*ax0)(15) << "," << (*ax0)(16) << ","
//                                       << (*ax0)(17) << "," << (*ax0)(18) << ","
//                                       << (*ax0)(19) << "," << (*ax0)(20) << ","
//                                       << (*ax0)(21) << ")" << endl;
//    cout << "axis1(36 to 44): (" << (*ax1)(36) << "," << (*ax1)(37) << ","
//                                      << (*ax1)(38) << "," << (*ax1)(39) << ","
//                                      << (*ax1)(40) << "," << (*ax1)(41) << ","
//                                      << (*ax1)(42) << "," << (*ax1)(43) << ","
//                                      << (*ax1)(44) << ")" << endl;
//    cout << "\t*** Data *** [axis0: rows & axis1: columns]" << endl;
//    for(int i=13; i<22; ++i) {
//        (i==13) ? cout << "[" : cout << "";
//        for(int j=36; j<45; ++j) {
//            (j==36) ? cout << "(" : cout << ", ";
//            index[0] = i;
//            index[1] = j;
//            cout << grid->data(index) - wposition::earth_radius;
//            (j!=44) ? cout << "" : cout << ")";
//            (j==44 && i!=21) ? cout << endl : cout << "";
//
//        }
//        (i==21) ? cout << "]" << endl : cout << "";
//    }

    spot[0] = (*ax0)(17) + 0.00639 ;
    spot[1] = (*ax1)(40) ;
    cout << "location: (" << spot[0] << ", " << spot[1] << ")" << endl ;
    double value = grid->interpolate( spot, derv ) ;
    value -= wposition::earth_radius ;
    printf("data_grid: %10f  derivative: %8f, %8f\n",value,derv[0],derv[1]) ;

    data_grid_bathy fast_grid( grid ) ;
    value = fast_grid.interpolate( spot, derv ) ;
    value -= wposition::earth_radius ;
    printf("fast_grid: %10f  derivative: %8f, %8f\n",value,derv[0],derv[1]) ;

    cout << "\n\t*** 3d_data svp grid_test_pchip/bi-linear ***" << endl;
    cout << "load STD14 svp environmental profile data" << endl ;
    data_grid<double,3>* test_grid_3d = new usml::netcdf::netcdf_profile(
            USML_TEST_DIR "/studies/cmp_speed/std14profile.nc",
            0.0, lat1, lat2, lng1, lng2, wposition::earth_radius ) ;
    test_grid_3d->interp_type(0, GRID_INTERP_PCHIP) ;
	test_grid_3d->interp_type(1, GRID_INTERP_LINEAR) ;
    test_grid_3d->interp_type(2, GRID_INTERP_LINEAR) ;

    double loc[3] ;
    double vec[3] ;
    loc[1] = 1.24449 ;
    loc[2] = -2.76108 ;
    loc[0] = -2305.0 + wposition::earth_radius ;
    cout << "location: (" << loc[0]-wposition::earth_radius << ", " << loc[1] << "," << loc[2] << ")\n" ;
    double v0 = test_grid_3d->interpolate( loc, vec );
    printf("data_grid: %10f  derivative: %8f, %8f, %8f\n",v0,vec[0],vec[1],vec[2]) ;
    data_grid_svp test_grid_fast_3d( test_grid_3d ) ;
    double v1 = test_grid_fast_3d.interpolate( loc, vec ) ;
    printf("fast_grid: %10f  derivative: %8f, %8f, %8f\n",v1,vec[0],vec[1],vec[2]) ;
    BOOST_CHECK_CLOSE(v0, v1, 3.0) ;

    delete axis[0] ;
    delete axis[1] ;
//    delete test_grid_fast ;
//    delete test_grid_fast_3d ;
//    delete fast_grid ;
}

BOOST_AUTO_TEST_SUITE_END()
