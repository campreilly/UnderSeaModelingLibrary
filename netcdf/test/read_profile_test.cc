/**
 * @example netcdf/test/read_bathy_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <usml/netcdf/netcdf_files.h>
#include <iostream>
#include <fstream>

BOOST_AUTO_TEST_SUITE(read_profile_test)

using namespace boost::unit_test ;
using namespace usml::netcdf ;

/**
 * @ingroup netcdf_test
 * @{
 */

/**
 * Tests the ability of the netcdf_woa class to splice deep and shallow
 * databases into a combined product. Extract Hawaii ocean temperature and
 * dumps the resulting data to the read_woa.csv file.
 *
 * Automatically compares results for a handfull of individual location
 * to data that was extracted by hand from the woacmp.log file created
 * by the Makefile. Generates BOOST errors if these values differ by
 * more that 1E-6 percent.
 */
BOOST_AUTO_TEST_CASE( read_woa ) {
    cout << "=== profile_test: read_woa ===" << endl;
    int month = 6 ;
    const double earth_radius = 6378137.0 ;
    netcdf_woa profile(
	USML_DATA_DIR "/woa09/temperature_seasonal_1deg.nc",
	USML_DATA_DIR "/woa09/temperature_monthly_1deg.nc",
        month, 18.5, 22.5, 200.5, 205.5, earth_radius ) ;

    // compare depth axis to values read using ncdump

    const seq_vector& altitude = *(profile.axis(0)) ;
    unsigned num_alt = altitude.size() ;
    cout << "altitude[" << num_alt << "] =" ;
    for ( unsigned n=0 ; n < num_alt ; ++n ) {
        cout << " " << ( altitude(n) - earth_radius ) ;
    }
    cout << endl ;
    BOOST_CHECK_EQUAL( num_alt, 33 ) ;
    BOOST_CHECK_CLOSE( altitude(0)-earth_radius, 0.0, 1e-6 ) ;
    BOOST_CHECK_CLOSE( altitude(num_alt-1)-earth_radius, -5500.0, 1e-6 ) ;

    // compare latitude axis to values read using ncdump

    const seq_vector& latitude = *(profile.axis(1)) ;
    unsigned num_lat = latitude.size() ;
    double lat1 = to_latitude( latitude(0) ) ;
    double lat2 = to_latitude( latitude(num_lat-1) ) ;
    double inc_lat = to_degrees( latitude.increment(0) ) ;
    cout << "latitude[" << num_lat
         << "] = " << lat1 << " to " << lat2 << " by " << inc_lat << endl ;
    BOOST_CHECK_EQUAL( num_lat, 5 ) ;
    BOOST_CHECK_CLOSE( lat1, 18.5, 1e-6 ) ;
    BOOST_CHECK_CLOSE( lat2, 22.5, 1e-6 ) ;

    // compare longitude axis to values read using ncdump

    const seq_vector& longitude = *(profile.axis(2)) ;
    unsigned num_lng = longitude.size() ;
    double lng1 = to_degrees( longitude(0) ) ;
    double lng2 = to_degrees( longitude(num_lng-1) ) ;
    double inc_lng = to_degrees( longitude.increment(0) ) ;
    cout << "longitude[" << num_lng
         << "] = " << lng1 << " to " << lng2 << " by " << inc_lng << endl ;
    BOOST_CHECK_EQUAL( num_lng, 6 ) ;
    BOOST_CHECK_CLOSE( lng1, 200.5, 1e-6 ) ;
    BOOST_CHECK_CLOSE( lng2, 205.5, 1e-6 ) ;

    // compare depth to some known values

    unsigned index[3] ;
    index[0]=0 ; index[1]=0 ; index[2]=0 ;  // 1st depth, 1st lat, 1st long
    BOOST_CHECK_CLOSE( profile.data(index), 25.8543f, 1e-6f ) ;
    index[0]=0 ; index[1]=0 ; index[2]=1 ;  // 1st depth, 1st lat, 2nd long
    BOOST_CHECK_CLOSE( profile.data(index), 25.7594f, 1e-6f ) ;
    index[0]=0 ; index[1]=1 ; index[2]=0 ;  // 1st depth, 2nd lat, 1st long
    BOOST_CHECK_CLOSE( profile.data(index), 25.8541f, 1e-6f ) ;
    index[0]=1 ; index[1]=0 ; index[2]=0 ;  // 2nd depth, 1st lat, 1st long
    BOOST_CHECK_CLOSE( profile.data(index), 25.8575f, 1e-6f ) ;

    // dump CSV file for processing by graphics program
    // each row is a depth, cycle through all longitudes
    // before moving to next latitude

    std::ofstream ofile( USML_TEST_DIR "/netcdf/test/read_woa.csv" ) ;

    // print header row
    ofile << "depth" ;
    for ( unsigned n=0 ; n < num_lat ; ++n ) {
        for ( unsigned m=0 ; m < num_lng ; ++m ) {
            ofile << ",\"" << to_latitude( latitude(n) ) << "N "
                  << to_degrees( longitude(m) ) << "E\"" ;
        }
    }
    ofile << std::endl ;

    // print data
    for ( unsigned d=0 ; d < num_alt ; ++d ) {
        index[0] = d ;
        ofile << ( altitude(d) - earth_radius ) ;
        for ( unsigned n=0 ; n < num_lat ; ++n ) {
            index[1] = n ;
            for ( unsigned m=0 ; m < num_lng ; ++m ) {
                index[2] = m ;
                ofile << "," << profile.data(index) ;
            }
        }
        ofile << std::endl ;
    }
    cout << "does boost::math::isnan work? "
         << ( ( (boost::math::isnan(NAN)) ) ? "yes" : "no" )
         << endl ;
}

