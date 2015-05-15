/**
 * @example types/test/sequence_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <usml/types/types.h>
#include <iostream>
#include <boost/foreach.hpp>
#include <cstdlib>
#include <iomanip>

BOOST_AUTO_TEST_SUITE(sequence_test)

using namespace boost::unit_test;
using namespace usml::types;

/**
 * @ingroup types_test
 * @{
 */

/**
 * Basic tests for a linearly spaced sequence of numbers.
 *
 *   - test (min,inc,max) form of constructor
 *   - test sequence use as a uBLAS vector
 *   - test simple accessors:
 *          use operator() form of element access
 *   - test find_index() methods:
 *          less than min, between elements, on element, greater than max,
 *          mix of float and int arguments
 *   - test conversion into a uBLAS vector
 *   - test iterator looping
 *   - separate dereferencing and pre-increment operations
 *
 * Generate errors if values differ by more that 1E-6 percent.
 */
BOOST_AUTO_TEST_CASE( sequence_linear_test1 ) {

    cout << "=== sequence_test: sequence_linear_test1() ===" << endl;

    // test (min,inc,max) form of contructor
    // test sequence use as a uBLAS vector
    seq_linear seq( 1.0, 2.0, 15.0 );
    cout << "seq                => " << seq << endl;

    // test simple accessors
    // use operator() form of element access
    cout << "size               => " << seq.size() << endl;
    BOOST_CHECK_EQUAL( seq.size(), 8u );
    cout << "increment(3)       => " << seq.increment(3) << endl;
    BOOST_CHECK_CLOSE( seq.increment(3), 2.0, 1e-6 );
    cout << "seq(3)             => " << seq(3) << endl;
    BOOST_CHECK_CLOSE( seq(3), 7.0, 1e-6 );

    // test find_index() methods
    // less than min, between elements, on element, greater than max
    // mix of float and int arguments
    cout << "find_index(-2)     => " << seq.find_index(-2) << endl;
    BOOST_CHECK_EQUAL( seq.find_index(-2), 0u );
    cout << "find_index(11.5)   => " << seq.find_index(11.5) << endl;
    BOOST_CHECK_EQUAL( seq.find_index(11.5), 5u );
    cout << "find_index(13)     => " << seq.find_index(13) << endl;
    BOOST_CHECK_EQUAL( seq.find_index(13), 6u );
    cout << "find_index(100.0)  => " << seq.find_index(100.0) <<endl;
    BOOST_CHECK_EQUAL( seq.find_index(100.0), 6u );

    // test iterator looping
    // separate dereferencing and pre-increment operations
    seq_linear::iterator current = seq.begin();
    cout << "iterator           => ";
    while ( current != seq.end() ) {
        cout << *current << " ";
        ++current;
    }
    cout << endl;

    // test conversion into a uBLAS vector
    vector<double> vect;
    vect = seq;
    cout << "vect               => " << vect << endl;
}

/**
 * Alternate tests for a linearly spaced sequence of numbers.
 *
 *   - test (min,inc,number) form of constructor
 *   - test sequence use as a uBLAS vector
 *   - test simple accessors:
 *          use operator[] form of element access
 *   - test find_index() methods:
 *          less than min, between elements, on element, greater than max,
 *          mix of float and int arguments
 *   - test iterator looping:
 *          integrate dereferencing and post-increment operations
 *
 * Generate errors if values differ by more that 1E-6 percent.
 */
