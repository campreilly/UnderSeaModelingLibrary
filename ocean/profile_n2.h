/** 
 * @file profile_n2.h
 * Creates an analytic model for the N^2 Linear profile.
 */

#ifndef USML_OCEAN_PROFILE_N2_H
#define USML_OCEAN_PROFILE_N2_H

#include <usml/ocean/profile_model.h>

namespace usml {
namespace ocean {

/// @ingroup profiles
/// @{

/**
 * Creates an analytic model for an N^2 Linear profile.
 * In this profile, the square of the index of refraction 
 * is linear. This is a useful profile because its ray paths
 * can be computed analytically and used to test propagation
 * loss modeling accuracy against standard benchmarks.
 * The general form of the N^2 Linear profile is defined as:
 * <pre>
 *          n(z) = c0 / c(z)
 *
 *          c(z) = c0 / sqrt( 1 - 2 g0 / c0 * z )
 *
 * where:
 *      z    = depth below the ocean surface (negative is down).
 *      n(z) = index of refraction as a function of depth.
 *      c(z) = speed of sound as a function of depth.
 *      c0   = speed of sound at z=0
 *      g0   = sound speed gradient at z=0 (+ when deeper depths are slower).
 * </pre>
 * @xref M. A. Pedersen, D. F. Gordon, "Normal-Mode and Ray Theory Applied
 * to Underwater Acoustic conditions of Extreme Downward Refraction",
 * J. Acoust. Soc. Am. 51 (1B), 323-368 (June 1972).
 */
class profile_n2 : public profile_model {

    //**************************************************
    // sound speed model

    /** Speed of sound at the surface of the water. */
    double _soundspeed0 ;

    /** Gradient factor = 2 g0 / c0 */
    double _factor ;

  public:

    /**
     * Compute the speed of sound and it's first derivatives at
     * a series of locations.
     *
     * @param location      Location at which to compute sound speed.
     * @param speed         Speed of sound (m/s) at each location (output).
     * @param gradient      Sound speed gradient at each location (output).
     */
    virtual void sound_speed( const wposition& location, 
        matrix<double>* speed, wvector* gradient=NULL ) ;

    //**************************************************
    // initialization

    /**
     * Default behavior for new profile models.
     * 
     * @param c0        Speed of sound at the surface of the water.
     * @param g0        Gradient for the square of the index of refraction.
     * @param attmodel  In-water attenuation model.  Defaults to Thorp.
     *                  The profile_model takes over ownship of this
     *                  reference and deletes it as part of its destructor.
     */
    profile_n2(
        double c0, double g0, 
        attenuation_model* attmodel=NULL 
        ) : 
        profile_model(attmodel),
        _soundspeed0(c0), _factor(2.0*g0/c0)
        { }

};

/// @}
}  // end of namespace ocean
}  // end of namespace usml

#endif
