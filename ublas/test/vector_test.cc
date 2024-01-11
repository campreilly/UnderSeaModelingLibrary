/**
 * @example ublas/test/vector_test.cc
 */
#include <usml/ublas/test/vector_test_support.h>
#include <usml/ublas/vector_math.h>

#include <boost/test/unit_test.hpp>

using namespace usml::ublas;
using namespace usml::ublas::vector_test;

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
 * Warning: Some compilers, at low optimization levels, fail to properly
 * chain multiple ublas operations into a single evaluation.
 * When this happens, the result is equal to the first operation.
 * These tests have been designed to catch this flaw.
 */
BOOST_AUTO_TEST_CASE(real_vector_test) {
  vector<double> rvect(7);
  for (size_t n = 0; n < rvect.size(); ++n) {
    rvect(n) = static_cast<double>(n + 1);
  }

  cout << "=== vector_test: real_vector_test ===" << endl;
  cout << "a:         " << rvect << endl;
  cout << "a+a:       " << rvect + rvect << endl;
  USML_VECTOR_REAL_TESTER(rvect + rvect, rvect, add_helper<double>);
  cout << "a*2.1:     " << rvect * 2.1 << endl;
  USML_VECTOR_REAL_TESTER(rvect * 2.1, rvect, scalar2_multiply_helper<double>);
  cout << "a+2.1:     " << rvect + 2.1 << endl;
  USML_VECTOR_REAL_TESTER(rvect + 2.1, rvect, scalar2_add_helper<double>);
  cout << "2.1-a:     " << 2.1 - rvect << endl;
  USML_VECTOR_REAL_TESTER(2.1 - rvect, rvect, scalar1_minus_helper<double>);
  cout << "2.1/a:     " << 2.1 / rvect << endl;
  USML_VECTOR_REAL_TESTER(2.1 / rvect, rvect, scalar1_divide_helper<double>);
  cout << "a*a:       " << rvect * rvect << endl;
  USML_VECTOR_REAL_TESTER(rvect * rvect, rvect, multiply_helper<double>);
  cout << "a/a:       " << rvect / rvect << endl;  // NOLINT
  USML_VECTOR_REAL_TESTER(rvect / rvect, rvect, divide_helper<double>);
  cout << "max(a,3.0):   " << max(rvect, 3.0) << endl;
  USML_VECTOR_REAL_TESTER(max(rvect, 3.0), rvect, max_helper<double>);
  cout << "min(a,3.0):   " << min(rvect, 3.0) << endl;
  USML_VECTOR_REAL_TESTER(min(rvect, 3.0), rvect, min_helper<double>);
  cout << "floor(a+2.1): " << floor(rvect + 2.1) << endl;
  USML_VECTOR_REAL_TESTER(floor(rvect + 2.1), rvect, floor_helper<double>);
  cout << "ceil(a+2.1):  " << ceil(rvect + 2.1) << endl;
  USML_VECTOR_REAL_TESTER(ceil(rvect + 2.1), rvect, ceil_helper<double>);
  cout << "to_degrees(to_radians(a+2.1)):     "
       << to_degrees(to_radians(rvect + 2.1)) << endl;
  USML_VECTOR_REAL_TESTER(to_degrees(to_radians(rvect + 2.1)), rvect,
                          scalar2_add_helper<double>);
  cout << "to_latitude(to_colatitude(a+2.1)): "
       << to_latitude(to_colatitude(rvect + 2.1)) << endl;
  USML_VECTOR_REAL_TESTER(to_latitude(to_colatitude(rvect + 2.1)), rvect,
                          scalar2_add_helper<double>);

  cout << "abs(a):    " << abs(rvect) << endl;
  USML_VECTOR_REAL_TESTER(abs(rvect), rvect, std::abs<double>);
  cout << "abs2(a):   " << abs2(rvect) << endl;
  USML_VECTOR_REAL_TESTER(abs2(rvect), rvect, abs2_helper<double>);
  cout << "arg(a):    " << arg(rvect) << endl;
  USML_VECTOR_REAL_TESTER(arg(rvect), rvect, zero_helper<double>);
  cout << "sqrt(a):   " << sqrt(rvect) << endl;
  USML_VECTOR_REAL_TESTER(sqrt(rvect), rvect, std::sqrt<double>);

  cout << "cos(a):    " << cos(rvect) << endl;
  USML_VECTOR_REAL_TESTER(cos(rvect), rvect, std::cos<double>);
  cout << "cosh(a):   " << cosh(rvect) << endl;
  USML_VECTOR_REAL_TESTER(cosh(rvect), rvect, std::cosh<double>);
  cout << "sin(a):    " << sin(rvect) << endl;
  USML_VECTOR_REAL_TESTER(sin(rvect), rvect, std::sin<double>);
  cout << "sinh(a):   " << sinh(rvect) << endl;
  USML_VECTOR_REAL_TESTER(sinh(rvect), rvect, std::sinh<double>);
  cout << "tan(a):    " << tan(rvect) << endl;
  USML_VECTOR_REAL_TESTER(tan(rvect), rvect, std::tan<double>);
  cout << "tanh(a):   " << tanh(rvect) << endl;
  USML_VECTOR_REAL_TESTER(tanh(rvect), rvect, std::tanh<double>);

  cout << "exp(a):    " << exp(rvect) << endl;
  USML_VECTOR_REAL_TESTER(exp(rvect), rvect, std::exp<double>);
  cout << "log(a):    " << log(rvect) << endl;
  USML_VECTOR_REAL_TESTER(log(rvect), rvect, std::log<double>);
  cout << "log10(a):  " << log10(rvect) << endl;
  USML_VECTOR_REAL_TESTER(log10(rvect), rvect, std::log10<double>);
}