BOOST_AUTO_TEST_CASE( sequence_linear_test2 ) {

    cout << "=== sequence_test: sequence_linear_test2() ===" << endl;

    // test (min,inc,number) form of contructor
    // test use as a uBLAS vector
    seq_linear seq( 1.0, 3.5, 8 );
    cout << "seq                => " << seq << endl;

    // test simple accessors
    // use operator[] form of element access
    cout << "size               => " << seq.size() << endl;
    BOOST_CHECK_EQUAL( seq.size(), 8u );
    cout << "increment(3)       => " << seq.increment(3) << endl;
    BOOST_CHECK_CLOSE( seq.increment(3), 3.5, 1e-6 );
    cout << "seq[3]             => " << seq[3] << endl;
    BOOST_CHECK_CLOSE( seq[3], 11.5, 1e-6 );

    // test find_index() methods
    // less than min, between elements, on element, greater than max
    // mix of float and int arguments
    cout << "find_index(-2)     => " << seq.find_index(-2) << endl;
    BOOST_CHECK_EQUAL( seq.find_index(-2), 0u );
    cout << "find_index(11.5)   => " << seq.find_index(11.5) << endl;
    BOOST_CHECK_EQUAL( seq.find_index(11.5), 3u );
    cout << "find_index(13)     => " << seq.find_index(13) << endl;
    BOOST_CHECK_EQUAL( seq.find_index(13), 3u );
    cout << "find_index(100.0)  => " << seq.find_index(100.0) <<endl;
    BOOST_CHECK_EQUAL( seq.find_index(100.0), 6u );

    // test iterator looping
    // integrate dereferencing and post-increment operations
    seq_linear::iterator current = seq.begin();
    cout << "iterator           => ";
    while ( current != seq.end() ) {
        cout << *current++ << " ";
    }
    cout << endl;
}

/**
 * Basic tests for a logarithmically spaced sequence of numbers.
 *
 *   - test (min,inc,number) form of constructor (third octave bands)
 *   - test sequence use as a uBLAS vector
 *   - test simple accessors:
 *          use operator[] form of element access
 *   - test find_index() methods:
 *          less than min, between elements, on element, greater than max,
 *          mix of float and int arguments,
 *          note problem when searching exactly on element
 *   - test iterator looping:
 *          integrate dereferencing and post-increment operations
 *
 * Generate errors if values differ by more that 1E-6 percent, except for
 * the increment test which is only required to be within 1E-4 percent.
 */
BOOST_AUTO_TEST_CASE( sequence_log_test ) {

    cout << "=== sequence_test: sequence_log_test ===" << endl;

    // test (min,inc,number) form of contructor (third octave bands)
    // test use as a uBLAS vector
    seq_log seq( 10.0, std::pow(2.0,1.0/3.0), 10 );
    cout << "seq                => " << seq << endl;

    // test simple accessors
    // use operator[] form of element access
    cout << "size               => " << seq.size() << endl;
    BOOST_CHECK_EQUAL( seq.size(), 10u );
    cout << "increment(3)       => " << seq.increment(3) << endl;
    BOOST_CHECK_CLOSE( seq.increment(3), 5.19842, 1e-4 );
    cout << "seq[3]             => " << seq[3] << endl;
    BOOST_CHECK_CLOSE( seq[3], 20.0, 1e-6 );

    // test find_index() methods
    // less than min, between elements, on element, greater than max
    // mix of float and int arguments
    // note problem when searching exactly on element
    cout << "find_index(-2)     => " << seq.find_index(-2) << endl;
    BOOST_CHECK_EQUAL( seq.find_index(-2), 0u );
    cout << "find_index(13)     => " << seq.find_index(13) << endl;
    BOOST_CHECK_EQUAL( seq.find_index(13), 1u );
    cout << "find_index(20.0)   => " << seq.find_index(20.0) << endl;
    cout << "find_index(20.0+e) => " << seq.find_index(20.0+1e-10) << endl;
    BOOST_CHECK_EQUAL( seq.find_index(20.0+1e-10), 3u );
    cout << "find_index(1000.0) => " << seq.find_index(1000.0) <<endl;
    BOOST_CHECK_EQUAL( seq.find_index(1000.0), 8u );

    // test iterator looping
    // integrate dereferencing and post-increment operations
    seq_log::iterator current = seq.begin();
    cout << "iterator           => ";
    while ( current != seq.end() ) {
        cout << *current++ << " ";
    }
    cout << endl;
}

/**
 * Basic test for a sequence defined by an unevenly spaced vector of points.
 *
 *   - test data input form of constructor
 *   - test sequence use as a uBLAS vector
 *   - test simple accessors:
 *          use operator[] form of element access
 *   - test find_index() methods:
 *          less than min, between elements, on element, greater than max,
 *          mix of float and int arguments,
 *          note problem when searching exactly on element
 *   - test iterator looping:
 *          integrate dereferencing and post-increment operations
 *
 * Generate errors if values differ by more that 1E-6 percent, except for
 * the increment test which is only required to be within 1e-4 percent.
 */
