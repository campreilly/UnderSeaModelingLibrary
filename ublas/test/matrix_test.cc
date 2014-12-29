/** 
 * @example ublas/test/matrix_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <usml/ublas/matrix_math.h>

using namespace usml::ublas;

BOOST_AUTO_TEST_SUITE(matrix_test)

/**
 * @ingroup ublas_test
 * @{
 */

/**
 * Run a real valued matrix through the standard math routines.  Tests include:
 *
 *  - addition of matrix to itself
 *  - add scalar to matrix
 *  - subtract matrix from scalar
 *  - divide scalar by matrix
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
BOOST_AUTO_TEST_CASE( real_matrix_test ) {

    matrix<double> rmat(2,7);
    int k = 0;
    for ( size_t n=0; n < rmat.size1(); ++n ) {
        for ( size_t m=0; m < rmat.size2(); ++m ) {
            rmat(n,m) = (double) ((k++)+1);
        }
    }

    cout << "=== matrix_test: real_matrix_test ===" << endl;
    cout << "a:         " << rmat << endl;
    cout << "a+a:       " << rmat + rmat << endl;
    cout << "a+2.1:     " << rmat+2.1 << endl;
    cout << "2.1-a:     " << 2.1-rmat << endl;
    cout << "2.1/a:     " << 2.1/rmat << endl;
    cout << "a*a:       " << rmat * rmat << endl;
    cout << "a/a:       " << rmat / rmat << endl;
    cout << "max(a,3.0):   " << max(rmat,3.0) << endl;
    cout << "min(a,3.0):   " << min(rmat,3.0) << endl;
    cout << "floor(a+2.1): " << floor( rmat+2.1 ) << endl;
    cout << "ceil(a+2.1):  " << ceil( rmat+2.1 ) << endl;
    cout << "to_degrees(to_radians(a+2.1)):     " << to_degrees(to_radians(rmat+2.1)) << endl;
    cout << "to_latitude(to_colatitude(a+2.1)): " << to_latitude(to_colatitude(rmat+2.1)) << endl;

    cout << "abs(a):    " << abs(rmat) << endl;
    cout << "abs2(a):   " << abs2(rmat) << endl;
    cout << "arg(a):    " << arg(rmat) << endl;
    cout << "sqrt(a):   " << sqrt(rmat) << endl;

    cout << "cos(a):    " << cos(rmat) << endl;
    cout << "cosh(a):   " << cosh(rmat) << endl;
    cout << "sin(a):    " << sin(rmat) << endl;
    cout << "sinh(a):   " << sinh(rmat) << endl;
    cout << "tan(a):    " << tan(rmat) << endl;
    cout << "tanh(a):   " << tanh(rmat) << endl;

    cout << "exp(a):    " << exp(rmat) << endl;
    cout << "log(a):    " << log(rmat) << endl;
    cout << "log10(a):  " << log10(rmat) << endl;
}

/**
 * Run a complex valued matrix through the standard math routines.  Tests include:
 *
 *  - addition of matrix to itself
 *  - add scalar to matrix
 *  - subtract matrix from scalar
 *  - divide scalar by matrix
 *  - conversion routines for degrees/radians and latitude/colatitude
 *  - algebraic functions: abs(), abs2(), arg(), sqrt()
 *  - trig functions: cos(), cosh(), sin(), sinh(), tan(), tanh()
 *  - exponential functions: exp(), log(), log10()
 *
 * Assume that testers will visually inspect the results.
 */
BOOST_AUTO_TEST_CASE( complex_matrix_test ) {

    matrix< std::complex<double> > cmat(2,3);
    int k = 0;
    for ( size_t n=0; n < cmat.size1(); ++n ) {
        for ( size_t m=0; m < cmat.size2(); ++m ) {
            cmat(n,m) = std::complex<double>((k++)+1,1.0);
        }
    }

    cout << "=== matrix_test: complex_matrix_test ===" << endl;
    cout << "a:         " << cmat << endl;
    cout << "a+a:       " << cmat + cmat << endl;
    cout << "a+2.1:     " << cmat+2.1 << endl;
    cout << "2.1-a:     " << 2.1-cmat << endl;
    cout << "2.1/a:     " << 2.1/cmat << endl;

    cout << "abs(a):    " << abs(cmat) << endl;
    cout << "abs2(a):   " << abs2(cmat) << endl;
    cout << "arg(a):    " << arg(cmat) << endl;
    cout << "sqrt(a):   " << sqrt(cmat) << endl;

    cout << "cos(a):    " << cos(cmat) << endl;
    cout << "cosh(a):   " << cosh(cmat) << endl;
    cout << "sin(a):    " << sin(cmat) << endl;
    cout << "sinh(a):   " << sinh(cmat) << endl;
    cout << "tan(a):    " << tan(cmat) << endl;
    cout << "tanh(a):   " << tanh(cmat) << endl;

    cout << "exp(a):    " << exp(cmat) << endl;
    cout << "log(a):    " << log(cmat) << endl;
    cout << "log10(a):  " << log10(cmat) << endl;
}

