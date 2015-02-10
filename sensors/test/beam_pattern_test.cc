/*
 * @examples sensors/test/beam_pattern_test.cc
 */

#include <boost/test/unit_test.hpp>
#include <usml/sensors/beams.h>
#include <fstream>
#include <cstdlib>
#include <ctime>

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
    vector<double> freq( 1, 900.0 ) ;
    beam_pattern_omni omni ;

    vector<double> level( freq.size(), 0.0 ) ;
    double total = 0 ;
    for(int az=0; az<=360; ++az) {
        for(int de=-90; de<=90; ++de) {
            double de_rad = de * M_PI/180.0 ;
            double az_rad = az * M_PI/180.0 ;
            omni.beam_level( de_rad, az_rad, freq, &level ) ;
            total += level(0)*cos(de_rad)*(M_PI/180.0)*(M_PI/180.0) ;
            BOOST_CHECK_EQUAL( level(0), 1.0 ) ;
        }
    }

    total = 10.0 * log10( (4.0*M_PI) / total ) ;
    omni.directivity_index( freq, &level ) ;
    cout << "Directivity index: " << total << endl ;
    BOOST_CHECK_SMALL( level(0)-total, 0.02 ) ;

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
    vector<double> freq( 1, 900.0 ) ;
    beam_pattern_sine sine ;

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
            sine.beam_level( de_rad, az_rad, freq, &level ) ;
            of << level(0) ;
            if( de!=90 ) of << "," ;
            total += abs(level(0))*cos(de_rad)*(M_PI/180.0)*(M_PI/180.0) ;
            if( (az==yaw && de==pitch) || (az==(180+yaw) && de==-pitch) )
                BOOST_CHECK_CLOSE( abs(level(0)), 1.0, 0.2 ) ;
        }
        of << endl ;
    }
    total = 10.0*log10( (4*M_PI) / total ) ;
    sine.directivity_index( freq, &level ) ;
    cout << "Directivity index" << endl ;
    cout << "analytic: " << level(0) << "\napproximation: " << total << endl ;
    BOOST_CHECK_CLOSE( level(0), total, 1.0 ) ;

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
    vector<double> freq( 1, 900.0 ) ;
    beam_pattern_cosine cosine ;

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
            cosine.beam_level( de_rad, az_rad, freq, &level ) ;
            of << level(0) ;
            if( de!=90 ) of << "," ;
            total += abs(level(0))*cos(de_rad)*(M_PI/180.0)*(M_PI/180.0) ;
            if( (az==(270-yaw) && de==-pitch) || (az==(90-yaw) && de==pitch) )
                BOOST_CHECK_CLOSE( abs(level(0)), 1.0, 0.2 ) ;
        }
        of << endl ;
    }
    total = 10.0*log10( (4*M_PI) / total ) ;
    cosine.directivity_index( freq, &level ) ;
    cout << "Directivity index" << endl ;
    cout << "analytic: " << level(0) << "\napproximation: " << total << endl ;
    BOOST_CHECK_CLOSE( level(0), total, 1.0 ) ;

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
    double steering = M_PI/32.0 ;
    vector<double> freq( 1, 900.0 ) ;

    beam_pattern_line array( c0, d, n, steering ) ;

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
            array.beam_level( de_rad, az_rad, freq, &level ) ;
            of << level(0) ;
            if( de!=90 ) of << "," ;
            total += level(0)*cos(de_rad)*(M_PI/180.0)*(M_PI/180.0) ;
        }
        of << endl ;
    }

    // check that the main lobe is at the correct position
    array.beam_level( -(pitch+steering), -yaw, freq, &level ) ;
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
       << steering << "," ;
    for(int i=0; i<freq.size(); ++i) {
        ef << freq(i) ;
        if( i != freq.size()-1 ) ef << "," ;
    }
    ef << endl ;

    total = 10.0*log10( (4*M_PI)/total ) ;
    array.directivity_index( freq, &level ) ;
    cout << "Directivity index" << endl ;
    cout << "analytic: " << level(0) << "\napproximation: " << total << endl ;
    BOOST_CHECK_CLOSE( level(0), total, 1.0 ) ;
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
    double steering = M_PI/4.0 ;
    vector<double> freq( 1, 900.0 ) ;

    beam_pattern_line array( c0, d, n, steering, beam_pattern_line::HORIZONTAL ) ;

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
            array.beam_level( de_rad, az_rad, freq, &level ) ;
            of << level(0) ;
            if( de!=90 ) of << "," ;
            total += level(0)*cos(de_rad)*(M_PI/180.0)*(M_PI/180.0) ;
        }
        of << endl ;
    }

    // check that the main lobe is at the correct position
    array.beam_level( pitch+steering, yaw, freq, &level ) ;
    BOOST_CHECK_CLOSE( level(0), 1.0, 1e-3 ) ;

    std::ofstream ef( envname ) ;
    cout << "Saving environmental and array parameters to " << envname << endl ;
    ef << "c0,d,n,roll,pitch,yaw,steering,freq" << endl ;
    ef << c0 << ","
       << d << ","
       << n << ","
       << roll << ","
       << pitch << ","
       << yaw << ","
       << steering << "," ;
    for(int i=0; i<freq.size(); ++i) {
        ef << freq(i) ;
        if( i != freq.size()-1 ) ef << "," ;
    }
    ef << endl ;

    total = 10.0*log10( (4*M_PI)/total ) ;
    array.directivity_index( freq, &level ) ;
    cout << "Directivity index" << endl ;
    cout << "analytic: " << level(0) << "\napproximation: " << total << endl ;
    BOOST_CHECK_CLOSE( level(0), total, 1.0 ) ;
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
    vector<double> freq( 1, 900.0 ) ;
    beam_pattern_solid solid( max_de, min_de, max_az, min_az ) ;

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
            solid.beam_level( de_rad, az_rad, freq, &level ) ;
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
    solid.directivity_index( freq, &level ) ;
    cout << "Directivity index" << endl ;
    cout << "analytic: " << level(0) << "\napproximation: " << total << endl ;
    BOOST_CHECK_CLOSE( level(0), total, 1.5 ) ;
}