BOOST_AUTO_TEST_CASE( sequence_data_test1 ) {

    cout << "=== sequence_test: sequence_data_test1() ===" << endl;

    // test data input form of contructor
    // test use as a uBLAS vector
    int N = 10;
    double data[] = {10.0,12.5,16.0,20.0,25.0,32.0,40.0,50.0,63.5,80.0};
    vector<double> vect(N);
    std::copy( data, data+N, vect.begin() );

//    seq_data seq( vect-0.1 );
    seq_data seq( vect );
    cout << "seq                => " << seq << endl;

    // test simple accessors
    // use operator[] form of element access
    cout << "size               => " << seq.size() << endl;
    BOOST_CHECK_EQUAL( seq.size(), 10u );
    cout << "increment(3)       => " << seq.increment(3) << endl;
    BOOST_CHECK_CLOSE( seq.increment(3), 5.0, 1e-4 );
    cout << "seq[3]             => " << seq[3] << endl;
    BOOST_CHECK_CLOSE( seq[3], 20.0, 1e-6 );

    // test find_index() methods
    // less than min, between elements, on element, greater than max
    // mix of float and int arguments
    // note problem when searching exactly on element
    cout << "find_index(-2)     => " << seq.find_index(-2) << endl;
    BOOST_CHECK_EQUAL( seq.find_index(-2), 0u );
    cout << "find_index(13)     => " << seq.find_index(13) << endl;
    BOOST_CHECK_EQUAL( seq.find_index(13), 1u );
    cout << "find_index(20.0)   => " << seq.find_index(20.0) << endl;
    BOOST_CHECK_EQUAL( seq.find_index(20.0), 3u );
    cout << "find_index(1000.0) => " << seq.find_index(1000.0) <<endl;
    BOOST_CHECK_EQUAL( seq.find_index(1000.0), 8u );

    // test iterator looping
    // integrate dereferencing and post-increment operations
    seq_data::iterator current = seq.begin();
    cout << "iterator           => ";
    while ( current != seq.end() ) {
        cout << *current++ << " ";
    }
    cout << endl;
}

/**
 * Alternate test for a sequence defined by an unevenly spaced vector of points.
 *
 *   - test decreasing sequence form of constructor
 *   - test sequence use as a uBLAS vector
 *   - test simple accessors:
 *          use operator[] form of element access
 *   - test find_index() methods:
 *          less than min, between elements, on element, greater than max,
 *          mix of float and int arguments,
 *          note problem when searching exactly on element
 *   - test iterator looping:
 *          integrate dereferencing and post-increment operations
 *
 * Generate errors if values differ by more that 1E-6 percent, except for
 * the increment test which is only required to be within 1e-4 percent.
 */
BOOST_AUTO_TEST_CASE( sequence_data_test2 ) {

    cout << "=== sequence_test: sequence_data_test2() ===" << endl;

    // test decreasing sequence form of contructor
    // test use as a uBLAS vector
    int N = 10;
    double data[] = {80.0,63.5,50.0,40.0,32.0,25.0,20.0,16.0,12.5,10.0};
    vector<double> vect(N);
    std::copy( data, data+N, vect.begin() );

    seq_data seq( vect );
    cout << "seq                => " << seq << endl;

    // test simple accessors
    // use operator[] form of element access
    cout << "size               => " << seq.size() << endl;
    BOOST_CHECK_EQUAL( seq.size(), 10u );
    cout << "increment(3)       => " << seq.increment(3) << endl;
    BOOST_CHECK_CLOSE( seq.increment(3), -8.0, 1e-4 );
    cout << "seq[3]             => " << seq[3] << endl;
    BOOST_CHECK_CLOSE( seq[3], 40.0, 1e-6 );

    // test find_index() methods
    // less than min, between elements, on element, greater than max
    // mix of float and int arguments
    // note problem when searching exactly on element
    cout << "find_index(-2)     => " << seq.find_index(-2) << endl;
    BOOST_CHECK_EQUAL( seq.find_index(-2), 8u );
    cout << "find_index(13)     => " << seq.find_index(13) << endl;
    BOOST_CHECK_EQUAL( seq.find_index(13), 7u );
    cout << "find_index(20.0)   => " << seq.find_index(20.0) << endl;
    BOOST_CHECK_EQUAL( seq.find_index(20.0), 6u );
    cout << "find_index(1000.0) => " << seq.find_index(1000.0) <<endl;
    BOOST_CHECK_EQUAL( seq.find_index(1000.0), 0u );

    // test iterator looping
    // integrate dereferencing and post-increment operations
    seq_data::iterator current = seq.begin();
    cout << "iterator           => ";
    while ( current != seq.end() ) {
        cout << *current++ << " ";
    }
    cout << endl;
}

