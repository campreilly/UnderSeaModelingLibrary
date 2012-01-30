/** 
 * @example ublas/test/randgen_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <usml/ublas/randgen.h>
#include <usml/ublas/vector_math.h>

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
BOOST_AUTO_TEST_CASE( rand_vector_test ) {

    randgen::seed(100);

    cout << "=== randgen_test: randVectorTest ===" << endl;
    cout << "uniform:  " << randgen::uniform(7) << endl;
    cout << "gaussian: " << randgen::gaussian(7) << endl;
    cout << "noise:    " << randgen::noise(3) << endl;
}

/**
 * Generate matrices of random numbers. Re-use the same seed each time.
 *
 * Assume that testers will visually inspect the results.
 */
BOOST_AUTO_TEST_CASE( rand_matrix_test ) {

    randgen::seed(100);

    cout << "=== randgen_test: randMatrixTest ===" << endl;
    cout << "uniform:  " << randgen::uniform(2,3) << endl;
    cout << "gaussian: " << randgen::gaussian(2,3) << endl;
    cout << "noise:    " << randgen::noise(2,3) << endl;
}

/// @}

BOOST_AUTO_TEST_SUITE_END()

