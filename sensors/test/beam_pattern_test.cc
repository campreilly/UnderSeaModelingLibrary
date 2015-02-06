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
 * and directivity index should be 0 dB.
 *
 * A check for the directivity index, as computed
 * using a Simpson's rule approximation, fails if it
 * is different from zero by 0.02 dB or more.
 */
BOOST_AUTO_TEST_CASE( omni_pattern_test ) {
    cout << "=== beam_pattern_test/omni_pattern_test ===" << endl ;
    seq_linear freq(10.0, 10.0, 10) ;
    beam_pattern_omni omni( freq ) ;

    vector<double> level( freq.size(), 0.0 ) ;
    double total = 0 ;
    for(int az=0; az<=360; ++az) {
        for(int de=-90; de<=90; ++de) {
            double de_rad = de * M_PI/180.0 ;
            double az_rad = az * M_PI/180.0 ;
            omni.beam_level( de_rad, az_rad, 0, &level ) ;
            total += level(0)*cos(de_rad)*(M_PI/180.0)*(M_PI/180.0) ;
            BOOST_CHECK_EQUAL( level(0), 1.0 ) ;
        }
    }

    total = 10.0 * log10( (4.0*M_PI) / total ) ;
    cout << "Directivity index: " << total << endl ;
    BOOST_CHECK_SMALL( omni.directivity_index(0)-total, 0.02 ) ;

}

/**
 * Test the functionality of the sine-directional
 * beam pattern class. The beam_level should have a maximum
 * response in the East and West directions.
 *
 * This test fails if the maximum response axis differs from
 * the North (AZ=90) and South (AZ=270) directions. Also,
 * if the directivity index, as computed using a Simpson's
 * rule approximation, is different from the analytic solution
 * by 1% the test fails.
 */
BOOST_AUTO_TEST_CASE( sine_pattern_test ) {
    cout << "=== beam_pattern_test/sine_pattern_test ===" << endl ;
    const char* csvname = USML_TEST_DIR "/sensors/test/beam_pattern_sine.csv" ;
    seq_linear freq(10.0, 10.0, 10) ;
    beam_pattern_sine sine( freq ) ;

    int pitch = 62 ;
    int yaw = 31 ;
    double d2r = M_PI / 180.0 ;
    sine.orient_beam( 0.0, pitch*d2r, yaw*d2r ) ;

    std::ofstream of( csvname ) ;
    cout << "Saving beam data to " << csvname << endl ;
    vector<double> level( freq.size(), 0.0 ) ;
    double total = 0.0 ;
    for(int az=0; az<=360; ++az) {
        for(int de=-90; de<=90; ++de) {
            double de_rad = de * d2r ;
            double az_rad = az * d2r ;
            sine.beam_level( de_rad, az_rad, 0, &level ) ;
            of << level(0) ;
            if( de!=90 ) of << "," ;
            total += abs(level(0))*cos(de_rad)*(M_PI/180.0)*(M_PI/180.0) ;
            if( (az==yaw && de==pitch) || (az==(180+yaw) && de==-pitch) )
                BOOST_CHECK_CLOSE( abs(level(0)), 1.0, 0.2 ) ;
        }
        of << endl ;
    }
    total = 10.0*log10( (4*M_PI) / total ) ;
    cout << "Directivity index" << endl ;
    cout << "analytic: " << sine.directivity_index(0) << "\napproximation: " << total << endl ;
    BOOST_CHECK_CLOSE( sine.directivity_index(0), total, 1.0 ) ;

}

/**
 * Test the functionality of the cosine-directional
 * beam pattern class. The beam_level should have a maximum
 * response in the North and South directions.
 *
 * This test fails if the maximum response axis differs from
 * the East (AZ=0) and West (AZ=180) directions. Also,
 * if the directivity index, as computed using a Simpson's
 * rule approximation, is different from the analytic solution
 * by 1% the test fails.
 */
BOOST_AUTO_TEST_CASE( cosine_pattern_test ) {
    cout << "=== beam_pattern_test/cosine_pattern_test ===" << endl ;
    const char* csvname = USML_TEST_DIR "/sensors/test/beam_pattern_cosine.csv" ;
    seq_linear freq(10.0, 10.0, 10) ;
    beam_pattern_cosine cosine( freq ) ;

    int pitch = 21 ;
    int yaw = 57 ;
    double d2r = M_PI / 180.0 ;
    cosine.orient_beam( 0.0, pitch*d2r, yaw*d2r ) ;

    std::ofstream of( csvname ) ;
    cout << "Saving beam data to " << csvname << endl ;
    vector<double> level( freq.size(), 0.0 ) ;
    double total = 0.0 ;
    for(int az=0; az<=360; ++az) {
        for(int de=-90; de<=90; ++de) {
            double de_rad = de * d2r ;
            double az_rad = az * d2r ;
            cosine.beam_level( de_rad, az_rad, 0, &level ) ;
            of << level(0) ;
            if( de!=90 ) of << "," ;
            total += abs(level(0))*cos(de_rad)*(M_PI/180.0)*(M_PI/180.0) ;
            if( (az==(270-yaw) && de==-pitch) || (az==(90-yaw) && de==pitch) )
                BOOST_CHECK_CLOSE( abs(level(0)), 1.0, 0.2 ) ;
        }
        of << endl ;
    }
    total = 10.0*log10( (4*M_PI) / total ) ;
    cout << "Directivity index" << endl ;
    cout << "analytic: " << cosine.directivity_index(0) << "\napproximation: " << total << endl ;
    BOOST_CHECK_CLOSE( cosine.directivity_index(0), total, 1.0 ) ;

}