/**
 * Test for a sequence defined by a single point.
 *
 *   - test data set size of one
 *   - test simple accessors:
 *          use operator[] form of element access
 *   - test find_index() methods:
 *          less than min, between elements, on element, greater than max,
 *          mix of float and int arguments,
 *          note problem when searching exactly on element
 *   - test iterator looping:
 *          integrate dereferencing and post-increment operations
 *
 * Generate errors if values differ by more that 1E-6 percent, except for
 * the increment test which is only required to be within 1e-4 percent.
 */
BOOST_AUTO_TEST_CASE( sequence_data_test3 ) {

    cout << "=== sequence_test: sequence_data_test3() ===" << endl;

    // test decreasing sequence form of constructor
    // test use as a uBLAS vector
    int N = 1;
    double data[] = {123.5};
    vector<double> vect(N);
    std::copy( data, data+N, vect.begin() );

    seq_data seq( vect );
    cout << "seq                => " << seq << endl;

    // test simple accessors
    // use operator[] form of element access
    cout << "size               => " << seq.size() << endl;
    BOOST_CHECK_EQUAL( seq.size(), 1u );
    cout << "increment(0)       => " << seq.increment(0) << endl;
    BOOST_CHECK_CLOSE( seq.increment(0), 0.0, 1e-4 );
    cout << "seq[0]             => " << seq[0] << endl;
    BOOST_CHECK_CLOSE( seq[0], 123.5, 1e-6 );

    // test find_index() methods
    // less than min, between elements, on element, greater than max
    // mix of float and int arguments
    // note problem when searching exactly on element
    cout << "find_index(123.5)  => " << seq.find_index(123.5) << endl;
    BOOST_CHECK_EQUAL( seq.find_index(123.5), 0u );
    cout << "find_index(13)     => " << seq.find_index(13) << endl;
    BOOST_CHECK_EQUAL( seq.find_index(13), 0u );
    cout << "find_index(1000.0) => " << seq.find_index(1000.0) <<endl;
    BOOST_CHECK_EQUAL( seq.find_index(1000.0), 0u );

    // test iterator looping
    // integrate dereferencing and post-increment operations
    seq_data::iterator current = seq.begin();
    cout << "iterator           => ";
    while ( current != seq.end() ) {
        cout << *current++ << " ";
    }
    cout << endl;
}

/**
 * Basic tests for a gaussian spaced sequence of angles.
 *
 *   - test (min,max,number) form of constructor (7 beams)
 *   - test sequence use as a uBLAS vector
 *   - test iterator looping:
 *          integrate dereferencing and post-increment operations
 *
 * Generate errors if values differ by more that 1E-6 percent, except for
 * the increment test which is only required to be within 1e-4 percent.
 */
BOOST_AUTO_TEST_CASE( sequence_rayfan_test ) {

    cout << "=== sequence_test: sequence_rayfan_test ===" << endl;

    // test (min,max,number) form of contructor
    // test use as a uBLAS vector
    seq_rayfan seq( -90.0, 90.0, 7, 10.0 );
    cout << "seq                => " << seq << endl;

    // test iterator looping
    // integrate dereferencing and post-increment operations
    seq_log::iterator current = seq.begin();
    cout << "iterator           => ";
    while ( current != seq.end() ) {
        cout << *current++ << " ";
    }
    cout << endl;
}

/**
 * Tests the functionality of BOOST_FOREACH with all
 * sequence classes. Because of the inheritance chain
 * of seq_vectors, boost was unable to deduce the type
 * of iterator that was appropriate for the seq_vector
 * classes provided. Creating specialized template typedefs
 * for each subclass of seq_vector resolves this issue.
 */