/**
 * Run a complex valued vector through the standard math routines.  Tests
 * include:
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
 * Warning: Some compilers, at low optimization levels, fail to properly
 * chain multiple ublas operations into a single evaluation.
 * When this happens, the result is equal to the first operation.
 * These tests have been designed to catch this flaw.
 */
BOOST_AUTO_TEST_CASE(complex_vector_test) {
  vector<std::complex<double> > cvect(7);
  for (size_t n = 0; n < cvect.size(); ++n) {
    auto f = static_cast<double>(n + 1);
    cvect(n) = std::complex<double>(f, 1.0);
  }

  cout << "=== vector_test: complex_vector_test ===" << endl;
  cout << "a:         " << cvect << endl;
  cout << "a+a:       " << cvect + cvect << endl;
  USML_VECTOR_COMPLEX_TESTER(cvect + cvect, cvect,
                             add_helper<complex<double> >);
  cout << "a+2.1:     " << cvect + 2.1 << endl;
  USML_VECTOR_COMPLEX_TESTER(cvect + 2.1, cvect,
                             scalar2_add_helper<complex<double> >);
  cout << "2.1-a:     " << 2.1 - cvect << endl;
  USML_VECTOR_COMPLEX_TESTER(2.1 - cvect, cvect,
                             scalar1_minus_helper<complex<double> >);

  cout << "abs(a):    " << abs(cvect) << endl;
  USML_VECTOR_COMPLEX_TESTER(abs(cvect), cvect, abs_helper<complex<double> >);
  cout << "abs2(a):   " << abs2(cvect) << endl;
  USML_VECTOR_COMPLEX_TESTER(abs2(cvect), cvect, abs2_helper<complex<double> >);
  cout << "arg(a):    " << arg(cvect) << endl;
  USML_VECTOR_COMPLEX_TESTER(arg(cvect), cvect, arg_helper<double>);
  cout << "sqrt(a):   " << sqrt(cvect) << endl;
  USML_VECTOR_COMPLEX_TESTER(sqrt(cvect), cvect, std::sqrt<double>);

  cout << "cos(a):    " << cos(cvect) << endl;
  USML_VECTOR_COMPLEX_TESTER(cos(cvect), cvect, std::cos<double>);
  cout << "cosh(a):   " << cosh(cvect) << endl;
  USML_VECTOR_COMPLEX_TESTER(cosh(cvect), cvect, std::cosh<double>);
  cout << "sin(a):    " << sin(cvect) << endl;
  USML_VECTOR_COMPLEX_TESTER(sin(cvect), cvect, std::sin<double>);
  cout << "sinh(a):   " << sinh(cvect) << endl;
  USML_VECTOR_COMPLEX_TESTER(sinh(cvect), cvect, std::sinh<double>);
  cout << "tan(a):    " << tan(cvect) << endl;
  USML_VECTOR_COMPLEX_TESTER(tan(cvect), cvect, std::tan<double>);
  cout << "tanh(a):   " << tanh(cvect) << endl;
  USML_VECTOR_COMPLEX_TESTER(tanh(cvect), cvect, std::tanh<double>);

  cout << "exp(a):    " << exp(cvect) << endl;
  USML_VECTOR_COMPLEX_TESTER(exp(cvect), cvect, std::exp<double>);
  cout << "log(a):    " << log(cvect) << endl;
  USML_VECTOR_COMPLEX_TESTER(log(cvect), cvect, std::log<double>);
  cout << "log10(a):  " << log10(cvect) << endl;
  USML_VECTOR_COMPLEX_TESTER(log10(cvect), cvect, std::log10<double>);
}

