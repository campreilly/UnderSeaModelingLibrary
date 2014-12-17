/**
 * @file ublas.h uBLAS Extensions
 * @defgroup ublas uBLAS Extensions
 *
 * This package provides the extensions needed to transform Boost's
 * uBLAS library into a Matlab-like library of mathematical routines
 * for scientific applications.
 *
 * This library declares the following namespace using statements
 * within the local context of the usml::ublas namespace.
 *
 *      - using namespace boost::numeric::ublas ;
 *      - using std::cout ;
 *      - using std::endl ;
 *      - using std::operator<< ;
 *      - using std::complex;
 *      - using all of the std::math routines like std::sqrt()
 *
 * These declarations will be inherited by any applications that include
 * a "using namespace usml::ublas" statement.
 *
 * @defgroup vecmath Vector and Matrix Math
 * @ingroup ublas
 *
 * Adds support for complex numbers and elementary functions (trigonometric,
 * exponential, logrithmic, hyperbolic, etc.) to uBLAS's existing set of
 * mathematical algorithms. These functions are defined a series of math
 * trait templates with specializations for double, float, complex and
 * complex types. The uBLAS expression templates are then used to extend
 * these traits to vector and matrix types. The following files
 * document the implementation of these components.
 *
 * @defgroup randgen Random Numbers
 * @ingroup ublas
 *
 * This set of components integrates the Boost Random Number Library
 * with uBLAS vectors and matricies. Implemented as a singleton to ensure
 * that all random numbers are created from the same pseudo-random number
 * generator. This is designed to prevent un-intentional correlation between
 * random series in different parts of the application. The following files
 * are used to implement these components.
 *
 * @defgroup ublas_test Regression Tests
 * @ingroup ublas
 *
 * Regression tests for the ublas package.
 */
#pragma once

#include <usml/ublas/vector_math.h>
#include <usml/ublas/matrix_math.h>
#include <usml/ublas/randgen.h>