BOOST_AUTO_TEST_CASE( sequence_foreach_test ) {

    using namespace boost ;

    cout << "=== sequence_test: sequence_foreach_test ===" << endl ;
    bool complete = false ;
    size_t N = 5 ;

    // Tests foreach for rayfan
    seq_rayfan fan( -5.0, 5.0, N ) ;
    cout << "seq_rayfan: " << fan << endl ;
    cout << "using foreach: " ;
    BOOST_FOREACH( double i, fan )
        complete = false, cout << i << " ", complete = true ;
    cout << endl ;
    BOOST_CHECK( complete ) ;

    // Tests foreach for linear
    seq_linear line( -5.0, 5.0, N ) ;
    cout << "seq_linear: " << line << endl ;
    cout << "using foreach: " ;
    BOOST_FOREACH( double i, line )
        complete = false, cout << i << " ", complete = true ;
    cout << endl ;
    BOOST_CHECK( complete ) ;

    // Tests foreach for log
    seq_log log( 1.0, 5.0, N ) ;
    cout << "seq_log: " << log << endl ;
    cout << "using foreach: " ;
    BOOST_FOREACH( double i, log )
        complete = false, cout << i << " ", complete = true ;
    cout << endl ;
    BOOST_CHECK( complete ) ;

    // Tests foreach for data
    std::srand( 1 ) ;
    double* sample = new double[N] ;
    for(size_t i=0; i<N; ++i)
        sample[i] = ((double)std::rand() / RAND_MAX) * 100.0 ;
    std::sort( sample, sample+N ) ;
    seq_data data( sample, N ) ;
    cout << "seq_data: " << data << endl ;
    cout << "using foreach: " ;
    BOOST_FOREACH( double i, data )
        complete = false, cout << i << " ", complete = true ;
    cout << endl ;
    BOOST_CHECK( complete ) ;
    delete[] sample ;
}

/**
 * Tests the uBLas capabilities of seq_vectors
 */
BOOST_AUTO_TEST_CASE( seq_ublas_test ) {
    cout << "=== sequence_test/seq_ublas_test ===" << endl ;

    // Tests the use of seq_linear
    seq_linear line( -5.0, 1.0, 5.0 ) ;
    size_t n( line.size() ) ;
    vector<double> result(n) ;
    noalias(result) = 6.0 * line ;
    size_t count = 0 ;
    BOOST_FOREACH( double i, result )
        BOOST_CHECK_EQUAL( i, 6.0*line(count++) ) ;
    cout << "result_linear: " << result << endl ;

    // Tests the use of seq_log
    seq_log log( 1.0, 10.0, n ) ;
    result = 2.0 * log10( log ) ;
    count = 0 ;
    BOOST_FOREACH( double i, result )
        BOOST_CHECK_EQUAL( i, 2.0*log10(log(count++)) ) ;
    cout << "result_log: " << result << endl ;

    // Tests the use of seq_rayfan
    seq_rayfan fan( -90.0, 90.0, n ) ;
    result = atan( fan ) / 2.0 ;
    count = 0 ;
    BOOST_FOREACH( double i, result )
        BOOST_CHECK_EQUAL( i, (atan( fan(count++) ) / 2.0) ) ;
    cout << "result_rayfan: " << result << endl ;

    // Tests the use of seq_data
    std::srand(1) ;
    double* d = new double[n] ;
    for(size_t i=0; i<n; ++i)
        d[i] = ((double)std::rand() / RAND_MAX ) * 100.0 ;
    std::sort( d, d+n ) ;
    seq_data sdata( d, n ) ;
    result = element_div(element_prod(exp(sdata), line), log) ;
    count = 0 ;
    double tmp = 0 ;
    BOOST_FOREACH( double i, result )
    {
        tmp = exp(sdata(count)) * line(count) / log(count) ;
        ++count ;
        BOOST_CHECK_EQUAL( i, tmp ) ;
    }
    cout << "result_data: " << result << endl ;
    delete[] d ;
}

/**
 * Tests the seq_vector::clip method
 * Test fails if any the clipped values are
 * not equal to the predetermined correct values.
 */
