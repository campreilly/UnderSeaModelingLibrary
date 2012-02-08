/** 
 * @example netcdf/test/read_profile_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <usml/netcdf/netcdf_files.h>
#include <iostream>
#include <fstream>

BOOST_AUTO_TEST_SUITE(read_bathy_test)

using namespace boost::unit_test ;
using namespace usml::netcdf ;

/**
 * @ingroup netcdf_test
 * @{
 */

/**
 * Extract Hawaii bathymetry from March 2010 version of ETOPO1
 * using the netcdf_bathy class. Dump the resulting bathymetry to the
 * read_etopo.log file.
 * 
 * Automatically compares results for a handfull of individual location
 * to data that was extracted by hand from the etopo_cmp.log file created
 * by the Makefile. Generate BOOST errors if these values differ by
 * more that 1E-6 percent.
 */
BOOST_AUTO_TEST_CASE( read_etopo ) {
    cout << "=== bathy_test: read_etopo ===" << endl;
    cout << "reading " << USML_DATA_BATHYMETRY << endl ;
    netcdf_bathy bathy( USML_DATA_BATHYMETRY, 18.0, 23.0, 200.0, 206.0, 0.0 ) ;
    
    // compare latitude axis to values read using ncdump
    
    const seq_vector& latitude = *(bathy.axis(0)) ;
    int num_lat = latitude.size() ;
    double lat1 = to_latitude( latitude(0) ) ;
    double lat2 = to_latitude( latitude(num_lat-1) ) ;
    double inc_lat = to_degrees( -latitude.increment(0) ) ;
    cout << "latitude[" << num_lat 
         << "] = " << lat1 << " to " << lat2 << " by " << inc_lat << endl ;
    BOOST_CHECK_EQUAL( num_lat, 301 ) ;
    BOOST_CHECK_CLOSE( lat1, 18.0, 1e-6 ) ;
    BOOST_CHECK_CLOSE( lat2, 23.0, 1e-6 ) ;
    
    // compare longitude axis to values read using ncdump
    
    const seq_vector& longitude = *(bathy.axis(1)) ;
    int num_lng = longitude.size() ;
    double lng1 = to_degrees( longitude(0) ) ;
    double lng2 = to_degrees( longitude(num_lng-1) ) ;
    double inc_lng = to_degrees( longitude.increment(0) ) ;
    cout << "longitude[" << num_lng 
         << "] = " << lng1 << " to " << lng2 << " by " << inc_lng << endl ;
    BOOST_CHECK_EQUAL( num_lng, 361 ) ;
    BOOST_CHECK_CLOSE( lng1, 200.0, 1e-6 ) ;
    BOOST_CHECK_CLOSE( lng2, 206.0, 1e-6 ) ;

    // compare depth to some known values
    // extracted by hand from etopo_cmp.log
    
    unsigned index[2] ;
    index[0]=0 ; index[1]=0 ;
    BOOST_CHECK_CLOSE( bathy.data(index), -5262.0f, 1e-6f ) ;
    index[0]=0 ; index[1]=1 ;
    BOOST_CHECK_CLOSE( bathy.data(index), -5272.0f, 1e-6f ) ;
    index[0]=1 ; index[1]=0 ;
    BOOST_CHECK_CLOSE( bathy.data(index), -5249.0f, 1e-6f ) ;
    
    // dump CSV file for processing by graphics program
    
    std::ofstream ofile( "read_etopo.csv" ) ;
    for ( int n=0 ; n < num_lat ; ++n ) {
        index[0] = n ;
        for ( int m=0 ; m < num_lng ; ++m ) {
            index[1] = m ;
            ofile << bathy.data(index) ;
            if ( m < (num_lng-1) ) ofile << "," ;
        }
        ofile << std::endl ;
    }
}

/**
 * Extract Hawaii bathymetry from the etopo_cmp.nc file created
 * in the Makefile using the ncks tool.  Compare results for individual
 * array indices to equivalent data that extracted from this database
 * by hand from the etopo_cmp.log file created by the Makefile.
 * Throw errors if values differ by more that 1E-6 percent.
 * Comparisons of the log files output by each phase (etopo.log and
 * coards.log) should also show no differences.
 *
 * The input file may be missing if the current platform does not 
 * include the ncks utility.  This test quietly exits if 
 * etopo_cmp.nc is missing.
 */
