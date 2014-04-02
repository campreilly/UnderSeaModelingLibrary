/**
 * @example ocean/test/reflect_loss_netcdf_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <usml/ocean/ocean.h>
#include <usml/ocean/reflect_loss_netcdf.h>

BOOST_AUTO_TEST_SUITE(reflect_loss_netcdf_test)

using namespace boost::unit_test ;
using namespace usml::ocean ;

/**
 * @ingroup ocean_test
 * @{
 */

/**
 * Test the basic features of the reflection loss model using
 * the netCDF bottom type file.
 * Generate errors if values differ by more that 1E-5 percent.
 */
BOOST_AUTO_TEST_CASE( reflect_loss_netcdf_test ) {
	cout << " === reflection_loss_test: reflection_loss_netcdf bottom type file === " << endl;
	reflect_loss_netcdf netcdf( USML_DATA_DIR "/bottom_province/sediment_test.nc" );

    seq_linear frequency(1000.0, 1000.0, 0.01) ;
    double angle = M_PI_2 ;
    vector<double> amplitude( frequency.size() ) ;

    double limestone = 3.672875 ;
    double sand = 10.166660 ;
    double tolerance = 4e-4 ;

	/** bottom type numbers in the center of the data field top left, right, bottom left, right */
	netcdf.reflect_loss(wposition1(29.5, -83.4), frequency, angle, &amplitude) ;
	BOOST_CHECK_CLOSE(amplitude(0), limestone, tolerance) ;
	netcdf.reflect_loss(wposition1(30.5, -83.4), frequency, angle, &amplitude) ;
	BOOST_CHECK_CLOSE(amplitude(0), sand, tolerance) ;
	netcdf.reflect_loss(wposition1(29.5, -84.2), frequency, angle, &amplitude) ;
	BOOST_CHECK_CLOSE(amplitude(0), sand, tolerance) ;
	netcdf.reflect_loss(wposition1(30.5, -84.2), frequency, angle, &amplitude) ;
	BOOST_CHECK_CLOSE(amplitude(0), limestone, tolerance) ;
	/** bottom type numbers at the corners of the data field top left, bottom left, top right, bottom right */
	netcdf.reflect_loss(wposition1(26, -80), frequency, angle, &amplitude) ;
	BOOST_CHECK_CLOSE(amplitude(0), sand, tolerance) ;
	netcdf.reflect_loss(wposition1(26, -89), frequency, angle, &amplitude) ;
	BOOST_CHECK_CLOSE(amplitude(0), limestone, tolerance) ;
	netcdf.reflect_loss(wposition1(35, -80), frequency, angle, &amplitude) ;
	BOOST_CHECK_CLOSE(amplitude(0), sand, tolerance) ;
	netcdf.reflect_loss(wposition1(35, -89), frequency, angle, &amplitude) ;
	BOOST_CHECK_CLOSE(amplitude(0), limestone, tolerance) ;
}

BOOST_AUTO_TEST_SUITE_END()
