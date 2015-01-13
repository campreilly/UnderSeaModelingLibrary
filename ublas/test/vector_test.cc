/** 
 * @example ublas/test/vector_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <usml/ublas/vector_math.h>

using namespace usml::ublas;

BOOST_AUTO_TEST_SUITE(vector_test)

/**
 * @ingroup ublas_test
 * @{
 */

/**
 * Run a real valued vector through the standard math routines.  Tests include:
 *
 *  - addition of vector to itself
 *  - add scalar to vector
 *  - subtract vector from scalar
 *  - divide scalar by vector
 *  - limiting functions: min, max, floor, and ceil
 *  - conversion routines for degrees/radians and latitude/colatitude
 *  - algebraic functions: abs(), abs2(), arg(), sqrt()
 *  - trig functions: cos(), cosh(), sin(), sinh(), tan(), tanh()
 *  - exponential functions: exp(), log(), log10()
 *
 * Assume that testers will visually inspect the results.
 *
 * Warning: Some compilers, at low optimization levels, fail to properly 
 * chain multiple ublas operations into a single evaluation.  
 * When this happens, the result is equal to the first operation.
 * These tests have been designed to catch this flaw.
 */
BOOST_AUTO_TEST_CASE( real_vector_test ) {

    vector<double> rvect(7);
    for ( size_t n=0; n < rvect.size(); ++n ) {
        rvect(n) = (double) (n+1);
    }

    cout << "=== vector_test: real_vector_test ===" << endl;
    cout << "a:         " << rvect << endl;
    cout << "a+a:       " << rvect + rvect << endl;
    cout << "a*2.1:     " << rvect*2.1 << endl;
    cout << "a+2.1:     " << rvect+2.1 << endl;
    cout << "2.1-a:     " << 2.1-rvect << endl;
    cout << "2.1/a:     " << 2.1/rvect << endl;
    cout << "a*a:       " << rvect * rvect << endl;
    cout << "a/a:       " << rvect / rvect << endl;
    cout << "max(a,3.0):   " << max(rvect,3.0) << endl;
    cout << "min(a,3.0):   " << min(rvect,3.0) << endl;
    cout << "floor(a+2.1): " << floor( rvect+2.1 ) << endl;
    cout << "ceil(a+2.1):  " << ceil( rvect+2.1 ) << endl;
    cout << "to_degrees(to_radians(a+2.1)):     " << to_degrees(to_radians(rvect+2.1)) << endl;
    cout << "to_latitude(to_colatitude(a+2.1)): " << to_latitude(to_colatitude(rvect+2.1)) << endl;

    cout << "abs(a):    " << abs(rvect) << endl;
    cout << "abs2(a):   " << abs2(rvect) << endl;
    cout << "arg(a):    " << arg(rvect) << endl;
    cout << "sqrt(a):   " << sqrt(rvect) << endl;

    cout << "cos(a):    " << cos(rvect) << endl;
    cout << "cosh(a):   " << cosh(rvect) << endl;
    cout << "sin(a):    " << sin(rvect) << endl;
    cout << "sinh(a):   " << sinh(rvect) << endl;
    cout << "tan(a):    " << tan(rvect) << endl;
    cout << "tanh(a):   " << tanh(rvect) << endl;

    cout << "exp(a):    " << exp(rvect) << endl;
    cout << "log(a):    " << log(rvect) << endl;
    cout << "log10(a):  " << log10(rvect) << endl;
}

/**
 * Run a complex valued vector through the standard math routines.  Tests include:
 *
 *  - addition of vector to itself
 *  - add scalar to vector
 *  - subtract vector from scalar
 *  - divide scalar by vector
 *  - min, max, floor, and ceil clipping using a scalar 
 *  - conversion routines for degrees/radians and latitude/colatitude
 *  - algebraic functions: abs(), abs2(), arg(), sqrt()
 *  - trig functions: cos(), cosh(), sin(), sinh(), tan(), tanh()
 *  - exponential functions: exp(), log(), log10()
 *
 * Assume that testers will visually inspect the results.
 *
 * Warning: Some compilers, at low optimization levels, fail to properly 
 * chain multiple ublas operations into a single evaluation.  
 * When this happens, the result is equal to the first operation.
 * These tests have been designed to catch this flaw.
 */
