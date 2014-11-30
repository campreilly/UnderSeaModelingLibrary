/**
 * @example ocean/test/reflect_loss_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <usml/ocean/ocean.h>
#include <fstream>

BOOST_AUTO_TEST_SUITE(reflect_loss_test)

using namespace boost::unit_test;
using namespace usml::ocean;

/**
 * @ingroup ocean_test
 * @{
 */

/**
 * Test the basic features of the reflection loss model using
 * the reflect_loss_constant model.
 * Generate errors if values differ by more that 1E-6 percent.
 */
BOOST_AUTO_TEST_CASE( constant_reflect_loss_test ) {
    cout << "=== reflect_loss_test: constant_reflect_loss_test ===" << endl ;

    // simple values for points and distance

    wposition1 points ;

    // compute reflect_loss

    seq_log freq( 10.0, 10.0, 7 ) ;
    cout << "freq:  " << freq << endl ;
    vector<double> amplitude( freq.size() ) ;

    double value = 3.0 ;
    reflect_loss_constant model( value ) ;
    model.reflect_loss( points, freq, 0.1, &amplitude ) ;
    cout << "amplitude: " << amplitude << endl ;

    // check the answer

    for ( unsigned f=0 ; f < freq.size() ; ++f ) {
        BOOST_CHECK_CLOSE( amplitude(f), value, 1e-6 ) ;
    }
}

/**
 * Compare values of the Rayleigh model to test case (a)
 * from of the Rayleigh model in Jensen, Kuperman, Porter,
 * Schmidt, "Computational Ocean Acoustics",
 * Table 1.4 and Figure 1.22, Chapter 1.6.1, pp 46-47.
 * Write results to CSV file for processing in Excel or Matlab.
 */
BOOST_AUTO_TEST_CASE( rayleigh_test_a ) {
    cout << "=== reflect_loss_test: rayleigh_test_a ===" << endl ;
    const char* name = USML_TEST_DIR "/ocean/test/rayleigh_test_a.csv" ;
    std::ofstream os(name) ;
    cout << "writing tables to " << name << endl ;

    // simple values for points and distance

    wposition1 points ;
    points.altitude(-1000.0) ;

    seq_log freq( 10.0, 10.0, 7 ) ;
    cout << "freq:  " << freq << endl ;
    vector<double> amplitude( freq.size() ) ;

    // test case (a) - variations with sediment sound speed

    static double speed[] = { 1550.0, 1600.0, 1800 } ;
    os << "angle,cp=1550,cp=1600,cp=1800" << endl ;
    for ( int angle = 0 ; angle <= 90 ; ++angle ) {
        os << angle ;
        for ( int n=0 ; n < 3 ; ++n ) {
            reflect_loss_rayleigh model( 2.0, speed[n]/1500.0, 0.5 ) ;
            model.reflect_loss(
                points, freq, to_radians(90-angle), &amplitude ) ;
            os << "," << amplitude(0) ;
        }
        os << endl ;
    }
}

/**
 * Compare values of the Rayleigh model to test case (b)
 * from of the Rayleigh model in Jensen, Kuperman, Porter,
 * Schmidt, "Computational Ocean Acoustics",
 * Table 1.4 and Figure 1.22, Chapter 1.6.1, pp 46-47.
 * Write results to CSV file for processing in Excel or Matlab.
 */
BOOST_AUTO_TEST_CASE( rayleigh_test_b ) {
    cout << "=== reflect_loss_test: rayleigh_test_b ===" << endl ;
    const char* name = USML_TEST_DIR "/ocean/test/rayleigh_test_b.csv" ;
    std::ofstream os(name) ;
    cout << "writing tables to " << name << endl ;

    // simple values for points and distance

    wposition1 points ;
    points.altitude(-1000.0) ;

    seq_log freq( 10.0, 10.0, 7 ) ;
    cout << "freq:  " << freq << endl ;
    vector<double> amplitude( freq.size() ) ;

    // test case (a) - variations with sediment attenuation

    static double atten[] = { 1.0, 0.5, 0.0 } ;
    os << "angle,a=1.0,a=0.5,a=0.0" << endl ;
    for ( int angle = 0 ; angle <= 90 ; ++angle ) {
        os << angle ;
        for ( int n=0 ; n < 3 ; ++n ) {
            reflect_loss_rayleigh model( 2.0, 1600.0/1500.0, atten[n] ) ;
            model.reflect_loss(
                points, freq, to_radians(90-angle), &amplitude ) ;
            os << "," << amplitude(0) ;
        }
        os << endl ;
    }
}

