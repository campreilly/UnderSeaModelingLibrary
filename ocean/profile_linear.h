/**
 * @file profile_linear.h
 * Creates an analytic model for the Bi-Linear, Linear or Constant profile.
 */
#pragma once

#include <usml/ocean/profile_model.h>
#include <usml/types/wposition.h>
#include <usml/types/wvector.h>

namespace usml {
namespace ocean {

using namespace usml::types;

/// @ingroup profiles
/// @{

/**
 * Creates an analytic model for a bi-linear, linear, or constant profile.
 * This type of profile is commonly used as an analytic test case in
 * propagation loss modeling.  The general form of the bi-linear
 * profile is defined as:
 * <pre>
 *          c(z) = c0 + g0 * z                      for z < z1
 *
 *          c(z) = c0 + g0 * z1 + g1 * ( z - z1 )   for z >= z1
 *
 * where:
 *          c0   = speed of sound at the surface of the water
 *          z    = depth below the ocean surface (positive direction down)
 *          z1   = depth at which profile changes gradients
 *          g0   = sound speed gradient for z < z1
 *          g1   = sound speed gradient for z >= z1
 *          c(z) = speed of sound as a function of depth
 * </pre>
 * This can be used as a linear profile by setting z1=0 and g1=g0.
 * In a contant profile, all of the values are zero except c0.
 */
class USML_DECLSPEC profile_linear : public profile_model {
   public:
    /**
     * Bi-Linear verion of the profile.
     *
     * @param c0        Speed of sound at the surface of the water.
     * @param g0        Sound speed gradient at the surface of the water.
     * @param z1        Depth at which profile changes gradients.
     * @param g1        Sound speed gradient for depths below z1.
     * @param attmodel  In-water attenuation model.  Defaults to Thorp.
     *                  The profile_model takes over ownership of this
     *                  reference and deletes it as part of its destructor.
     */
    profile_linear(double c0, double g0, double z1, double g1,
                   const attenuation_model::csptr& attmodel = nullptr)
        : profile_model(attmodel),
          _soundspeed0(c0),
          _gradient0(g0),
          _depth1(z1),
          _gradient1(g1) {}

    /**
     * Linear verion of the profile.
     *
     * @param c0        Speed of sound at the surface of the water.
     * @param g0        Sound speed gradient at the surface of the water.
     * @param attmodel  In-water attenuation model.  Defaults to Thorp.
     *                  The profile_model takes over ownership of this
     *                  reference and deletes it as part of its destructor.
     */
    profile_linear(double c0, double g0,
                   const attenuation_model::csptr& attmodel = nullptr)
        : profile_model(attmodel),
          _soundspeed0(c0),
          _gradient0(g0),
          _depth1(0.0),
          _gradient1(g0) {}

    /**
     * Constant speed verion of the profile.
     *
     * @param c0        Speed of sound at all points in the water.
     * @param attmodel  In-water attenuation model.  Defaults to Thorp.
     *                  The profile_model takes over ownership of this
     *                  reference and deletes it as part of its destructor.
     */
    profile_linear(double c0 = 1500.0,
                   const attenuation_model::csptr& attmodel = nullptr)
        : profile_model(attmodel),
          _soundspeed0(c0),
          _gradient0(0.0),
          _depth1(0.0),
          _gradient1(0.0) {}

    /**
     * Compute the speed of sound and it's first derivatives at
     * a series of locations.
     *
     * @param location      Location at which to compute sound speed.
     * @param speed         Speed of sound (m/s) at each location (output).
     * @param gradient      Sound speed gradient at each location (output).
     */
    void sound_speed(const wposition& location, matrix<double>* speed,
                     wvector* gradient = nullptr) const override;

   private:
    /** Speed of sound at the surface of the water. */
    double _soundspeed0;

    /** Sound speed gradient at the surface of the water. */
    double _gradient0;

    /** Depth at which profile changes gradients. */
    double _depth1;

    /** Sound speed gradient for depths below _depth1. */
    double _gradient1;
};

}  // end of namespace ocean
}  // end of namespace usml
