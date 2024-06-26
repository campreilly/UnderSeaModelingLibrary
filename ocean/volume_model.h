/**
 * @file volume_model.h
 * Generic interface for volume scattering layers.
 */
#pragma once

#include <usml/ocean/scattering_constant.h>

namespace usml {
namespace ocean {

using boost::numeric::ublas::vector;

/// @ingroup boundaries
/// @{

/**
 * A "volume scattering layer model" computes the environmental parameters of
 * a single volume scattering layer in the ocean.  The modeled properties
 * include the depth, thickness, and reverberation scattering strength
 * of the layer.  Depth is defined using the average (center) distance
 * of the layer relative to the center of the earth.  Thickness is
 * the full distance from the bottom to the top of the layer.
 */
class USML_DECLSPEC volume_model : public scattering_model {
   public:
    /// Shared pointer to constant version of this class.
    typedef std::shared_ptr<const volume_model> csptr;

    /**
     * Initialize reflection loss components for a boundary.
     *
     * @param scatter        Reverberation scattering strength model
     */
    volume_model(const scattering_model::csptr& scatter = nullptr)
        : _scattering(scatter) {
        if (scatter) {
            _scattering = scatter;
        } else {
            _scattering = scattering_model::csptr(new scattering_constant());
        }
    }

    /**
     * Virtual destructor
     */
    virtual ~volume_model() {}

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
    virtual void depth(const wposition& location, matrix<double>* rho,
                       matrix<double>* thickness = nullptr) const = 0;

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
    virtual void depth(const wposition1& location, double* rho,
                       double* thickness = nullptr) const = 0;

    //**************************************************
    // reverberation scattering strength model

    /**
     * Define a new reverberation scattering strength model.
     *
     * @param scattering    Scattering model for this layer.
     */
    void scattering(const scattering_model::csptr& scattering) {
        _scattering = scattering;
    }

    /**
     * Computes the broadband scattering strength for a single location.
     *
     * @param location      Location at which to compute attenuation.
     * @param frequencies   Frequencies over which to compute loss. (Hz)
     * @param de_incident   Depression incident angle (radians).
     * @param de_scattered  Depression scattered angle (radians).
     * @param az_incident   Azimuthal incident angle (radians).
     * @param az_scattered  Azimuthal scattered angle (radians).
     * @param amplitude     Change in ray strength in dB (output).
     */
    void scattering(const wposition1& location,
                    const seq_vector::csptr& frequencies, double de_incident,
                    double de_scattered, double az_incident,
                    double az_scattered,
                    vector<double>* amplitude) const override {
        _scattering->scattering(location, frequencies, de_incident,
                                de_scattered, az_incident, az_scattered,
                                amplitude);
    }

    /**
     * Computes the broadband scattering strength for a collection of
     * scattering angles from a common incoming ray. Each scattering
     * has its own location, de_scattered, and az_scattered.
     * The result is a broadband reverberation scattering strength for
     * each scattering.
     *
     * @param location      Location at which to compute attenuation.
     * @param frequencies   Frequencies over which to compute loss. (Hz)
     * @param de_incident   Depression incident angle (radians).
     * @param de_scattered  Depression scattered angle (radians).
     * @param az_incident   Azimuthal incident angle (radians).
     * @param az_scattered  Azimuthal scattered angle (radians).
     * @param amplitude     Reverberation scattering strength ratio (output).
     */
    void scattering(const wposition& location,
                    const seq_vector::csptr& frequencies, double de_incident,
                    matrix<double> de_scattered, double az_incident,
                    matrix<double> az_scattered,
                    matrix<vector<double> >* amplitude) const override {
        _scattering->scattering(location, frequencies, de_incident,
                                de_scattered, az_incident, az_scattered,
                                amplitude);
    }

   private:
    /** Reference to the scattering strength model **/
    scattering_model::csptr _scattering;
};

/// @}
}  // end of namespace ocean
}  // end of namespace usml