/**
 * Test all of the real and complex combinations of the pow() function.
 */
BOOST_AUTO_TEST_CASE(pow_vector_test) {
  vector<double> rvect(3);
  vector<std::complex<double> > cvect(3);
  std::complex<double> cmplx(2.5, 3.5);

  for (size_t n = 0; n < cvect.size(); ++n) {
    auto f = static_cast<double>(n + 1);
    rvect(n) = f;
    cvect(n) = std::complex<double>(f, 1.0);
  }

  cout << "=== vector_test: pow_vector_test ===" << endl;
  cout << "N:      " << rvect << endl;
  cout << "M:      " << cvect << endl;
  cout << "C:      " << cmplx << endl;

  cout << "N^3:    " << pow(rvect, 3) << endl;
  USML_VECTOR_POW_SCALAR2_TESTER(pow(rvect, 3), rvect, 3);
  cout << "N^2.5:  " << pow(rvect, 2.5) << endl;
  USML_VECTOR_POW_SCALAR2_TESTER(pow(rvect, 2.5), rvect, 2.5);
  cout << "2^N:    " << pow(2.0, rvect) << endl;
  USML_VECTOR_POW_SCALAR1_TESTER(pow(2.0, rvect), 2.0, rvect);

  //    cout << "M^3:    " << pow(cvect,3.0) << endl;
  //    USML_VECTOR_POW_SCALAR2_TESTER( pow(cvect,3), cvect, 3 ) ;
  cout << "M^2.5:  " << pow(cvect, 2.5) << endl;
  USML_VECTOR_POW_SCALAR2_TESTER(pow(cvect, 2.5), cvect, 2.5);
  cout << "2^M:    " << pow(2.0, cvect) << endl;
  USML_VECTOR_POW_SCALAR1_TESTER(pow(2.0, cvect), 2.0, cvect);

  cout << "N^N:    " << pow(rvect, rvect) << endl;
  USML_VECTOR_POW_TESTER(pow(rvect, rvect), rvect, rvect);
  cout << "M^M:    " << pow(cvect, cvect) << endl;
  USML_VECTOR_POW_TESTER(pow(cvect, cvect), cvect, cvect);
  cout << "N^M:    " << pow(rvect, cvect) << endl;
  USML_VECTOR_POW_TESTER(pow(rvect, cvect), rvect, cvect);
  cout << "M^N:    " << pow(cvect, rvect) << endl;
  USML_VECTOR_POW_TESTER(pow(cvect, rvect), cvect, rvect);
}

/**
 * Test the generation of real valued and analytic signals from
 * a vector of arguments.
 *
 * Assume that testers will visually inspect the results.
 */
