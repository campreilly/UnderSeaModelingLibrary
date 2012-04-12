/**
 * @example ocean/test/boundary_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <usml/netcdf/netcdf_files.h>
#include <usml/ocean/ocean.h>

BOOST_AUTO_TEST_SUITE(boundary_test)

using namespace boost::unit_test;
using namespace usml::netcdf;
using namespace usml::ocean;

/**
 * @ingroup ocean_test
 * @{
 */

/**
 * Test the basic features of the boundary model using
 * the boundary_flat model.
 * Generate errors if values differ by more that 1E-6 percent.
 */
BOOST_AUTO_TEST_CASE( flat_boundary_test ) {
    cout << "=== boundary_test: flat_boundary_test ===" << endl;

    // simple values for points and depth

    wposition1 points;
    double depth;
    wvector1 normal;

    // compute profile

    boundary_flat model(1000.0);
    model.height(points, &depth, &normal);
    cout << "depth:        " << (wposition::earth_radius - depth) << endl;
    cout << "normal rho:   " << normal.rho() << endl;
    cout << "normal theta: " << normal.theta() << endl;
    cout << "normal phi:   " << normal.phi() << endl;

    // check the answer

    BOOST_CHECK_CLOSE(wposition::earth_radius - depth, 1000.0, 1e-6);
    BOOST_CHECK_CLOSE(normal.rho(), 1.0, 1e-6);
    BOOST_CHECK_CLOSE(normal.theta(), 0.0, 1e-6);
    BOOST_CHECK_CLOSE(normal.phi(), 0.0, 1e-6);
}

/**
 * Find the bottom slope that causes the bathymetry to break the ocean surface
 * after traveling 1 deg north of the equator.  The analytic result is
 * about 0.5 degrees.  Intuitively, a value equal to about half the traveled
 * distance represents the combined effect of the bottom reaching up to the
 * surface, and the surface bending down to the reach the bottom.
 * Generate errors if values differ by more that 1E-6 percent.
 */
BOOST_AUTO_TEST_CASE( sloped_boundary_test ) {
    cout << "=== boundary_test: sloped_boundary_test ===" << endl;

    wposition::compute_earth_radius(0.0);
    const double dlat = 1.0;
    const double d0 = 1000.0;
    const double slope = d0 / (to_radians(dlat)
            * (wposition::earth_radius - d0));
    const double alpha = atan(slope);

    // simple values for points and depth

    wposition1 reference;
    wposition1 points;
    points.latitude(dlat); // define field point 60 nmi north of reference

    double depth;
    wvector1 normal;

    // compute profile

    boundary_slope model(reference, d0, alpha);
    model.height(points, &depth, &normal);
    cout << "slope:        " << slope << endl;
    cout << "alpha:        " << to_degrees(alpha) << endl;
    cout << "depth:        " << wposition::earth_radius - depth << endl;
    cout << "normal rho:   " << normal.rho() << endl;
    cout << "normal theta: " << normal.theta() << endl;
    cout << "normal phi:   " << normal.phi() << endl;

    // check the answer

    BOOST_CHECK(abs(wposition::earth_radius - depth) < 0.1);
    BOOST_CHECK_CLOSE(normal.theta(), sin(alpha), 1e-6);
    BOOST_CHECK_CLOSE(normal.phi(), 0.0, 1e-6);
    BOOST_CHECK_CLOSE(normal.rho(), sqrt(1.0 - sin(alpha) * sin(alpha)), 1e-6);
}