BOOST_AUTO_TEST_CASE( seq_vector_clip_test ) {
    cout << "=== sequence_test/seq_vector_clip_test ===" << endl ;

     // 6.5K, 7.5K, 8.5K, 9.5K
    seq_linear values(6500.0, 1000.0, 4);

    cout << "original values: " << values << endl ;

    // Max Clip
    seq_vector* clipped_values_one = values.clip(5000.0, 9000.0);

    cout << "after max clip :  " <<  *clipped_values_one << endl ;
    BOOST_CHECK_EQUAL( clipped_values_one->size(), 3 ) ;

    double test_value = (*clipped_values_one)(0);
    BOOST_CHECK_EQUAL( test_value, 6500 ) ;
    test_value = (*clipped_values_one)(1);
    BOOST_CHECK_EQUAL( test_value, 7500 ) ;
    test_value = (*clipped_values_one)(2);
    BOOST_CHECK_EQUAL( test_value, 8500 ) ;
    delete clipped_values_one;

    // Min clip
    seq_vector* clipped_values_two = values.clip(7000.0, 11000.0);

    cout << "after min clip:  " << *clipped_values_two << endl ;
    BOOST_CHECK_EQUAL( clipped_values_two->size(), 3 ) ;

    test_value = (*clipped_values_two)(0);
    BOOST_CHECK_EQUAL( test_value, 7500 ) ;
    test_value = (*clipped_values_two)(1);
    BOOST_CHECK_EQUAL( test_value, 8500 ) ;
    test_value = (*clipped_values_two)(2);
    BOOST_CHECK_EQUAL( test_value, 9500 ) ;
    delete clipped_values_two;


    // Max and Min clip
    seq_vector* clipped_values_three = values.clip(7000.0, 9000.0);

    cout << "after max and min clip:  " << *clipped_values_three << endl ;
    BOOST_CHECK_EQUAL( clipped_values_three->size(), 2 ) ;

    test_value = (*clipped_values_three)(0);
    BOOST_CHECK_EQUAL( test_value, 7500 ) ;
    test_value = (*clipped_values_three)(1);
    BOOST_CHECK_EQUAL( test_value, 8500 ) ;
    delete clipped_values_three;
}

/**
 * Tests the implementation of seq_augment
 * Test fails if any values are unequal to the
 * predetermined truth vector.
 */
BOOST_AUTO_TEST_CASE( seq_augment_test ) {
    cout << "=== sequence_test/seq_augment_test ===" << endl ;

    //seq_linear origin( 6.0, 1.0, 5 ) ;
    seq_rayfan origin(6.0, 10.0, 5);
    size_t N = 6 ;

    //double tmp[] = { 6.0, 6.25, 6.5, 6.75, 7.0, 8.0, 9.0, 9.25, 9.5, 9.75, 10.0 } ;
    double tmp[]= { 6.0,6.19598,6.39196,6.58795,6.78393,7.68466,8.73893,8.93492,9.1309,9.32688,10.0 };
    size_t size( sizeof(tmp)/sizeof(double) ) ;
    vector<double> data( size, 0.0 ) ;
    size_t index = 0 ;
    BOOST_FOREACH( double& i, data )
        i = tmp[index++] ;
    seq_data truth( data ) ;

    seq_augment aug( &origin, N) ;
    cout << std::setprecision(8);
    cout << "origin: " << origin << endl ;
    cout << "augment: " << aug << endl ;
    index = 0 ;
    BOOST_FOREACH( double i, aug )
        BOOST_CHECK_CLOSE( i, tmp[index++], 0.0001 ) ;
}

/**
 * Tests the implementation of seq_vector operator==
 * Test fails if equal seq_vector's are not found false, or
 * unequal seq_vector's are found true.
 */
BOOST_AUTO_TEST_CASE( seq_vector_equal_test ) {
    cout << "=== sequence_test/seq_vector_equal_test ===" << endl ;

    seq_linear linear( 6.0, 1.0, 5 ) ;

    seq_rayfan rayfan_five( 6.0, 10.0, 5);

    if (linear == rayfan_five) {
        BOOST_FAIL("seq_vector_compare_test are not equal ");
    }

    seq_rayfan rayfan_six( 6.0, 10.0, 6);

    if (rayfan_five == rayfan_six) {
        BOOST_FAIL("seq_vector_compare_test are not equal ");
    }

    // Same should be equal
    seq_rayfan rayfan( 6.0, 10.0, 6);

    if (!(rayfan == rayfan_six)) {
        BOOST_FAIL("seq_vector_compare_test are equal ");
    }
}

BOOST_AUTO_TEST_SUITE_END()
