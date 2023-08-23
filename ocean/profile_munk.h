/**
 * @file profile_munk.h
 * Creates an analytic model for the Munk profile.
 */
#pragma once

#include <usml/ocean/profile_model.h>
#include <usml/types/wposition.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/matrix.hpp>

namespace usml {
namespace ocean {

using namespace usml::types;

/// @ingroup profiles
/// @{

/**
 * Creates an analytic model for the Munk profile, an idealized representation
 * of a deep sound (SOFAR) channel. The Munk profile is defined as:
 * <pre>
 *          z' = 2 * (z-z1)/B ;
 *
 *          c(z) = c1 * (1.0 + e * (z'-1.0+exp(-z'))) ;
 *
 * where:
 *          z    = depth below the ocean surface (positive direction down),
 *          z1   = depth of the deep sound channel axis,
 *          B    = depth scaling factor,
 *          c1   = sound speed on the deep sound channel axis,
 *          e    = perturbation coefficient,
 *          c(z) = speed of sound as a function of depth.
 * </pre>
 * @xref W. H. Munk, "Sound channel in an exponentially stratified ocean,
 * with application to SOFAR", J. Acoust. Soc. Amer. (55) (1974) pp. 220-226.
 */
class USML_DECLSPEC profile_munk : public profile_model {
   public:
    /**
     * Build Munk profile from its component parts.
     * Default to the profile parameters defined in Jensen, Kuperman, et. al.
     *
     * @param axis_depth    Depth of the deep sound channel axis (meters).
     * @param scale         Depth scaling factor (meters).
     * @param axis_speed    Sound speed on the deep sound channel axis (m/s).
     * @param epsilon       Perturbation coefficient.
     * @param attmodel      In-water attenuation model.  Defaults to Thorp.
     *
     * @xref F.B. Jensen, W.A. Kuperman, M.B. Porter, H. Schmidt,
     * "Computational Ocean Acoustics", Figure 3.19 (b).
     */
    profile_munk(double axis_depth = 1300.0, double scale = 1300.0,
                 double axis_speed = 1500.0, double epsilon = 7.37e-3,
                 attenuation_model::csptr attmodel = nullptr)
        : profile_model(attmodel),
          _axis_depth(axis_depth),
          _scale(scale),
          _axis_speed(axis_speed),
          _epsilon(epsilon) {}

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
    /** Depth of the deep sound channel axis (meters) */
    const double _axis_depth;

    /** Depth scaling factor (meters). */
    const double _scale;

    /** Sound speed on the deep sound channel axis (m/s). */
    const double _axis_speed;

    /** Perturbation coefficient. */
    const double _epsilon;
};

/// @}
}  // end of namespace ocean
}  // end of namespace usml