/**
 * Compare values of the Rayleigh model to test case (c)
 * from of the Rayleigh model in Jensen, Kuperman, Porter,
 * Schmidt, "Computational Ocean Acoustics",
 * Table 1.4 and Figure 1.22, Chapter 1.6.1, pp 46-47.
 * Write results to CSV file for processing in Excel or Matlab.
 */
BOOST_AUTO_TEST_CASE( rayleigh_test_c ) {
    cout << "=== reflect_loss_test: rayleigh_test_c ===" << endl ;
    const char* name = USML_TEST_DIR "/ocean/test/rayleigh_test_c.csv" ;
    std::ofstream os(name) ;
    cout << "writing tables to " << name << endl ;

    // simple values for points and distance

    wposition1 points ;
    points.altitude(-1000.0) ;

    seq_log freq( 10.0, 10.0, 7 ) ;
    cout << "freq:  " << freq << endl ;
    vector<double> amplitude( freq.size() ) ;

    // test case (c) - variations with density

    static double density[] = { 1.5, 2.0, 2.5 } ;
    os << "angle,d=1.5,d=2.0,d=2.5" << endl ;
    for ( int angle = 0 ; angle <= 90 ; ++angle ) {
        os << angle ;
        for ( int n=0 ; n < 3 ; ++n ) {
            reflect_loss_rayleigh model( density[n], 1600.0/1500.0, 0.5 ) ;
            model.reflect_loss(
                points, freq, to_radians(90-angle), &amplitude ) ;
            os << "," << amplitude(0) ;
        }
        os << endl ;
    }
}

/**
 * Compare values of the Rayleigh model to test case (b)
 * from of the Rayleigh model in Jensen, Kuperman, Porter,
 * Schmidt, "Computational Ocean Acoustics",
 * Table 1.4 and Figure 1.22, Chapter 1.6.1, pp 46-47.
 * Write results to CSV file for processing in Excel or Matlab.
 *
 * Although Table 1.4 indicates an attenuation of 0.5,
 * you can tell from the cs=0 case that an attenuation of 0.0
 * was used to make Figure 1.22.
 */
BOOST_AUTO_TEST_CASE( rayleigh_test_d ) {
    cout << "=== reflect_loss_test: rayleigh_test_d ===" << endl ;
    const char* name = USML_TEST_DIR "/ocean/test/rayleigh_test_d.csv" ;
    std::ofstream os(name) ;
    cout << "writing tables to " << name << endl ;

    // simple values for points and distance

    wposition1 points ;
    points.altitude(-1000.0) ;

    seq_log freq( 10.0, 10.0, 7 ) ;
    cout << "freq:  " << freq << endl ;
    vector<double> amplitude( freq.size() ) ;

    // test case (d) - variations with shear speed

    static double shear[] = { 0.0, 200.0, 400.0, 600.0 } ;
    os << "angle,cs=0.0,cs=200,cs=400,cs=600" << endl ;
    for ( int angle = 0 ; angle <= 90 ; ++angle ) {
        os << angle ;
        for ( int n=0 ; n < 4 ; ++n ) {
            reflect_loss_rayleigh model(
                2.0, 1600.0/1500.0, 0.0, shear[n]/1500.0 ) ;
            model.reflect_loss(
                points, freq, to_radians(90-angle), &amplitude ) ;
            os << "," << amplitude(0) ;
        }
        os << endl ;
    }
}

/**
 * Compute Rayleigh model values for generic sediments.
 * Write results to CSV file for processing in Excel or Matlab.
 */
