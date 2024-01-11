/**
 * @file profile_munk.cc
 * Creates an analytic model for the Munk profile.
 */

#include <usml/ocean/profile_munk.h>
#include <usml/ublas/matrix_math.h>

#include <boost/numeric/ublas/expression_types.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_expression.hpp>

using namespace usml::ocean;

/**
 * Compute the speed of sound and it's first derivatives at
 * a series of locations.
 */
void profile_munk::sound_speed(const wposition& location, matrix<double>* speed,
                               wvector* gradient) const {
    if (gradient != nullptr) {
        gradient->clear();
    }

    matrix<double> z = 2 * (-location.altitude() - _axis_depth) / _scale;
    *speed = (((z - 1.0) + exp(-z)) * _epsilon + 1.0) * _axis_speed;
    if (gradient != nullptr) {
        gradient->rho((1.0 - exp(-z)) * (-_epsilon * _axis_speed * 2 / _scale));
    }

    adjust_speed(location, speed, gradient);
}
