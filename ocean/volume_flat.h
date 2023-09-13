/**
 * @file volume_flat.h
 * Models a simple volume reverberation layer in the ocean.
 */
#pragma once

#include <usml/ocean/volume_model.h>

#include <boost/numeric/ublas/vector_expression.hpp>

namespace usml {
namespace ocean {

using boost::numeric::ublas::vector;

/// @ingroup boundaries
/// @{

/**
 * Models a simple volume reverberation layer with constant depth
 * and thickness.
 */
class USML_DECLSPEC volume_flat : public volume_model {
   public:
    /**
     * Initialize depth and reflection loss components for a boundary.
     *
     * @param depth         Depth of layer relative to mean sea level.
     * @param thickness     Height of the layer from the bottom to the top.
     * @param amplitude     Reverberation scattering strength ratio.
     */
    volume_flat(double depth = 0.0, double thickness = 0.0,
                double amplitude = -300.0)
        : volume_model(
              scattering_model::csptr(new scattering_constant(amplitude))),
          _rho(wposition::earth_radius - abs(depth)),
          _thickness(thickness) {}

    /**
     * Initialize depth and reflection loss components for a boundary.
     *
     * @param depth         Depth of layer relative to mean sea level.
     * @param thickness     Height of the layer from the bottom to the top.
     * @param scattering    Reverberation scattering strength model.
     */
    volume_flat(double depth, double thickness,
                const scattering_model::csptr& scattering)
        : volume_model(scattering),
          _rho(wposition::earth_radius - abs(depth)),
          _thickness(thickness) {}

    /**
     * Compute the depth of the layer and it's thickness at
     * a series of locations.  Often used to detect collisions
     * between ray paths and the volume scattering layer.
     *
     * @param location      Location at which to compute boundary.
     * @param rho           Depth of layer center in spherical earth coords
     * (output).
     * @param thickness     Layer thickness (output).
     */
    void depth(const wposition& location, matrix<double>* rho,
               matrix<double>* thickness = nullptr) const override {
        noalias(*rho) = scalar_matrix<double>(rho->size1(), rho->size2(), _rho);
        if (thickness) {
            noalias(*thickness) = scalar_matrix<double>(
                thickness->size1(), thickness->size2(), _thickness);
        }
    }

    /**
     * Compute the depth of the layer and it's thickness at
     * a single location.  Often used to process the reverberation
     * for individual collisions with volume scattering layer.
     *
     * @param location      Location at which to compute boundary.
     * @param rho           Depth of layer center in spherical earth coords
     * (output).
     * @param thickness     Layer thickness (output).
     */
    void depth(const wposition1& location, double* rho,
               double* thickness = nullptr) const override {
        *rho = _rho;
        if (thickness) {
            *thickness = _thickness;
        }
    }

   private:
    /** Depth of layer relative to center of earth. (m) */
    const double _rho;

    /** Height of the layer from the bottom to the top. (m) */
    const double _thickness;
};

/// @}
}  // end of namespace ocean
}  // end of namespace usml