/**
 * Test for a mesh beam pattern for a 1-D beam pattern. With
 * 1-D beam patterns the beam_level and directivity_index are
 * the same values. This test fails if any values that are
 * passed back are different from the data points for either
 * directivity_index or beam_level.
 */
BOOST_AUTO_TEST_CASE( grid_pattern_1d_test ) {
    cout << "===== beam_pattern_test/grid_pattern_1d_test =====" << endl ;

    typedef beam_pattern_grid<double,1>     grid_type ;

    const double tmp_data[] = { 1.0, 0.75, 0.5, 0.5, 0.75, 1.0 } ;
    int N = sizeof(tmp_data)/sizeof(double) ;
    vector<double> freq(N) ;
    double axis_data[] = { 10.1, 57.0, 79.0, 81.5, 100.7, 152.7 } ;
//    seq_vector* axes = new seq_log( 10.0, 10.0, N ) ;
    seq_vector* axes[1] ;
    axes[0] = new seq_data( axis_data, N ) ;
    double* data = new double[N] ;
    for(int i=0; i<N; ++i) {
        freq[i] = (*axes[0])[i] ;
        data[i] = tmp_data[i] ;
    }

    grid_type test_grid( axes, data, grid_type::LINEAR_UNITS ) ;
    cout << "frequencies: " << freq << endl ;

    const char* grid_file = "beam_pattern_grid1_test.nc" ;
    cout << "Writing data_grid to disk, " << grid_file << endl ;
    test_grid.write_netcdf( grid_file ) ;

    vector<double> level( N, 0.0 ) ;
    test_grid.beam_level( 0.0, 0.0, freq, &level ) ;
    for(int i=0; i<level.size(); ++i) {
        BOOST_CHECK_EQUAL( tmp_data[i], level(i) ) ;
    }
    cout << "beam level: " << level << endl ;
    test_grid.directivity_index( freq, &level ) ;
    for(int i=0; i<level.size(); ++i) {
        double result = 10.0*log10( tmp_data[i] ) ;
        BOOST_CHECK_EQUAL( result, level(i) ) ;
    }
    cout << "Directivity index: " << level << endl ;

    delete axes[0] ;
    delete data ;
}

/**
 * Test for a mesh beam pattern for a 2-D beam pattern.
 * This test fails if any values that are passed back are
 * different from the data points for either directivity
 * index or beam level.
 */
