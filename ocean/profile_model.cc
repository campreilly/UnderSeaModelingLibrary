/**
 * @file profile_model.cc
 * Sound velocity profile and attenuation
 */

#include <usml/ocean/profile_model.h>

using namespace usml::ocean;

/**
 * When the flat earth option is enabled, this routine
 * applies an anti-correction term to the profile.
 */
void profile_model::adjust_speed(const wposition& location,
                                 matrix<double>* speed,
                                 wvector* gradient) const {
    if (_flat_earth) {
        if (gradient != nullptr) {
            gradient->rho(
                (element_prod(gradient->rho(), location.rho()) + *speed) /
                    wposition::earth_radius,
                false);
        }
        *speed = element_prod(*speed, location.rho()) / wposition::earth_radius;
    }
}