BOOST_AUTO_TEST_CASE( complex_vector_test ) {

    vector< std::complex<double> > cvect(7);
    for ( size_t n=0; n < cvect.size(); ++n ) {
        cvect(n) = std::complex<double>(n+1,1.0);
    }

    cout << "=== vector_test: complex_vector_test ===" << endl;
    cout << "a:         " << cvect << endl;
    cout << "a+a:       " << cvect + cvect << endl;
    cout << "a+2.1:     " << cvect+2.1 << endl;
    cout << "2.1-a:     " << 2.1-cvect << endl;

    cout << "abs(a):    " << abs(cvect) << endl;
    cout << "abs2(a):   " << abs2(cvect) << endl;
    cout << "arg(a):    " << arg(cvect) << endl;
    cout << "sqrt(a):   " << sqrt(cvect) << endl;

    cout << "cos(a):    " << cos(cvect) << endl;
    cout << "cosh(a):   " << cosh(cvect) << endl;
    cout << "sin(a):    " << sin(cvect) << endl;
    cout << "sinh(a):   " << sinh(cvect) << endl;
    cout << "tan(a):    " << tan(cvect) << endl;
    cout << "tanh(a):   " << tanh(cvect) << endl;

    cout << "exp(a):    " << exp(cvect) << endl;
    cout << "log(a):    " << log(cvect) << endl;
    cout << "log10(a):  " << log10(cvect) << endl;
}

/**
 * Test all of the real and complex combinations of the pow() function.
 *
 * Assume that testers will visually inspect the results.
 */
BOOST_AUTO_TEST_CASE( pow_vector_test ) {

    vector<double> rvect(3);
    vector< std::complex<double> > cvect(3);
    std::complex<double> cmplx(2.5,3.5);

    for ( size_t n=0; n < cvect.size(); ++n ) {
        rvect(n) = (double) (n+1);
        cvect(n) = std::complex<double>(n+1,1.0);
    }

    cout << "=== vector_test: pow_vector_test ===" << endl;
    cout << "N:      " << rvect << endl;
    cout << "M:      " << cvect << endl;
    cout << "C:      " << cmplx << endl;

    cout << "N^3:    " << pow(rvect,3) << endl;
    cout << "N^2.5:  " << pow(rvect,2.5) << endl;
    cout << "2^N:    " << pow( 2.0, rvect ) << endl;

    cout << "M^3:    " << pow(cvect,3.0) << endl;
    cout << "M^2.5:  " << pow(cvect,2.5) << endl;
    cout << "2^M:    " << pow(2.0,cvect) << endl;

    cout << "N^N:    " << pow(rvect,rvect) << endl;
    cout << "M^M:    " << pow(cvect,cvect) << endl;
    cout << "N^M:    " << pow(rvect,cvect) << endl;
    cout << "M^N:    " << pow(cvect,rvect) << endl;
}

/**
 * Test the generation of real valued and analytic signals from
 * a vector of arguments.
 *
 * Assume that testers will visually inspect the results.
 */
BOOST_AUTO_TEST_CASE( signal_vector_test ) {

    vector<double> rvect(3);
    for ( size_t n=0; n < rvect.size(); ++n ) {
        rvect(n) = (double) (n+1);
    }

    cout << "=== vector_test: signal_vector_test ===" << endl;
    cout << "input:  " << rvect << endl;
    cout << "signal: " << signal(rvect) << endl;
    cout << "signal: " << asignal(rvect) << endl;
}

/**
 * Run trig routines forward and backward to check algorithms.
 *
 * Assume that testers will visually inspect the results.
 */
BOOST_AUTO_TEST_CASE( realInverse_vector_test ) {

    vector<double> rvect(3);
    for ( size_t n=0; n < rvect.size(); ++n ) {
        rvect(n) = 0.1 * (double) (n+1);
    }

    cout << "=== vector_test: realInverse_vector_test ===" << endl;
    cout << "acos(cos(t)):    " << acos(cos(rvect)) << endl;
    cout << "acosh(cosh(t)):  " << acosh(cosh(rvect)) << endl;
    cout << "asin(sin(t)):    " << asin(sin(rvect)) << endl;
    cout << "asinh(sinh(t)):  " << asinh(sinh(rvect)) << endl;
    cout << "atan(tan(t)):    " << atan(tan(rvect)) << endl;
    cout << "atanh(tanh(t)):  " << atanh(tanh(rvect)) << endl;
}

/**
 * Run complex trig routines forward and backward to check algorithms.
 *
 * Assume that testers will visually inspect the results.
 */
BOOST_AUTO_TEST_CASE( complexInverse_vector_test ) {

    vector< std::complex<double> > cvect(3);
    for ( size_t n=0; n < cvect.size(); ++n ) {
        cvect(n) = 0.1 * std::complex<double>(n+1,1.0);
    }

    cout << "=== vector_test: complexInverse_vector_test ===" << endl;
    cout << "acos(cos(t)):    " << acos(cos(cvect)) << endl;
    cout << "acosh(cosh(t)):  " << acosh(cosh(cvect)) << endl;
    cout << "asin(sin(t)):    " << asin(sin(cvect)) << endl;
    cout << "asinh(sinh(t)):  " << asinh(sinh(cvect)) << endl;
    cout << "atan(tan(t)):    " << atan(tan(cvect)) << endl;
    cout << "atanh(tanh(t)):  " << atanh(tanh(cvect)) << endl;
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
