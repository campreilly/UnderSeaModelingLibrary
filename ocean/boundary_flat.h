/**
 * @file boundary_flat.h
 * Models any flat boundary in the ocean including the ocean surface.
 */
#pragma once

#include <usml/ocean/boundary_model.h>
#include <usml/types/wposition.h>
#include <usml/types/wposition1.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/matrix.hpp>

namespace usml {
namespace ocean {

/// @ingroup boundaries
/// @{

/**
 * Models any flat boundary in the ocean including the ocean surface.
 */
class USML_DECLSPEC boundary_flat : public boundary_model {
   public:
    /**
     * Initialize depth and reflection loss components for a boundary.
     *
     * @param depth         Water depth relative to mean sea level.
     * @param reflect_loss  Reflection loss model.  Assumes depth=0 is used to
     *                      define the water surface and any other depths
     *                      define the ocean bottom. Use perfect surface or
     *                      bottom reflection if no model specified.
     * @param scattering    Reverberation scattering strength model.
     */
    boundary_flat(double depth = 0.0,
                  const reflect_loss_model::csptr& reflect_loss = nullptr,
                  const scattering_model::csptr& scattering = nullptr);

    /**
     * Compute the height of the boundary and it's surface normal at
     * a series of locations.
     *
     * @param location      Location at which to compute boundary.
     * @param rho           Surface height in spherical earth coords (output).
     * @param normal        Unit normal relative to location (output).
     */
    void height(const wposition& location, matrix<double>* rho,
                wvector* normal = nullptr) const override;

    /**
     * Compute the height of the boundary and it's surface normal at
     * a single location.  Often used during reflection processing.
     *
     * @param location      Location at which to compute boundary.
     * @param rho           Surface height in spherical earth coords (output).
     * @param normal        Unit normal relative to location (output).
     */
    void height(const wposition1& location, double* rho,
                wvector1* normal = nullptr) const override;

   private:
    /** Surface height in spherical earth coords. */
    const double _height;

    /** Rho component of the surface normal. */
    double _normal_rho;
};

/// @}
}  // end of namespace ocean
}  // end of namespace usml
