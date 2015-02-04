/*
 * @examples sensors/test/beam_pattern_test.cc
 */

#include <boost/test/unit_test.hpp>
#include <usml/sensors/beams.h>
#include <fstream>

BOOST_AUTO_TEST_SUITE(beam_pattern_test)

using namespace boost::unit_test ;
using namespace usml::sensors ;

/**
 * @ingroup beams_test
 * @{
 */

/**
 * Test the functionality of the omni-directional
 * beam pattern class. The beam_level should always be 1.0
 * and directivity index should be 0 dB. An error is passed
 * if ever these things are not true.
 */
BOOST_AUTO_TEST_CASE( omni_pattern_test ) {
    cout << "=== beam_pattern_test/omni_pattern_test ===" << endl ;
    seq_linear freq(10.0, 10.0, 10) ;
    beam_pattern_omni omni( freq ) ;
    cout << "frequencies: " << freq << endl ;

    BOOST_CHECK_EQUAL( omni.directivity_index(0), 0.0 ) ;
    vector<double> level( freq.size(), 0.0 ) ;
    for(int az=0; az<=360; ++az) {
        for(int de=-90; de<=90; ++de) {
            double de_rad = de * M_PI/180.0 ;
            double az_rad = az * M_PI/180.0 ;
            omni.beam_level( de_rad, az_rad, 0, &level ) ;
            BOOST_CHECK_EQUAL( level(0), 1.0 ) ;
        }
    }

}

/**
 * Test the basic features of the beam_pattern_model using
 * a vertical array of elements model. Output data to a file
 * and run supplied matlab code to verify that the data is
 * comparable to the matlab generated version.
 *
 * Added a check to confirm the main lobe is in the correct
 * direction. If the level value is different from 1.0 by
 * 1e-4% an error is thrown. The Directivity index is also
 * checked using a simpson rule integration approximation.
 * The approximate integration is then compared to the
 * analytic solution. An error is thrown if the values
 * differ by more than 1%.
 */
BOOST_AUTO_TEST_CASE( vertical_array_test ) {
    cout << "===== beam_pattern_test/vertical_array_test =====" << endl ;
    const char* envname = USML_TEST_DIR "/sensors/test/vertical_array_parameters.csv" ;
    const char* csvname = USML_TEST_DIR "/sensors/test/vertical_array_beam_pattern.csv" ;

    // Physical and Environmental parameters concerning the array
    double c0 = 1500.0 ;
    double d = 0.75 ;
    double n = 5 ;
    vector<double> steering( 1, M_PI/32.0 ) ;
    seq_linear freq(900.0, 1.0, 1.0) ;

    vertical_array array( c0, d, n, freq, &steering ) ;

    double roll = 0.0 * M_PI/180.0 ;
    double pitch = 35.0 * M_PI/180.0 ;
    double yaw = 45.0 * M_PI/180.0 ;
    array.orient_beam( roll, pitch, yaw ) ;
    cout << "beam oriented (roll,pitch,yaw): ("
         << roll << ", " << pitch << ", " << yaw << ")" << endl ;

    std::ofstream of( csvname ) ;
    cout << "Saving beam data to " << csvname << endl ;
    vector<double> level( freq.size(), 0.0 ) ;
    double total = 0 ;
    for(int az=0; az<=360; ++az) {
        for(int de=-90; de<=90; ++de) {
            double de_rad = de * M_PI/180.0 ;
            double az_rad = az * M_PI/180.0 ;
            array.beam_level( de_rad, az_rad, 0, &level ) ;
            of << level(0) ;
            if( de!=90 ) of << "," ;
            total += level(0)*cos(de_rad)*(M_PI/180.0)*(M_PI/180.0) ;
        }
        of << endl ;
    }

    // check that the main lobe is at the correct position
    array.beam_level( -(pitch+steering(0)), -yaw, 0, &level ) ;
    BOOST_CHECK_CLOSE( level(0), 1.0, 1e-4 ) ;

    std::ofstream ef( envname ) ;
    cout << "Saving environmental and array parameters to " << envname << endl ;
    ef << "c0,d,n,roll,pitch,yaw,steering,freq" << endl ;
    ef << c0 << ","
       << d << ","
       << n << ","
       << roll << ","
       << pitch << ","
       << yaw << ","
       << steering(0) << "," ;
    for(int i=0; i<freq.size(); ++i) {
        ef << freq(i) ;
        if( i != freq.size()-1 ) ef << "," ;
    }
    ef << endl ;

    total = 10.0*log10( (4*M_PI)/total ) ;
    BOOST_CHECK_CLOSE( array.directivity_index(0), total, 1.0 ) ;
}

