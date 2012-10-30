/**
 * @example ocean/test/reflection_loss_netcdf_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <usml/ocean/ocean.h>
#include <usml/ocean/reflection_loss_netcdf.h>

BOOST_AUTO_TEST_SUITE(reflection_loss_netcdf_test)

using namespace boost::unit_test;
using namespace usml::ocean;

BOOST_AUTO_TEST_CASE( reflection_loss_netcdf_test ) {
	cout << " === reflection_loss_test: reflection_loss_netcdf bottom province file === " << endl;
	reflection_loss_netcdf( USML_DATA_DIR "/bottom_province/sediment_test.nc" );
}

BOOST_AUTO_TEST_SUITE_END()
