/**
 * @example ublas/test/randgen_test.cc
 */

#include <usml/ublas/ublas.h>

#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/test/unit_test.hpp>
#include <iostream>

using namespace usml::ublas;

BOOST_AUTO_TEST_SUITE(randgen_test)

/**
 * @ingroup ublas_test
 * @{
 */

/**
 * Generate vectors of random numbers. Re-use the same seed each time.
 *
 * Assume that testers will visually inspect the results.
 */
BOOST_AUTO_TEST_CASE(rand_vector_test) {
  randgen gen(100);
  cout << "=== randgen_test: randVectorTest ===" << endl;
  cout << "uniform:  " << gen.uniform(7) << endl;
  cout << "gaussian: " << gen.gaussian(7) << endl;
  cout << "noise:    " << gen.cgaussian(3) << endl;
}

/**
 * Generate matrices of random numbers. Re-use the same seed each time.
 *
 * Assume that testers will visually inspect the results.
 */
BOOST_AUTO_TEST_CASE(rand_matrix_test) {
  randgen gen(100);
  cout << "=== randgen_test: randMatrixTest ===" << endl;
  cout << "uniform:  " << gen.uniform(2, 3) << endl;
  cout << "gaussian: " << gen.gaussian(2, 3) << endl;
  cout << "noise:    " << gen.cgaussian(2, 3) << endl;
}

/// @}
BOOST_AUTO_TEST_SUITE_END()