/**
 * Test the basic features of the beam_pattern_model using
 * a horizontal array of elements model. Output data to a file
 * and run supplied matlab code to verify that the data is
 * comparable to the matlab generated version.
 *
 * Added a check to confirm the main lobe is in the correct
 * direction. If the level value is different from 1.0 by
 * 1e-4% an error is thrown. The Directivity index is also
 * checked using a simpson rule integration approximation.
 * The approximate integration is then compared to the
 * analytic solution. An error is thrown if the values
 * differ by more than 1%.
 */
BOOST_AUTO_TEST_CASE( horizontal_array_test ) {
    cout << "===== beam_pattern_test/horizontal_array_test =====" << endl ;
    const char* envname = USML_TEST_DIR "/sensors/test/horizontal_array_parameters.csv" ;
    const char* csvname = USML_TEST_DIR "/sensors/test/horizontal_array_beam_pattern.csv" ;

    // Physical and Environmental parameters concerning the array
    double c0 = 1500.0 ;
    double d = 0.75 ;
    double n = 5 ;
    vector<double> steering( 1, M_PI/4.0 ) ;
    seq_linear freq(900.0, 1.0, 1.0) ;

    horizontal_array array( c0, d, n, freq, &steering ) ;

    double roll = 0.0 * M_PI/180.0 ;
    double pitch = 45.0 * M_PI/180.0 ;
    double yaw = 45.0 * M_PI/180.0 ;
    array.orient_beam( roll, pitch, yaw ) ;
    cout << "beam oriented (roll,pitch,yaw): ("
         << roll << ", " << pitch << ", " << yaw << ")" << endl ;

    std::ofstream of( csvname ) ;
    cout << "Saving beam data to " << csvname << endl ;
    vector<double> level( freq.size(), 0.0 ) ;
    double total = 0 ;
    for(int az=0; az<=360; ++az) {
        for(int de=-90; de<=90; ++de) {
            double de_rad = de * M_PI/180.0 ;
            double az_rad = az * M_PI/180.0 ;
            array.beam_level( de_rad, az_rad, 0, &level ) ;
            of << level(0) ;
            if( de!=90 ) of << "," ;
            total += level(0)*cos(de_rad)*(M_PI/180.0)*(M_PI/180.0) ;
        }
        of << endl ;
    }

    // check that the main lobe is at the correct position
    array.beam_level( pitch+steering(0)-M_PI/2.0, yaw, 0, &level ) ;
    BOOST_CHECK_CLOSE( level(0), 1.0, 1e-4 ) ;

    std::ofstream ef( envname ) ;
    cout << "Saving environmental and array parameters to " << envname << endl ;
    ef << "c0,d,n,roll,pitch,yaw,steering,freq" << endl ;
    ef << c0 << ","
       << d << ","
       << n << ","
       << roll << ","
       << pitch << ","
       << yaw << ","
       << steering(0) << "," ;
    for(int i=0; i<freq.size(); ++i) {
        ef << freq(i) ;
        if( i != freq.size()-1 ) ef << "," ;
    }
    ef << endl ;

    total = 10.0*log10( (4*M_PI)/total ) ;
    BOOST_CHECK_CLOSE( array.directivity_index(0), total, 1.0 ) ;
}

BOOST_AUTO_TEST_SUITE_END()