BOOST_AUTO_TEST_CASE(signal_vector_test) {
  vector<double> rvect(3);
  for (size_t n = 0; n < rvect.size(); ++n) {
    rvect(n) = static_cast<double>(n + 1);
  }

  cout << "=== vector_test: signal_vector_test ===" << endl;
  cout << "input:  " << rvect << endl;
  cout << "signal: " << signal(rvect) << endl;
  USML_VECTOR_REAL_TESTER(signal(rvect), rvect, signal_helper<double>);
  cout << "signal: " << asignal(rvect) << endl;
  USML_VECTOR_CR_TESTER(asignal(rvect), rvect, asignal_helper<double>);
}

/**
 * Run trig routines forward and backward to check algorithms.
 */
BOOST_AUTO_TEST_CASE(realInverse_vector_test) {
  vector<double> rvect(3);
  for (size_t n = 0; n < rvect.size(); ++n) {
    rvect(n) = 0.1 * static_cast<double>(n + 1);
  }

  cout << "=== vector_test: realInverse_vector_test ===" << endl;
  cout << "acos(cos(t)):    " << acos(cos(rvect)) << endl;
  USML_VECTOR_REAL_TESTER(acos(cos(rvect)), rvect, identity<double>);
  cout << "acosh(cosh(t)):  " << acosh(cosh(rvect)) << endl;
  USML_VECTOR_REAL_TESTER(acosh(cosh(rvect)), rvect, identity<double>);
  cout << "asin(sin(t)):    " << asin(sin(rvect)) << endl;
  USML_VECTOR_REAL_TESTER(asin(sin(rvect)), rvect, identity<double>);
  cout << "asinh(sinh(t)):  " << asinh(sinh(rvect)) << endl;
  USML_VECTOR_REAL_TESTER(asinh(sinh(rvect)), rvect, identity<double>);
  cout << "atan(tan(t)):    " << atan(tan(rvect)) << endl;
  USML_VECTOR_REAL_TESTER(atan(tan(rvect)), rvect, identity<double>);
  cout << "atanh(tanh(t)):  " << atanh(tanh(rvect)) << endl;
  USML_VECTOR_REAL_TESTER(atanh(tanh(rvect)), rvect, identity<double>);
}

/**
 * Run complex trig routines forward and backward to check algorithms.
 */
BOOST_AUTO_TEST_CASE(complexInverse_vector_test) {
  vector<std::complex<double> > cvect(3);
  for (size_t n = 0; n < cvect.size(); ++n) {
    auto f = static_cast<double>(n + 1);
    cvect(n) = 0.1 * std::complex<double>(f, 1.0);
  }

  cout << "=== vector_test: complexInverse_vector_test ===" << endl;
  cout << "acos(cos(t)):    " << acos(cos(cvect)) << endl;
  USML_VECTOR_COMPLEX_TESTER(acos(cos(cvect)), cvect,
                             identity<complex<double> >);
  cout << "acosh(cosh(t)):  " << acosh(cosh(cvect)) << endl;
  USML_VECTOR_COMPLEX_TESTER(acosh(cosh(cvect)), cvect,
                             identity<complex<double> >);
  cout << "asin(sin(t)):    " << asin(sin(cvect)) << endl;
  USML_VECTOR_COMPLEX_TESTER(asin(sin(cvect)), cvect,
                             identity<complex<double> >);
  cout << "asinh(sinh(t)):  " << asinh(sinh(cvect)) << endl;
  USML_VECTOR_COMPLEX_TESTER(asinh(sinh(cvect)), cvect,
                             identity<complex<double> >);
  cout << "atan(tan(t)):    " << atan(tan(cvect)) << endl;
  USML_VECTOR_COMPLEX_TESTER(atan(tan(cvect)), cvect,
                             identity<complex<double> >);
  cout << "atanh(tanh(t)):  " << atanh(tanh(cvect)) << endl;
  USML_VECTOR_COMPLEX_TESTER(atanh(tanh(cvect)), cvect,
                             identity<complex<double> >);
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
