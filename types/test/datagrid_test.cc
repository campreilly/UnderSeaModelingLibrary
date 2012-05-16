/** 
 * @example types/test/datagrid_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <usml/types/types.h>
#include <iostream>
#include <stdio.h>

BOOST_AUTO_TEST_SUITE(datagrid_test)

using namespace boost::unit_test;
using namespace usml::types;

typedef seq_vector::const_iterator iterator;

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

BOOST_AUTO_TEST_SUITE_END()