BOOST_AUTO_TEST_CASE( plot_rayleigh_sediments ) {
    cout << "=== reflect_loss_test: plot_rayleigh_sediments ===" << endl ;
    const char* name = USML_TEST_DIR "/ocean/test/rayleigh_sediments.csv" ;
    std::ofstream os(name) ;
    cout << "writing tables to " << name << endl ;

    // simple values for points and distance

    wposition1 points ;
    points.altitude(-1000.0) ;

    seq_log freq( 10.0, 10.0, 7 ) ;
    cout << "freq:  " << freq << endl ;
    vector<double> amplitude( freq.size() ) ;

    // test case (a) - variations with sediment sound speed

    static reflect_loss_rayleigh::bottom_type_enum sediment[] = {
        reflect_loss_rayleigh::CLAY,
        reflect_loss_rayleigh::SILT,
        reflect_loss_rayleigh::SAND,
        reflect_loss_rayleigh::GRAVEL,
        reflect_loss_rayleigh::MORAINE,
        reflect_loss_rayleigh::CHALK,
        reflect_loss_rayleigh::LIMESTONE,
        reflect_loss_rayleigh::BASALT
    } ;

    os << "angle,clay,silt,sand,gravel,moraine,chalk,limestone,basalt" << endl ;
    for ( int angle = 0 ; angle <= 90 ; ++angle ) {
        os << angle ;
        for ( int n=0 ; n < 8 ; ++n ) {
            reflect_loss_rayleigh model( sediment[n] ) ;
            model.reflect_loss(
                points, freq, to_radians(angle), &amplitude ) ;
            os << "," << amplitude(0) ;
        }
        os << endl ;
    }
}

/**
 * Test the basic features of the reflection loss model using
 * the netCDF bottom type file.
 * Generate errors if values differ by more that 1E-5 percent.
 */
BOOST_AUTO_TEST_CASE( reflect_loss_netcdf_test ) {
	cout << " === reflection_loss_test: reflection_loss_netcdf bottom type file === " << endl;
	reflect_loss_netcdf netcdf( USML_DATA_DIR "/bottom_province/sediment_test.nc" ) ;

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

/**
 * Test the accuracy Pierson and Moskowitz model for computing wave height
 * from wind speed. Compare to significant wave height plot from
 * http://www.wikiwaves.org/Ocean-Wave_Spectra.
 */
BOOST_AUTO_TEST_CASE( wave_height_pierson_test ) {
    cout << "=== reflect_loss_test: wave_height_pierson_test ===" << endl;

    // display plotting data

    const char* csv_name = USML_TEST_DIR "/ocean/test/wave_height_pierson_test.csv" ;
    std::ofstream os(csv_name) ;
    cout << "writing tables to " << csv_name << endl ;
	os << "wind,Hsig" << endl ;
    for ( double wind=0.0 ; wind <= 25.0 ; wind += 1.0 ) {
    	os << wind << "," << 4*wave_height_pierson(wind) << endl ;
    }

    // check the answer against key points in plot

    BOOST_CHECK_CLOSE(wave_height_pierson(0.0), 0.0, 1e-6);
    BOOST_CHECK_CLOSE(wave_height_pierson(15.0), 5.0/4.0, 5.0 );
    BOOST_CHECK_CLOSE(wave_height_pierson(25.0), 14.0/4.0, 5.0 );
}

/**
 * Test the accuracy of the reflect_loss_eckart surface reflection loss model.
 */
BOOST_AUTO_TEST_CASE( reflect_loss_eckart_test ) {
    cout << "=== reflect_loss_test: reflect_loss_eckart_test ===" << endl;
    const char* name = USML_TEST_DIR "/ocean/test/reflect_loss_eckart_test.csv" ;
    std::ofstream os(name) ;
    cout << "writing tables to " << name << endl ;

    // simple values for points and distance

    wposition1 points ;
    seq_log freq( 1000.0, 1.0, 1 ) ;
    vector<double> amplitude( freq.size() ) ;

    // variations with wind speed

    os << "angle,wind=5,wind=10,wind=15" << endl ;
    for ( double angle = 0.0 ; angle <= 90.0 ; angle += 1.0 ) {
        os << angle ;
        for ( double wind=5.0 ; wind <= 15.0 ; wind += 5.0 ) {
        	reflect_loss_eckart model( wind ) ;
            model.reflect_loss(
                points, freq, to_radians(angle), &amplitude ) ;
            os << "," << -amplitude(0) ;
        }
        os << endl ;
    }
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
