/**
 * @file ocean_model.h
 * Combines the effects of surface, bottom, volume and profile into a single
 * model.
 */
#pragma once

#include <usml/ocean/boundary_model.h>
#include <usml/ocean/profile_model.h>
#include <usml/ocean/volume_model.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition1.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/vector.hpp>
#include <cmath>
#include <cstddef>
#include <memory>
#include <vector>

namespace usml {
namespace ocean {

/// @ingroup ocean_model
/// @{

/**
 * Combines the effects of surface, bottom, volume, and profile into a single
 * model.
 */
class USML_DECLSPEC ocean_model {
   public:
    /// Shared pointer to constant version of this class.
    typedef std::shared_ptr<const ocean_model> csptr;

    /** Retrieve current model for the ocean surface. */
    boundary_model::csptr surface() const { return _surface; }

    /** Retrieve current model for the ocean bottom. */
    boundary_model::csptr bottom() const { return _bottom; }

    /** Retrieve one layer of the ocean volume. */
    volume_model::csptr volume(
        std::vector<volume_model::csptr>::size_type n) const {
        return _volume.at(n);
    }

    /** Retrieve number of ocean volume layers. */
    size_t num_volume() const { return _volume.size(); }

    /** Adds a layer to list of ocean volumes. */
    void add_volume(volume_model::csptr layer) { _volume.push_back(layer); }

    /** Retrieve current model for the ocean profile. */
    profile_model::csptr profile() const { return _profile; }

    /**
     * Associate ocean parts with this model.
     * The ocean model takes over ownership of these models and
     * destroys them in its destructor.
     */
    ocean_model(boundary_model::csptr surface, boundary_model::csptr bottom,
                profile_model::csptr profile,
                std::vector<volume_model::csptr>* volume = nullptr)
        : _surface(surface), _bottom(bottom), _profile(profile) {
        if (volume) {
            _volume = *volume;
        } else {
            _volume = std::vector<volume_model::csptr>();
        }
    }

    /**
     * Computes the broadband scattering strength for a specific interface.
     * Checks that the scattering strength is greater than the
     * intensity_threshold.  This check allows the model to skip interfaces
     * with trivial scattering strengths.
     *
     * @param interface 	Interface number of scattering ocean component
     * @param location      Location at which to compute attenuation.
     * @param frequencies   Frequencies over which to compute loss. (Hz)
     * @param de_incident   Depression incident angle (radians).
     * @param de_scattered  Depression scattered angle (radians).
     * @param az_incident   Azimuthal incident angle (radians).
     * @param az_scattered  Azimuthal scattered angle (radians).
     * @param amplitude     Change in ray strength in dB (output).
     */
    void scattering(size_t interface, const wposition1& location,
                    const seq_vector::csptr& frequencies, double de_incident,
                    double de_scattered, double az_incident,
                    double az_scattered, vector<double>* amplitude) const {
        switch (interface) {
            case 0:  // bottom
                _bottom->scattering(location, frequencies, de_incident,
                                    de_scattered, az_incident, az_scattered,
                                    amplitude);
                break;
            case 1:  // surface
                _surface->scattering(location, frequencies, de_incident,
                                     de_scattered, az_incident, az_scattered,
                                     amplitude);
                break;
            default:  // volume
                auto layer = (size_t)floor(((double)interface - 2.0) / 2.0);
                _volume.at(layer)->scattering(
                    location, frequencies, de_incident, de_scattered,
                    az_incident, az_scattered, amplitude);
                break;
        }
    }

   private:
    /** Model of the ocean surface. */
    boundary_model::csptr _surface;

    /** Model of the ocean bottom. */
    boundary_model::csptr _bottom;

    /** Models of ocean volume scattering strength layers. */
    std::vector<volume_model::csptr> _volume;

    /** Model of the sound speed profile and attenuation. */
    profile_model::csptr _profile;
};

/// @}
}  // end of namespace ocean
}  // end of namespace usml
