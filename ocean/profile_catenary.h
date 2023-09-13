/**
 * @file profile_catenary.h
 * Creates an analytic model for a deep duct catenary profile.
 */
#pragma once

#include <stddef.h>
#include <usml/ocean/profile_model.h>
#include <usml/types/wposition.h>
#include <usml/types/wvector.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/matrix.hpp>

namespace usml {
namespace ocean {

using namespace usml::types;

/// @ingroup profiles
/// @{

/**
 * Creates an analytic model for a deep sound channel profile.
 * The catenary form of this profile useful because its ray paths
 * can be computed analytically and used to test propagation
 * loss modeling accuracy against standard benchmarks.
 * The general form of the deep duct catenary profile is defined as:
 * <pre>
 *          c(z) = c1 * cosh( (z-z1) / g1 )
 *
 * where:
 *          z    = depth below the ocean surface  (positive direction down).
 *          c(z) = speed of sound as a function of depth
 *          z1   = depth of the deep sound channel axis
 *          c1   = speed of sound at the deep sound channel axis
 *          g1   = sound speed gradient scaling factor
 * </pre>
 *
 * @xref S.M. Reilly, M.S. Goodrich, "Geodetic Acoustic Rays in the
 * Time Domain, Comprehensive Test Results", Alion Science and
 * Technology, Norfolk, VA, September, 2006.
 */
class USML_DECLSPEC profile_catenary : public profile_model {
   public:
    /**
     * Default behavior for new profile models.
     *
     * @param c1        Speed of sound at the deep sound channel axis.
     * @param g1        Sound speed gradient scaling factor.
     * @param z1        Depth of the deep sound channel axis.
     * @param attmodel  In-water attenuation model.  Defaults to Thorp.
     */
    profile_catenary(double c1, double g1, double z1,
                     const attenuation_model::csptr& attmodel = nullptr)
        : profile_model(attmodel),
          _soundspeed1(c1),
          _gradient1(g1),
          _depth1(z1) {}

    /**
     * Compute the speed of sound and it's first derivatives at
     * a series of locations.
     *
     * @param location      Location at which to compute sound speed.
     * @param speed         Speed of sound (m/s) at each location (output).
     * @param gradient      Sound speed gradient at each location (output).
     */
    void sound_speed(const wposition& location, matrix<double>* speed,
                     wvector* gradient) const override;

   private:
    /** Speed of sound at the deep sound channel axis. */
    double _soundspeed1;

    /** Sound speed gradient scaling factor. */
    double _gradient1;

    /** Depth of the deep sound channel axis. */
    double _depth1;
};

/// @}
}  // end of namespace ocean
}  // end of namespace usml