/**
 * Tests the ability of the netcdf_profile class to span a longitude
 * cut point in the database.  To test this, it reads data from WOA09
 * across the area from -1E (aka 1W) to 1E.  It relies on the fact that
 * WOA09 defines its data in terms of longitudes from 0 to 360.
 * Dumps the resulting profiles to the span_profile.csv file.
 *
 * Automatically compares results for a handfull of individual location
 * to data that was extracted by hand from the woacmp2.log file created
 * by the Makefile. Generates BOOST errors if these values differ by
 * more that 1E-6 percent.
 */
BOOST_AUTO_TEST_CASE( span_profile ) {
    cout << "=== profile_test: span_profile ===" << endl;
    const double earth_radius = 0.0 ;
    netcdf_profile profile(
	USML_DATA_DIR "/woa09/temperature_monthly_1deg.nc",
	15.0, -1.0, 2.0, -1.0, 2.0, earth_radius ) ;

    // compare depth axis to values read using ncdump

    const seq_vector& altitude = *(profile.axis(0)) ;
    unsigned num_alt = altitude.size() ;
    cout << "altitude = " << altitude << endl ;
    BOOST_CHECK_EQUAL( num_alt, 24 ) ;
    BOOST_CHECK_CLOSE( altitude(0)-earth_radius, 0.0, 1e-6 ) ;
    BOOST_CHECK_CLOSE( altitude(num_alt-1)-earth_radius, -1500.0, 1e-6 ) ;

    // compare latitude axis to values read using ncdump

    const seq_vector& latitude = *(profile.axis(1)) ;
    unsigned num_lat = latitude.size() ;
    double lat1 = to_latitude( latitude(0) ) ;
    double lat2 = to_latitude( latitude(num_lat-1) ) ;
    double inc_lat = -to_degrees( latitude.increment(0) ) ;
    cout << "latitude[" << num_lat
         << "] = " << lat1 << " to " << lat2 << " by " << inc_lat << endl ;
    BOOST_CHECK_EQUAL( num_lat, 5 ) ;
    BOOST_CHECK_CLOSE( lat1, -1.5, 1e-6 ) ;
    BOOST_CHECK_CLOSE( lat2, 2.5, 1e-6 ) ;

    // compare longitude axis to values read using ncdump

    const seq_vector& longitude = *(profile.axis(2)) ;
    unsigned num_lng = longitude.size() ;
    double lng1 = to_degrees( longitude(0) ) ;
    double lng2 = to_degrees( longitude(num_lng-1) ) ;
    double inc_lng = to_degrees( longitude.increment(0) ) ;
    cout << "longitude[" << num_lng
         << "] = " << lng1 << " to " << lng2 << " by " << inc_lng << endl ;
    BOOST_CHECK_EQUAL( num_lng, 5 ) ;
    BOOST_CHECK_CLOSE( lng1, -1.5, 1e-6 ) ;
    BOOST_CHECK_CLOSE( lng2, 2.5, 1e-6 ) ;

    // compare temperature to some known values

    unsigned index[3] ;
    index[0]=0 ; index[1]=2 ; index[2]=2 ;  // zero depth at 0.5N 0.5E
    BOOST_CHECK_CLOSE( profile.data(index), 28.2047f, 1e-6f ) ;
    index[0]=23 ; index[1]=2 ; index[2]=2 ; // max depth at 0.5N 0.5E
    BOOST_CHECK_CLOSE( profile.data(index), 4.1774f, 1e-6f ) ;

    // dump CSV file for processing by graphics program
    // each row is a depth, cycle through all longitudes
    // before moving to next latitude

    std::ofstream ofile( USML_TEST_DIR "/netcdf/span_profile.csv" ) ;

    // print header row
    ofile << "depth" ;
    for ( unsigned n=0 ; n < num_lat ; ++n ) {
        for ( unsigned m=0 ; m < num_lng ; ++m ) {
            ofile << ",\"" << to_latitude( latitude(n) ) << "N "
                  << to_degrees( longitude(m) ) << "E\"" ;
        }
    }
    ofile << std::endl ;

    // print data
    for ( unsigned d=0 ; d < num_alt ; ++d ) {
        index[0] = d ;
        ofile << ( altitude(d) - earth_radius ) ;
        for ( unsigned n=0 ; n < num_lat ; ++n ) {
            index[1] = n ;
            for ( unsigned m=0 ; m < num_lng ; ++m ) {
                index[2] = m ;
                ofile << "," << profile.data(index) ;
            }
        }
        ofile << std::endl ;
    }
}

/// @}

BOOST_AUTO_TEST_SUITE_END()

