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
 * Test the basic features of the beam_pattern_model using
 * a vertical array of elements model. Output data to a file
 * and run supplied matlab code to verify that the data is
 * comparable to the matlab generated version.
 */
BOOST_AUTO_TEST_CASE( vertical_array_test ) {
    cout << "===== beam_pattern_test/vertical_array_test =====" << endl ;
    const char* envname = USML_TEST_DIR "/sensors/test/vertical_array_parameters.csv" ;
    const char* csvname = USML_TEST_DIR "/sensors/test/vertical_array_beam_pattern.csv" ;

    // Physical and Environmental parameters concerning the array
    double c0 = 1500.0 ;
    double d = 0.75 ;
    double n = 5 ;
    vector<double> steering( 1, M_PI/4.0 ) ;
    seq_linear freq(900.0, 1.0, 1.0) ;

    vertical_array array( c0, d, n, freq, &steering ) ;
    cout << "Vertical array class has been constructed" << endl ;

    double roll = 0.0 * M_PI/180.0 ;
    double pitch = 45.0 * M_PI/180.0 ;
    double yaw = 45.0 * M_PI/180.0 ;
    array.orient_beam( roll, pitch, yaw ) ;
    cout << "beam oriented (roll,pitch,yaw): ("
         << roll << ", " << pitch << ", " << yaw << ")" << endl ;

    std::ofstream of( csvname ) ;
    cout << "Saving beam data to " << csvname << endl ;
    vector<double> level( freq.size(), 0.0 ) ;
    for(int az=0; az<=360; ++az) {
        for(int de=-90; de<=90; ++de) {
            double de_rad = de * M_PI/180.0 ;
            double az_rad = az * M_PI/180.0 ;
            array.beam_level( de_rad, az_rad, 0, &level ) ;
            of << level(0) ;
            if( de!=90 ) of << "," ;
        }
        of << endl ;
    }

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
}

/**
 * Test the basic features of the beam_pattern_model using
 * a vertical array of elements model. Output data to a file
 * and run supplied matlab code to verify that the data is
 * comparable to the matlab generated version.
 */
BOOST_AUTO_TEST_CASE( horizontal_array_test ) {
    cout << "===== beam_pattern_test/vertical_array_test =====" << endl ;
    const char* envname = USML_TEST_DIR "/sensors/test/horizontal_array_parameters.csv" ;
    const char* csvname = USML_TEST_DIR "/sensors/test/horizontal_array_beam_pattern.csv" ;

    // Physical and Environmental parameters concerning the array
    double c0 = 1500.0 ;
    double d = 0.75 ;
    double n = 5 ;
    vector<double> steering( 1, M_PI/4.0 ) ;
    seq_linear freq(900.0, 1.0, 1.0) ;

    horizontal_array array( c0, d, n, freq, &steering ) ;
    cout << "Vertical array class has been constructed" << endl ;

    double roll = 0.0 * M_PI/180.0 ;
    double pitch = 45.0 * M_PI/180.0 ;
    double yaw = 45.0 * M_PI/180.0 ;
    array.orient_beam( roll, pitch, yaw ) ;
    cout << "beam oriented (roll,pitch,yaw): ("
         << roll << ", " << pitch << ", " << yaw << ")" << endl ;

    std::ofstream of( csvname ) ;
    cout << "Saving beam data to " << csvname << endl ;
    vector<double> level( freq.size(), 0.0 ) ;
    for(int az=0; az<=360; ++az) {
        for(int de=-90; de<=90; ++de) {
            double de_rad = de * M_PI/180.0 ;
            double az_rad = az * M_PI/180.0 ;
            array.beam_level( de_rad, az_rad, 0, &level ) ;
            of << level(0) ;
            if( de!=90 ) of << "," ;
        }
        of << endl ;
    }

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
}

BOOST_AUTO_TEST_SUITE_END()