BOOST_AUTO_TEST_CASE( read_coards ) {
    cout << "=== bathy_test: read_coards ===" << endl;
    static const char* filename = "etopo_cmp.nc" ;
	NcFile file(filename) ;
	if ( file.id() < 0 ) {
		cout << filename << " not found, test skipped" << endl ;
		return ;
	}
	cout << "reading " << filename << endl ;
	netcdf_coards<float,2> bathy( file, "z" ) ;
    
	// compare latitude axis to values read using ncdump
    
	const seq_vector& latitude = *(bathy.axis(0)) ;
	int num_lat = latitude.size() ;
	cout << "latitude[" << num_lat << "] = " 
			<< latitude(0) << " to " << latitude(num_lat-1) 
			<< " by " << latitude.increment(0) << endl ;
	BOOST_CHECK_EQUAL( num_lat, 301 ) ;
	BOOST_CHECK_CLOSE( latitude(0), 18.0, 1e-6 ) ;
	BOOST_CHECK_CLOSE( latitude(num_lat-1), 23.0, 1e-6 ) ;
    
	// compare longitude axis to values read using ncdump
    
	const seq_vector& longitude = *(bathy.axis(1)) ;
	int num_lng = longitude.size() ;
	cout << "longitude[" << num_lng << "] = " 
			<< longitude(0) << " to " << longitude(num_lng-1) 
			<< " by " << longitude.increment(0) << endl ;
	BOOST_CHECK_EQUAL( num_lng, 361 ) ;
	BOOST_CHECK_CLOSE( longitude(0), -160.0, 1e-6 ) ;
	BOOST_CHECK_CLOSE( longitude(num_lng-1), -154.0, 1e-6 ) ;

	// compare depth to some known values
	// extracted by hand from etopo_cmp.log
    
	unsigned index[2] ;
	index[0]=0 ; index[1]=0 ;
	BOOST_CHECK_CLOSE( bathy.data(index), -5262.0f, 1e-6f ) ;
	index[0]=0 ; index[1]=1 ;
	BOOST_CHECK_CLOSE( bathy.data(index), -5272.0f, 1e-6f ) ;
	index[0]=1 ; index[1]=0 ;
	BOOST_CHECK_CLOSE( bathy.data(index), -5249.0f, 1e-6f ) ;
    
	// dump CSV file for processing by graphics program
    
	std::ofstream ofile( "read_coards.csv" ) ;
	for ( int n=0 ; n < num_lat ; ++n ) {
		index[0] = n ;
		for ( int m=0 ; m < num_lng ; ++m ) {
			index[1] = m ;
			ofile << bathy.data(index) ;
			if ( m < (num_lng-1) ) ofile << "," ;
		}
		ofile << std::endl ;
	}
}

/**
 * Tests the ability of the netcdf_bathy class to span a longitude
 * cut point in the database.  To test this, it reads data from ETOPO1
 * across the area from 179E to 181E (aka 179W).  This assumes that
 * ETOPO1 defines its data in terms of longitudes from -180 to 180.
 *
 * Dump the resulting bathymetry to the span_bathy.log file.
 */
BOOST_AUTO_TEST_CASE( span_bathy ) {
    cout << "=== bathy_test: span_bathy ===" << endl;
    cout << "reading " << USML_DATA_BATHYMETRY << endl ;
    netcdf_bathy bathy( USML_DATA_BATHYMETRY, -1.0, 2.0, 179, 182, 0.0 ) ;

    // compare latitude axis to values read using ncdump

    const seq_vector& latitude = *(bathy.axis(0)) ;
    int num_lat = latitude.size() ;
    double lat1 = to_latitude( latitude(0) ) ;
    double lat2 = to_latitude( latitude(num_lat-1) ) ;
    double inc_lat = to_degrees( latitude.increment(0) ) ;
    cout << "latitude[" << num_lat
         << "] = " << lat1 << " to " << lat2 << " by " << inc_lat << endl ;
    BOOST_CHECK_EQUAL( num_lat, 181 ) ;
    BOOST_CHECK_CLOSE( lat1, -1.0, 1e-6 ) ;
    BOOST_CHECK_CLOSE( lat2, 2.0, 1e-6 ) ;

    // compare longitude axis to values read using ncdump

    const seq_vector& longitude = *(bathy.axis(1)) ;
    int num_lng = longitude.size() ;
    double lng1 = to_degrees( longitude(0) ) ;
    double lng2 = to_degrees( longitude(num_lng-1) ) ;
    double inc_lng = to_degrees( longitude.increment(0) ) ;
    cout << "longitude[" << num_lng
         << "] = " << lng1 << " to " << lng2 << " by " << inc_lng << endl ;
    BOOST_CHECK_EQUAL( num_lng, 181 ) ;
    BOOST_CHECK_CLOSE( lng1, 179.0, 1e-6 ) ;
    BOOST_CHECK_CLOSE( lng2, 182.0, 1e-6 ) ;

    // compare depth to some known values
    // extracted by hand from etopo_cmp2w.log

    unsigned index[2] ;
    index[0]=0 ; index[1]=0 ;   // 1S 179E = first entry, first row
    BOOST_CHECK_CLOSE( bathy.data(index), -5436.0f, 1e-6f ) ;
    index[0]=0 ; index[1]=60 ;  // 1S 180E = last entry, first row
    BOOST_CHECK_CLOSE( bathy.data(index), -5249.0f, 1e-6f ) ;
    index[0]=180 ; index[1]=0 ;  // 2N 179E = first entry, last row
    BOOST_CHECK_CLOSE( bathy.data(index), -5600.0f, 1e-6f ) ;

    // compare depth to some known values
    // extracted by hand from etopo_cmp2w.log

    index[0]=0 ; index[1]=61 ;  // 1S 180.0167E (aka -179.98333W) = first entry, first row
    BOOST_CHECK_CLOSE( bathy.data(index), -5284.0f, 1e-6f ) ;
    index[0]=0 ; index[1]=180 ;  // 1S 182E (aka 178W) = last entry, first row
    BOOST_CHECK_CLOSE( bathy.data(index), -4775.0f, 1e-6f ) ;

    // dump CSV file for processing by graphics program

    std::ofstream ofile( "span_bathy.csv" ) ;
    for ( int n=0 ; n < num_lat ; ++n ) {
        index[0] = n ;
        for ( int m=0 ; m < num_lng ; ++m ) {
            index[1] = m ;
            ofile << bathy.data(index) ;
            if ( m < (num_lng-1) ) ofile << "," ;
        }
        ofile << std::endl ;
    }
}


/// @}

BOOST_AUTO_TEST_SUITE_END()