/**
 * Extract Malta Escarpment bathymetry from March 2010 version of ETOPO1.
 * Compare results to data extracted from this database by hand.
 *
 * The ETOPO1 database for this area contains the following data:
 * <pre>
 *	netcdf boundary_test {
 *	dimensions:
 *		x = 9 ;
 *		y = 13 ;
 *	variables:
 *		double x(x) ;
 *			x:long_name = "Longitude" ;
 *			x:actual_range = -180., 180. ;
 *			x:units = "degrees" ;
 *		double y(y) ;
 *			y:long_name = "Latitude" ;
 *			y:actual_range = -90., 90. ;
 *			y:units = "degrees" ;
 *		int z(y, x) ;
 *			z:long_name = "z" ;
 *			z:_FillValue = -2147483648 ;
 *			z:actual_range = -10898., 8271. ;
 *
 *	// global attributes:
 *			:Conventions = "COARDS/CF-1.0" ;
 *			:title = "ETOPO1_Ice_g_gmt4.grd" ;
 *			:GMT_version = "4.4.0" ;
 *			:node_offset = 0 ;
 *			:NCO = "20111109" ;
 *	data:
 *
 *	 x = 15.8666666666667, 15.8833333333333, 15.9, 15.9166666666667,
 *	    15.9333333333333, 15.95, 15.9666666666667, 15.9833333333333, 16 ;
 *
 *	 y = 36, 36.0166666666667, 36.0333333333334, 36.05, 36.0666666666667,
 *	    36.0833333333334, 36.1, 36.1166666666667, 36.1333333333334, 36.15,
 *	    36.1666666666667, 36.1833333333334, 36.2 ;
 *
 *	 z =
 *	  -3616, -3665, -3678, -3703, -3708, -3705, -3718, -3724, -3730,
 *	  -3653, -3672, -3686, -3711, -3705, -3710, -3698, -3708, -3718,
 *	  -3696, -3693, -3701, -3693, -3707, -3700, -3689, -3695, -3706,
 *	  -3674, -3703, -3720, -3709, -3710, -3702, -3695, -3682, -3679,
 *	  -3717, -3720, -3710, -3704, -3701, -3683, -3679, -3675, -3661,
 *	  -3702, -3702, -3691, -3677, -3678, -3685, -3658, -3662, -3654,
 *	  -3661, -3660, -3644, -3651, -3668, -3673, -3655, -3649, -3648,
 *	  -3633, -3630, -3632, -3643, -3653, -3657, -3651, -3640, -3640,
 *	  -3625, -3634, -3626, -3629, -3647, -3643, -3640, -3635, -3630,
 *	  -3615, -3614, -3619, -3628, -3634, -3634, -3637, -3633, -3584,
 *	  -3613, -3616, -3622, -3624, -3626, -3631, -3634, -3615, -3581,
 *	  -3609, -3607, -3619, -3616, -3617, -3634, -3635, -3593, -3574,
 *	  -3593, -3613, -3612, -3623, -3638, -3615, -3594, -3569, -3557 ;
 *	}
 * </pre>
 * Given that boundary_grid is using PCHIP interpolation,
 * the expected results are:
 *
 *	Location:     lat=36.0004 long=15.8904
 *	World Coords: theta=0.94247 phi=0.27734
 *	Depth:        3613.3142451792955
 *	Normal:       theta=-0.0053183248950815359 phi=0.075681405852260769
 *	Slope:        theta=0.303697 phi=-4.32587 deg
 *
 * When the gcc -ffast-math compiler option is turned off, these results are
 * expected to be accurate to at least 1e-6 percent.  With fast-math turned on,
 * the accuracy of the normal drops to 1e-5 percent, and the about 0.15 meters
 * is lost on the accuracy in depth.
 *
 */
BOOST_AUTO_TEST_CASE( etopo_boundary_test ) {
    cout << "=== boundary_test: etopo_boundary_test ===" << endl;
    cout << "reading " << USML_DATA_BATHYMETRY << endl;
    boundary_grid<float, 2> model( new netcdf_bathy(
    	USML_DATA_BATHYMETRY, 36.0, 36.2, 15.85, 16.0, wposition::earth_radius));

    // simple values for points and depth

    wposition1 points;
    points.latitude(36.000447);
    points.longitude(15.890411);
    double depth;
    wvector1 normal;

    // compute bathymetry

    model.height(points, &depth, &normal);

    // check the answer

    cout << "Location:     lat=" << points.latitude() << " long="
            << points.longitude() << endl << "World Coords: theta="
            << points.theta() << " phi=" << points.phi() << endl
            << "Depth:        " << wposition::earth_radius - depth << endl
            << "Normal:       theta=" << normal.theta() << " phi="
            << normal.phi() << endl << "Slope:        theta=" << to_degrees(
            -asin(normal.theta())) << " phi="
            << to_degrees(-asin(normal.phi())) << " deg" << endl;

    #ifdef __FAST_MATH__
        const double depth_accuracy = 0.005 ;
        const double normal_accuracy = 1e-5 ;
    #else
        const double depth_accuracy = 1e-6 ;
        const double normal_accuracy = 1e-6 ;
    #endif
    BOOST_CHECK_CLOSE(wposition::earth_radius - depth, 3670.8142451792955, depth_accuracy );
    BOOST_CHECK_CLOSE( normal.theta(), -0.00015194593993928332, normal_accuracy );
    BOOST_CHECK_CLOSE( normal.phi(), 0.012764947615721318, normal_accuracy );
}

/**
 * Test the extraction of bathymetry data from ASCII files with an ARC header.
 * The test file creates a simple environment with 3 latitudes and 4 longitudes.
 * Testing individual depth points in latitude and longitude ensures the
 * the data is oriented correctly as it is read in.  Errors on the order 3 cm
 * are introduced by the conversion from earth spherical coordinates and back.
 */
BOOST_AUTO_TEST_CASE( ascii_arc_test ) {
    cout << "=== boundary_test: ascii_arc_test ===" << endl;

    ascii_arc_bathy bathy( USML_ASCII_ARC_TEST_DATA ) ;

    BOOST_CHECK_EQUAL( bathy.axis(0)->size(), 3 );
    BOOST_CHECK_EQUAL( bathy.axis(1)->size(), 4 );

    unsigned index[2] ;
    index[0]=0; index[1]=0; BOOST_CHECK_CLOSE(wposition::earth_radius - bathy.data(index), -100.0, 0.1);
    index[0]=1; index[1]=1; BOOST_CHECK_CLOSE(wposition::earth_radius - bathy.data(index), -111.0, 0.1);
    index[0]=2; index[1]=2; BOOST_CHECK_CLOSE(wposition::earth_radius - bathy.data(index), -122.0, 0.1);
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