/**
 * Test all of the real and complex combinations of the pow() function.
 *
 * Assume that testers will visually inspect the results.
 */
BOOST_AUTO_TEST_CASE( pow_matrix_test ) {

    matrix<double> rmat(2,3);
    matrix< std::complex<double> > cmat(2,3);
    std::complex<double> cmplx(2.5,3.5);

    int k = 0;
    for ( size_t n=0; n < cmat.size1(); ++n ) {
        for ( size_t m=0; m < cmat.size2(); ++m ) {
            rmat(n,m) = (double) (k+1);
            cmat(n,m) = std::complex<double>(k+1,1.0);
            ++k;
        }
    }

    cout << "=== matrix_test: pow_matrix_test ===" << endl;
    cout << "N:      " << rmat << endl;
    cout << "M:      " << cmat << endl;
    cout << "C:      " << cmplx << endl;

    cout << "N^3:    " << pow(rmat,3) << endl;
    cout << "N^2.5:  " << pow(rmat,2.5) << endl;
    cout << "2^N:    " << pow( 2.0, rmat ) << endl;

    cout << "M^3:    " << pow(cmat,3.0) << endl;
    cout << "M^2.5:  " << pow(cmat,2.5) << endl;
    cout << "2^M:    " << pow(2.0,cmat) << endl;

    cout << "N^N:    " << pow(rmat,rmat) << endl;
    cout << "M^M:    " << pow(cmat,cmat) << endl;
    cout << "N^M:    " << pow(rmat,cmat) << endl;
    cout << "M^N:    " << pow(cmat,rmat) << endl;
}

/**
 * Test the generation of real valued and analytic signals from
 * a matrix of arguments.
 *
 * Assume that testers will visually inspect the results.
 */
BOOST_AUTO_TEST_CASE( signal_matrix_test ) {

    matrix<double> rmat(2,3);
    int k = 0;
    for ( size_t n=0; n < rmat.size1(); ++n ) {
        for ( size_t m=0; m < rmat.size2(); ++m ) {
            rmat(n,m) = (double) ((k++)+1);
        }
    }

    cout << "=== matrix_test: signal_matrix_test ===" << endl;
    cout << "input:  " << rmat << endl;
    cout << "signal: " << signal(rmat) << endl;
    cout << "signal: " << asignal(rmat) << endl;
}

/**
 * Run real valued trig routines forward and backward to check algorithms.
 *
 * Assume that testers will visually inspect the results.
 */
BOOST_AUTO_TEST_CASE( realInverse_matrix_test ) {

    matrix<double> rmat(2,3);
    int k = 0;
    for ( size_t n=0; n < rmat.size1(); ++n ) {
        for ( size_t m=0; m < rmat.size2(); ++m ) {
            rmat(n,m) = 0.1 * (double) ((k++)+1);
        }
    }

    cout << "=== matrix_test: realInverse_matrix_test ===" << endl;
    cout << "acos(cos(t)):    " << acos(cos(rmat)) << endl;
    cout << "acosh(cosh(t)):  " << acosh(cosh(rmat)) << endl;
    cout << "asin(sin(t)):    " << asin(sin(rmat)) << endl;
    cout << "asinh(sinh(t)):  " << asinh(sinh(rmat)) << endl;
    cout << "atan(tan(t)):    " << atan(tan(rmat)) << endl;
    cout << "atanh(tanh(t)):  " << atanh(tanh(rmat)) << endl;
}

/**
 * Run complex trig routines forward and backward to check algorithms.
 *
 * Assume that testers will visually inspect the results.
 */
BOOST_AUTO_TEST_CASE( complexInverse_matrix_test ) {

    matrix< std::complex<double> > cmat(2,3);
    int k = 0;
    for ( size_t n=0; n < cmat.size1(); ++n ) {
        for ( size_t m=0; m < cmat.size2(); ++m ) {
            cmat(n,m) = 0.1 * std::complex<double>((k++)+1,1.0);
        }
    }

    cout << "=== matrix_test: complexInverse_matrix_test ===" << endl;
    cout << "acos(cos(t)):    " << acos(cos(cmat)) << endl;
    cout << "acosh(cosh(t)):  " << acosh(cosh(cmat)) << endl;
    cout << "asin(sin(t)):    " << asin(sin(cmat)) << endl;
    cout << "asinh(sinh(t)):  " << asinh(sinh(cmat)) << endl;
    cout << "atan(tan(t)):    " << atan(tan(cmat)) << endl;
    cout << "atanh(tanh(t)):  " << atanh(tanh(cmat)) << endl;
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
