/** 
 * @example ublas/test/matrix_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <usml/ublas/matrix_math.h>
#include <usml/ublas/vector_math.h>
#include <usml/ublas/test/matrix_test_support.h>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>

using namespace usml::ublas;
using namespace matrix_test ;

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

    matrix<double> result ;
    cout << "=== matrix_test: real_matrix_test ===" << endl;
    cout << "a:         " << rmat << endl;
    cout << "a+a:       " << rmat+rmat << endl;
    USML_MATRIX_REAL_TESTER( rmat+rmat, rmat, add_helper<double> ) ;
    cout << "a+2.1:     " << rmat+2.1 << endl;
    USML_MATRIX_REAL_TESTER( rmat+2.1, rmat, scalar2_add_helper<double> ) ;
    cout << "2.1-a:     " << 2.1-rmat << endl;
    USML_MATRIX_REAL_TESTER( 2.1-rmat, rmat, scalar1_minus_helper<double> ) ;
    cout << "2.1/a:     " << 2.1/rmat << endl;
    USML_MATRIX_REAL_TESTER( 2.1/rmat, rmat, scalar1_divide_helper<double> ) ;
    cout << "a*a:       " << rmat * rmat << endl;
    USML_MATRIX_REAL_TESTER( rmat*rmat, rmat, multiply_helper<double> ) ;
    cout << "a/a:       " << rmat / rmat << endl;
    USML_MATRIX_REAL_TESTER( rmat/rmat, rmat, divide_helper<double> ) ;
    cout << "max(a,3.0):   " << max(rmat,3.0) << endl;
    USML_MATRIX_REAL_TESTER( max(rmat,3.0), rmat, max_helper<double> ) ;
    cout << "min(a,3.0):   " << min(rmat,3.0) << endl;
    USML_MATRIX_REAL_TESTER( min(rmat,3.0), rmat, min_helper<double> ) ;
    cout << "floor(a+2.1): " << floor(rmat+2.1) << endl;
    USML_MATRIX_REAL_TESTER( floor(rmat+2.1), rmat, floor_helper<double> ) ;
    cout << "ceil(a+2.1):  " << ceil(rmat+2.1) << endl;
    USML_MATRIX_REAL_TESTER( ceil(rmat+2.1), rmat, ceil_helper<double> ) ;
    cout << "to_degrees(to_radians(a+2.1)):     " << to_degrees(to_radians(rmat+2.1)) << endl;
    USML_MATRIX_REAL_TESTER( to_degrees(to_radians(rmat+2.1)), rmat, scalar2_add_helper<double>) ;
    cout << "to_latitude(to_colatitude(a+2.1)): " << to_latitude(to_colatitude(rmat+2.1)) << endl;
    USML_MATRIX_REAL_TESTER( to_latitude(to_colatitude(rmat+2.1)), rmat, scalar2_add_helper<double>) ;

    cout << "abs(a):    " << abs(rmat) << endl;
    USML_MATRIX_REAL_TESTER( abs(rmat), rmat, std::abs<double> ) ;
    cout << "abs2(a):   " << abs2(rmat) << endl;
    USML_MATRIX_REAL_TESTER( abs2(rmat), rmat, abs2_helper<double> ) ;
    cout << "arg(a):    " << arg(rmat) << endl;
    USML_MATRIX_REAL_TESTER( arg(rmat), rmat, zero_helper<double> ) ;
    cout << "sqrt(a):   " << sqrt(rmat) << endl;
    USML_MATRIX_REAL_TESTER( sqrt(rmat), rmat, std::sqrt<double> ) ;

    cout << "cos(a):    " << cos(rmat) << endl;
    USML_MATRIX_REAL_TESTER( cos(rmat), rmat, std::cos<double> ) ;
    cout << "cosh(a):   " << cosh(rmat) << endl;
    USML_MATRIX_REAL_TESTER( cosh(rmat), rmat, std::cosh<double> ) ;
    cout << "sin(a):    " << sin(rmat) << endl;
    USML_MATRIX_REAL_TESTER( sin(rmat), rmat, std::sin<double> ) ;
    cout << "sinh(a):   " << sinh(rmat) << endl;
    USML_MATRIX_REAL_TESTER( sinh(rmat), rmat, std::sinh<double> ) ;
    cout << "tan(a):    " << tan(rmat) << endl;
    USML_MATRIX_REAL_TESTER( tan(rmat), rmat, std::tan<double> ) ;
    cout << "tanh(a):   " << tanh(rmat) << endl;
    USML_MATRIX_REAL_TESTER( tanh(rmat), rmat, std::tanh<double> ) ;

    cout << "exp(a):    " << exp(rmat) << endl;
    USML_MATRIX_REAL_TESTER( exp(rmat), rmat, std::exp<double> ) ;
    cout << "log(a):    " << log(rmat) << endl;
    USML_MATRIX_REAL_TESTER( log(rmat), rmat, std::log<double> ) ;
    cout << "log10(a):  " << log10(rmat) << endl;
    USML_MATRIX_REAL_TESTER( log10(rmat), rmat, std::log10<double> ) ;
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
    USML_MATRIX_COMPLEX_TESTER( cmat+cmat, cmat, add_helper<complex<double> > ) ;
    cout << "a+2.1:     " << cmat+2.1 << endl;
    USML_MATRIX_COMPLEX_TESTER( cmat+2.1, cmat, scalar2_add_helper<complex<double> > ) ;
    cout << "2.1-a:     " << 2.1-cmat << endl;
    USML_MATRIX_COMPLEX_TESTER( 2.1-cmat, cmat, scalar1_minus_helper<complex<double> > ) ;
    cout << "2.1/a:     " << 2.1/cmat << endl;
    USML_MATRIX_COMPLEX_TESTER( 2.1/cmat, cmat, scalar1_divide_helper<complex<double> > ) ;

    cout << "abs(a):    " << abs(cmat) << endl;
    USML_MATRIX_COMPLEX_TESTER( abs(cmat), cmat, abs_helper<complex<double> > ) ;
    cout << "abs2(a):   " << abs2(cmat) << endl;
    USML_MATRIX_COMPLEX_TESTER( abs2(cmat), cmat, abs2_helper<complex<double> > ) ;
    cout << "arg(a):    " << arg(cmat) << endl;
    USML_MATRIX_COMPLEX_TESTER( arg(cmat), cmat, arg_helper<double> ) ;
    cout << "sqrt(a):   " << sqrt(cmat) << endl;
    USML_MATRIX_COMPLEX_TESTER( sqrt(cmat), cmat, std::sqrt<double> ) ;

    cout << "cos(a):    " << cos(cmat) << endl;
    USML_MATRIX_COMPLEX_TESTER( cos(cmat), cmat, std::cos<double> ) ;
    cout << "cosh(a):   " << cosh(cmat) << endl;
    USML_MATRIX_COMPLEX_TESTER( cosh(cmat), cmat, std::cosh<double> ) ;
    cout << "sin(a):    " << sin(cmat) << endl;
    USML_MATRIX_COMPLEX_TESTER( sin(cmat), cmat, std::sin<double> ) ;
    cout << "sinh(a):   " << sinh(cmat) << endl;
    USML_MATRIX_COMPLEX_TESTER( sinh(cmat), cmat, std::sinh<double> ) ;
    cout << "tan(a):    " << tan(cmat) << endl;
    USML_MATRIX_COMPLEX_TESTER( tan(cmat), cmat, std::tan<double> ) ;
    cout << "tanh(a):   " << tanh(cmat) << endl;
    USML_MATRIX_COMPLEX_TESTER( tanh(cmat), cmat, std::tanh<double> ) ;

    cout << "exp(a):    " << exp(cmat) << endl;
    USML_MATRIX_COMPLEX_TESTER( exp(cmat), cmat, std::exp<double> ) ;
    cout << "log(a):    " << log(cmat) << endl;
    USML_MATRIX_COMPLEX_TESTER( log(cmat), cmat, std::log<double> ) ;
    cout << "log10(a):  " << log10(cmat) << endl;
    USML_MATRIX_COMPLEX_TESTER( log10(cmat), cmat, std::log10<double> ) ;
}

/**
 * Test all of the real and complex combinations of the pow() function.
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
    USML_MATRIX_POW_SCALAR2_TESTER( pow(rmat,3), rmat, 3 ) ;
    cout << "N^2.5:  " << pow(rmat,2.5) << endl;
    USML_MATRIX_POW_SCALAR2_TESTER( pow(rmat,2.5), rmat, 2.5 ) ;
    cout << "2^N:    " << pow( 2.0, rmat ) << endl;
    USML_MATRIX_POW_SCALAR1_TESTER( pow(2.0,rmat), 2.0, rmat ) ;

    cout << "M^3:    " << pow(cmat,3.0) << endl;
    USML_MATRIX_POW_SCALAR2_TESTER( pow(cmat,3.0), cmat, 3.0 ) ;
    cout << "M^2.5:  " << pow(cmat,2.5) << endl;
    USML_MATRIX_POW_SCALAR2_TESTER( pow(cmat,2.5), cmat, 2.5 ) ;
    cout << "2^M:    " << pow(2.0,cmat) << endl;
    USML_MATRIX_POW_SCALAR1_TESTER( pow(2.0,cmat), 2.0, cmat ) ;

    cout << "N^N:    " << pow(rmat,rmat) << endl;
    USML_MATRIX_POW_TESTER( pow(rmat,rmat), rmat, rmat ) ;
    cout << "M^M:    " << pow(cmat,cmat) << endl;
    USML_MATRIX_POW_TESTER( pow(cmat,cmat), cmat, cmat ) ;
    cout << "N^M:    " << pow(rmat,cmat) << endl;
    USML_MATRIX_POW_TESTER( pow(rmat,cmat), rmat, cmat ) ;
    cout << "M^N:    " << pow(cmat,rmat) << endl;
    USML_MATRIX_POW_TESTER( pow(cmat,rmat), cmat, rmat ) ;
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
    USML_MATRIX_REAL_TESTER( signal(rmat), rmat, signal_helper<double> ) ;
    cout << "signal: " << asignal(rmat) << endl;
    USML_MATRIX_CR_TESTER( asignal(rmat), rmat, asignal_helper<double> ) ;
}

/**
 * Run real valued trig routines forward and backward to check algorithms.
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
    USML_MATRIX_REAL_TESTER( acos(cos(rmat)), rmat, identity<double> ) ;
    cout << "acosh(cosh(t)):  " << acosh(cosh(rmat)) << endl;
    USML_MATRIX_REAL_TESTER( acosh(cosh(rmat)), rmat, identity<double> ) ;
    cout << "asin(sin(t)):    " << asin(sin(rmat)) << endl;
    USML_MATRIX_REAL_TESTER( asin(sin(rmat)), rmat, identity<double> ) ;
    cout << "asinh(sinh(t)):  " << asinh(sinh(rmat)) << endl;
    USML_MATRIX_REAL_TESTER( asinh(sinh(rmat)), rmat, identity<double> ) ;
    cout << "atan(tan(t)):    " << atan(tan(rmat)) << endl;
    USML_MATRIX_REAL_TESTER( atan(tan(rmat)), rmat, identity<double> ) ;
    cout << "atanh(tanh(t)):  " << atanh(tanh(rmat)) << endl;
    USML_MATRIX_REAL_TESTER( atanh(tanh(rmat)), rmat, identity<double> ) ;
}

/**
 * Run complex trig routines forward and backward to check algorithms.
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
    USML_MATRIX_COMPLEX_TESTER( acos(cos(cmat)), cmat, identity<complex<double> > ) ;
    cout << "acosh(cosh(t)):  " << acosh(cosh(cmat)) << endl;
    USML_MATRIX_COMPLEX_TESTER( acosh(cosh(cmat)), cmat, identity<complex<double> > ) ;
    cout << "asin(sin(t)):    " << asin(sin(cmat)) << endl;
    USML_MATRIX_COMPLEX_TESTER( asin(sin(cmat)), cmat, identity<complex<double> > ) ;
    cout << "asinh(sinh(t)):  " << asinh(sinh(cmat)) << endl;
    USML_MATRIX_COMPLEX_TESTER( asinh(sinh(cmat)), cmat, identity<complex<double> > ) ;
    cout << "atan(tan(t)):    " << atan(tan(cmat)) << endl;
    USML_MATRIX_COMPLEX_TESTER( atan(tan(cmat)), cmat, identity<complex<double> > ) ;
    cout << "atanh(tanh(t)):  " << atanh(tanh(cmat)) << endl;
    USML_MATRIX_COMPLEX_TESTER( atanh(tanh(cmat)), cmat, identity<complex<double> > ) ;
}

/**
* Test the ability to use uBLAS matrix proxies.
* If successful the first digit of each entry should be 1,
* the second digit should be the row number, and
* the third digit should be the column number.
*/
BOOST_AUTO_TEST_CASE(ublas_proxy_test) {
	cout << "=== matrix_test: ublas_proxy_test ===" << endl;

	// fill a vector wit column numbers

	vector<double> v(3);
	for (unsigned i = 0; i < v.size(); ++i) {
		v(i) = i + 1;
	}

	// initialize matrix to all values of 100.0

	matrix<double> m(3, 3);
	m = matrix<double>(3, 3, 100.0);

	// add column and row number to existing matrix data

	for (unsigned i = 0; i < m.size1(); ++i) {
		matrix_row< matrix<double> > mr(m, i);
		mr += v + 10 * (i + 1);
	}
	std::cout << m << std::endl;
	BOOST_CHECK_CLOSE(m(2,2), 133, 1e-10);
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