/**
 * Test the basic features of the beam_pattern_model using
 * a vertical array of elements model. Data is save to a
 * file and support matlab code is provided to verify the
 * spatial orientation of the beam pattern is correct.
 *
 * The test fails if either the following are false:
 *
 *  - The main lobe is in the correct is not in the correct
 *    direction. If the maximum response is not in the
 *    correct direction, the beam level will differ from
 *    1.0 by greater than 1e-4%
 *  - The Directivity index is also differs from a
 *    a Simpson's rule integration approximation of
 *    directivity index from beam level, by more than
 *    1.0%.
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

    beam_pattern_line array( c0, d, n, freq, &steering ) ;

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
    cout << "Directivity index" << endl ;
    cout << "analytic: " << array.directivity_index(0) << "\napproximation: " << total << endl ;
    BOOST_CHECK_CLOSE( array.directivity_index(0), total, 1.0 ) ;
}

/**
 * Test the basic features of the beam_pattern_model using
 * a horizontal array of elements model. Data is save to a
 * file and support matlab code is provided to verify the
 * spatial orientation of the beam pattern is correct.
 *
 * The test fails if either the following are false:
 *
 *  - The main lobe is in the correct is not in the correct
 *    direction. If the maximum response is not in the
 *    correct direction, the beam level will differ from
 *    1.0 by greater than 1e-4%
 *  - The Directivity index is also differs from a
 *    a Simpson's rule integration approximation of
 *    directivity index from beam level, by more than
 *    1.0%.
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

    beam_pattern_line array( c0, d, n, freq, &steering, beam_pattern_line::HORIZONTAL ) ;

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
    cout << "Directivity index" << endl ;
    cout << "analytic: " << array.directivity_index(0) << "\napproximation: " << total << endl ;
    BOOST_CHECK_CLOSE( array.directivity_index(0), total, 1.0 ) ;
}

/**
 * Test the basic features of the beam_pattern_model using
 * a beam pattern function of solid angle.
 *
 * The test fails if either the following are false:
 *
 *  - The main lobe is in the correct is not in the correct
 *    direction. If the maximum response is not in the
 *    correct direction, the beam level will differ from
 *    1.0 by greater than 1e-4%
 *  - The Directivity index is also differs from a
 *    a Simpson's rule integration approximation of
 *    directivity index from beam level, by more than
 *    1.0%.
 */
BOOST_AUTO_TEST_CASE( solid_pattern_test ) {
    cout << "===== beam_pattern_test/solid_pattern_test =====" << endl ;
    const char* csvname = USML_TEST_DIR "/sensors/test/beam_pattern_solid.csv" ;

    // Physical and Environmental parameters concerning the array
    double max_de = 20.0 ;
    double min_de = -20.0 ;
    double max_az = 135.0 ;
    double min_az = 45.0 ;
    seq_linear freq(900.0, 1.0, 1.0) ;
    beam_pattern_solid solid( max_de, min_de, max_az, min_az, freq ) ;

    double pitch = 17.0 ;
    double yaw = 41.0 ;
    solid.orient_beam( 0.0, pitch*M_PI/180.0, yaw*M_PI/180.0 ) ;
    cout << "beam oriented (roll,pitch,yaw): ("
         << 0.0 << ", " << pitch << ", " << yaw << ")" << endl ;

    std::ofstream of( csvname ) ;
    cout << "Saving beam data to " << csvname << endl ;
    vector<double> level( freq.size(), 0.0 ) ;
    double total = 0 ;
    for(int az=0; az<=360; ++az) {
        for(int de=-90; de<=90; ++de) {
            double de_rad = de * M_PI/180.0 ;
            double az_rad = az * M_PI/180.0 ;
            solid.beam_level( de_rad, az_rad, 0, &level ) ;
            of << level(0) ;
            if( de!=90 ) of << "," ;
            total += level(0)*cos(de_rad)*(M_PI/180.0)*(M_PI/180.0) ;
            double result = 0.0 ;
            if( (de<(max_de-pitch)) && (de>=(min_de-pitch)) ) {
                if( (az<=(max_az+yaw)) && (az>=(min_az+yaw)) ) {
                    result = 1.0 ;
                }
            }
            BOOST_CHECK_EQUAL( level(0), result ) ;
        }
        of << endl ;
    }

    total = 10.0*log10( (4*M_PI)/total ) ;
    cout << "Directivity index" << endl ;
    cout << "analytic: " << solid.directivity_index(0) << "\napproximation: " << total << endl ;
    BOOST_CHECK_CLOSE( solid.directivity_index(0), total, 1.5 ) ;
}

BOOST_AUTO_TEST_SUITE_END()
