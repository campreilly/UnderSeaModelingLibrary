/**
 * @example rvbenv/test/rvbenv_test.cc
 */

#include <usml/eigenverbs/eigenverb_collection.h>
#include <usml/eigenverbs/eigenverb_model.h>
#include <usml/rvbenv/rvbenv_collection.h>
#include <usml/types/seq_linear.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition1.h>
#include <usml/ublas/math_traits.h>

#include <algorithm>
#include <boost/geometry/geometry.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_expression.hpp>
#include <boost/test/unit_test.hpp>
#include <cmath>
#include <complex>
#include <cstddef>
#include <iostream>
#include <sstream>
#include <string>

BOOST_AUTO_TEST_SUITE(rvbenv_test)

using namespace usml::eigenverbs;
using namespace usml::rvbenv;
using namespace usml::types;
using namespace usml::ublas;

static const double c0 = 1500.0;  // constant sound speed

/**
 * @ingroup rvbenv_test
 * @{
 */

// TODO build tests for rvbenv_test

/// @}
BOOST_AUTO_TEST_SUITE_END()