BOOST_AUTO_TEST_CASE( grid_pattern_2d_test ) {
    cout << "===== beam_pattern_test/grid_pattern_2d_test =====" << endl ;

    typedef beam_pattern_grid<double,2>     grid_type ;

    // ----> axis0
    // |
    // |
    // v axis1
    const double tmp_data[] = { 1.0, 0.75, 0.5, 0.75, 0.81,
                                0.87, 0.75, 0.5, 0.75, 0.41,
                                0.2, 0.75, 0.5, 0.75, 0.33,
                                0.61, 0.75, 0.5, 0.75, 0.97,
                                0.53, 0.75, 0.5, 0.75, 0.53 } ;
    int N = sizeof(tmp_data)/sizeof(double) ;
    int n = sqrt(N) ;
    seq_linear* frequencies = new seq_linear( 100.0, 100.0, n ) ;
    vector<double> freq = *frequencies ;
    seq_vector* de = new seq_linear( to_radians(-2.0), to_radians(1.0), n ) ;
    seq_vector* axes[2] ;
    axes[0] = frequencies ;
    axes[1] = de ;
    double* data = new double[N] ;
    for(int i=0; i<N; ++i) {
        data[i] = tmp_data[i] ;
    }

    grid_type test_grid( axes, data, grid_type::LINEAR_UNITS ) ;
    cout << "frequencies: " << freq << endl ;

    const char* grid_file = "beam_pattern_grid2_test.nc" ;
    cout << "Writing data_grid to disk, " << grid_file << endl ;
    test_grid.write_netcdf( grid_file ) ;

    vector<double> level( freq.size(), 0.0 ) ;
    for(int i=0; i<de->size(); ++i) {
        test_grid.beam_level( (*de)[i], 0.0, freq, &level ) ;
        for(int j=0; j<level.size(); ++j) {
            BOOST_CHECK_CLOSE( tmp_data[j*n+i], level(j), 1e-8 ) ;
        }
    }
    cout << "beam level: " << level << endl ;
    test_grid.directivity_index( freq, &level ) ;
    vector<double> sum( n, 0.0 ) ;
    for(int i=0; i<n; ++i) {
        for(int j=0; j<n; ++j) {
            sum[i] += tmp_data[i*n+j] * cos( (*de)[j] ) * de->increment(j) ;
        }
    }
    for(int i=0; i<level.size(); ++i) {
        double result = 10.0*log10( 2.0 / sum[i] ) ;
        BOOST_CHECK_EQUAL( result, level(i) ) ;
    }
    cout << "Directivity index: " << level << endl ;

    delete frequencies ;
    delete de ;
    delete data ;
}

/**
 * Test for a mesh beam pattern for a 3-D beam pattern.
 * This test fails if any values that are passed back are
 * different from the data points for either directivity
 * index or beam level.
 */
BOOST_AUTO_TEST_CASE( grid_pattern_3d_test ) {
    cout << "===== beam_pattern_test/grid_pattern_3d_test =====" << endl ;

    typedef beam_pattern_grid<double,3>     grid_type ;
    typedef grid_type::size_type            size_type ;

    // ----> axis0
    // |
    // |
    // v axis1
    // * axis2

    int n = 5 ;
    int N = n*n*n ;
    seq_linear frequencies( 100.0, 100.0, n ) ;
    vector<double> freq = frequencies ;
    seq_linear de( to_radians(-2.0), to_radians(1.0), n ) ;
    seq_linear az( to_radians(0.0), to_radians(5.0), n ) ;
    seq_vector* axes[3] ;
    axes[0] = &frequencies ;
    axes[1] = &de ;
    axes[2] = &az ;
    double* data = new double[N] ;
    std::srand(1) ;
    for(int i=0; i<N; ++i) {
        double v = std::rand() ;
        data[i] = v/RAND_MAX ;
    }

    grid_type test_grid( axes, data, grid_type::LINEAR_UNITS ) ;
    cout << "frequencies: " << freq << endl ;

    const char* grid_file = "beam_pattern_grid3_test.nc" ;
    cout << "Writing data_grid to disk, " << grid_file << endl ;
    test_grid.write_netcdf( grid_file ) ;

    size_type num_freq( freq.size() ) ;
    size_type num_de( de.size() ) ;
    size_type num_az( az.size() ) ;
    vector<double> level( num_freq, 0.0 ) ;
    for(size_type i=0; i<num_de; ++i) {
        for(size_type j=0; j<num_az; ++j) {
            test_grid.beam_level( de[i], az[j], freq, &level ) ;
            for(int k=0; k<num_freq; ++k) {
                size_type index = j + num_az*(i + num_de*k) ;
                BOOST_CHECK_CLOSE( data[index], level(k), 1e-8 ) ;
            }
        }
    }
    cout << "beam level: " << level << endl ;
    test_grid.directivity_index( freq, &level ) ;
    vector<double> sum( num_freq, 0.0 ) ;
    for(size_type i=0; i<num_de; ++i) {
        for(size_type j=0; j<num_az; ++j) {
            for(size_type k=0; k<num_freq; ++k) {
                size_type index = j + num_az*(i + num_de*k) ;
                sum[k] += data[index] * cos( de[i] )
                        * de.increment(i) * az.increment(j) ;
            }
        }
    }
    for(int i=0; i<num_freq; ++i) {
        double result = 10.0*log10( (4.0*M_PI) / sum[i] ) ;
        BOOST_CHECK_CLOSE( result, level(i), 1e-8 ) ;
    }
    cout << "Directivity index: " << level << endl ;

    delete data ;
}

BOOST_AUTO_TEST_SUITE_END()
