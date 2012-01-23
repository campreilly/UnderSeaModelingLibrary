/** 
 * @example types/test/position_test.cc
 */
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <usml/types/types.h>
#include <iostream>

BOOST_AUTO_TEST_SUITE(position_test)

using namespace boost::unit_test;
using namespace usml::types;

/**
 * @ingroup types_test
 * @{
 */

/**
 * Compare the earth's radius at specific latitudes to known values.
 * Generate errors if values differ by more that 1E-6 percent.
 *
 * @xref WGS 84 IMPLEMENTATION MANUAL, Version 2.4, 1998. 
 *       See http://www.dqts.net/wgs84.htm for more information.
 */
BOOST_AUTO_TEST_CASE( earth_radius_test ) {
    cout << "=== position_test: earth_radius_test ===" << endl;

    double radius90 = 6399593.62578493 ;
    double radius45 = 6378101.030201019 ;
    double radius00 = 6356752.314245179 ;

    wposition::compute_earth_radius(90.0);
    cout << "earth_radius at 90.0 latitude: "
         << wposition::earth_radius << endl;
    BOOST_CHECK_CLOSE( wposition::earth_radius, radius90, 1e-6 );

    wposition::compute_earth_radius(45.0);
    cout << "earth_radius at 45.0 latitude: "
         << wposition::earth_radius << endl;
    BOOST_CHECK_CLOSE( wposition::earth_radius, radius45, 1e-6 );

    wposition::compute_earth_radius(0.0);
    cout << "earth_radius at  0.0 latitude: "
         << wposition::earth_radius << endl;
    BOOST_CHECK_CLOSE( wposition::earth_radius, radius00, 1e-6 );
}

/**
 * Compute the dot product between vectors at different latitudes
 * and a vector pointing due north.  If correct, the dots products
 * should correspond to the angles defined by the latitudes of
 * the original points.
 *
 * Generate errors if values differ by more that 1E-6 percent.
 */
BOOST_AUTO_TEST_CASE( dot_test ) {
    cout << "=== position_test: dot_test ===" << endl;

    // define a series of lat/long points on earth

    wposition points(10);
    for ( int n=0; n < 10; ++n ) {
        points.latitude( n, 0, 10.0*n );
        points.longitude( n, 0, 90.0+10.0*n );
    }

    // dot this with vector through the north pole

    wposition1 north( 90.0, 0.0 );
    matrix<double> angle( 10, 1 );
    points.dotnorm( north, angle );
    angle = to_degrees( acos(angle) );

    // check the answer

    for ( int n=0; n < 10; ++n ) {
        double analytic = 90.0 - 10.0 * n ;
        cout << "angle=" << angle(n,0) << " analytic=" << analytic << endl;
        BOOST_CHECK_CLOSE( angle(n,0), analytic, 1e-6 );
    }
}

/**
 * Compute the dot product between vectors at different latitudes
 * and a vector pointing due north.  If correct, the dots products
 * should correspond to the angles defined by the latitudes of
 * the original points.
 *
 * Generate errors if values differ by more that 1E-6 percent.
 */
BOOST_AUTO_TEST_CASE( dot1_test ) {
    cout << "=== position_test: dot1_test ===" << endl;

    // define a series of lat/long points on earth

    wposition1 point ;
    for ( int n=0; n < 10; ++n ) {
        point.latitude( 10.0*n );
        point.longitude( 90.0+10.0*n );

        // dot this with vector through the north pole

        wposition1 north( 90.0, 0.0 );
        double angle = point.dotnorm( north );
        angle = to_degrees( acos(angle) );

        // check the answer

        double analytic = 90.0 - 10.0 * n ;
        cout << "angle=" << angle << " analytic=" << analytic << endl;
        BOOST_CHECK_CLOSE( angle, analytic, 1e-6 );
    }
}

/**
 * Compute the straight line distance between positions at different latitudes
 * and the point 40N 45E. The results are compared to the analytic solution
 * <pre>
 *          distance^2 = 2 R^2 ( 1-cos(40-latitude) )
 * </pre>
 * Generate errors if values differ by more that 1E-6 meters.  The one
 * expection to this accuracy is the case where distance is almost zero.
 * In that case, the calculation accuracy is limited to 0.02 meters.
 */
BOOST_AUTO_TEST_CASE( distance_test ) {
    cout << "=== position_test: distance_test ===" << endl;

    // define a series of lat/long points on earth

    wposition points(10);
    for ( int n=0; n < 10; ++n ) {
        points.latitude( n, 0, 10.0*n );
        points.longitude( n, 0, 45.0 );
    }

    // compute the distance to some point on earth

    wposition1 origin( 40.0, 45.0 );
    matrix<double> distance( 10, 1 );
    points.distance( origin, distance );

    // check the answer
    // distance^2 = 2 R^2 ( 1-cos(angle) )

    for ( int n=0; n < 10; ++n ) {
        double analytic = wposition::earth_radius
            * sqrt( 2.0 * ( 1.0 - cos( to_radians(40.0-10.0*n) ) ) );
        cout << "distance=" << distance(n,0) << " analytic=" << analytic << endl;
        if ( analytic > 1e-6 ) {
            BOOST_CHECK_CLOSE( distance(n,0), analytic, 1e-10 );
        } else {
            BOOST_CHECK_SMALL( distance(n,0)-analytic, 0.1 );
        }
    }
}

/**
 * Compute the straight line distance between positions at different latitudes
 * and the point 40N 45E. The results are compared to the analytic solution
 * <pre>
 *          distance^2 = 2 R^2 ( 1-cos(40-latitude) )
 * </pre>
 * Generate errors if values differ by more that 1E-6 meters.  The one
 * expection to this accuracy is the case where distance is almost zero.
 * In that case, the calculation accuracy is limited to 0.02 meters.
 */
BOOST_AUTO_TEST_CASE( distance1_test ) {
    cout << "=== position_test: distance1_test ===" << endl;

    // define a series of lat/long points on earth

    wposition1 point ;
    for ( int n=0; n < 10; ++n ) {
        point.latitude( 10.0*n );
        point.longitude( 45.0 );

        // compute the distance to some point on earth

        wposition1 origin( 40.0, 45.0 );
        double distance = point.distance( origin );

        // check the answer
        // distance^2 = 2 R^2 ( 1-cos(angle) )

        double analytic = wposition::earth_radius
            * sqrt( 2.0 * ( 1.0 - cos( to_radians(40.0-10.0*n) ) ) );
        cout << "distance=" << distance << " analytic=" << analytic << endl;
        if ( analytic > 1e-6 ) {
            BOOST_CHECK_CLOSE( distance, analytic, 1e-10 );
        } else {
            BOOST_CHECK_SMALL( distance-analytic, 0.1 );
        }
    }
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
