/**
 * @file profile_n2.cc
 * Creates an analytic model for the N^2 Linear profile.
 */

#include <usml/ocean/profile_n2.h>

using namespace usml::ocean;

/**
 * Compute the speed of sound and it's first derivatives at
 * a series of locations.
 */
void profile_n2::sound_speed(const wposition& location, matrix<double>* speed,
                             wvector* gradient) const {
    *speed = _soundspeed0 / sqrt(1.0 - location.altitude() * _factor);
    if (gradient != nullptr) {
        gradient->clear();
        gradient->rho(pow(*speed, 3.0) * _factor /
                      (2.0 * _soundspeed0 * _soundspeed0));
    }

    adjust_speed(location, speed, gradient);
}
