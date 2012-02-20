/**
 * @example ocean/test/reflect_loss_test.cc
 */
#define BOOST_TEST_DYN_LINK
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
    const char* name = "rayleigh_test_a.csv" ;
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
    const char* name = "rayleigh_test_b.csv" ;
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
    const char* name = "rayleigh_test_c.csv" ;
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
    const char* name = "rayleigh_test_d.csv" ;
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
    const char* name = "rayleigh_sediments.csv" ;
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
                points, freq, to_radians(90-angle), &amplitude ) ;
            os << "," << amplitude(0) ;
        }
        os << endl ;
    }
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
