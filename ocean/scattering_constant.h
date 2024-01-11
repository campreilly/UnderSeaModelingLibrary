/**
 * @file scattering_constant.h
 * Models reverberation scattering strength as a constant factor.
 */
#pragma once

#include <usml/ocean/scattering_model.h>

namespace usml {
namespace ocean {

using namespace usml::ublas;
using namespace usml::types;

using boost::numeric::ublas::vector;

/// @ingroup boundaries
/// @{

/**
 * Models reverberation scattering strength as a constant factor that
 * is independent of grazing angle and frequency.
 */
class USML_DECLSPEC scattering_constant : public scattering_model {
   public:
    /**
     * Initialize model with a constant factors.
     *
     * @param amplitude     Reverberation scattering strength (dB).
     *                         Provided in dB but converted to ratio internally.
     */
    scattering_constant(double amplitude = -300.0)
        : _amplitude(pow(10.0, amplitude / 10.0)) {}

    /**
     * Computes the broadband scattering strength for a single location.
     *
     * @param location      Location at which to compute attenuation.
     * @param frequencies   Frequencies over which to compute loss. (Hz)
     * @param de_incident   Depression incident angle (radians).
     * @param de_scattered  Depression scattered angle (radians).
     * @param az_incident   Azimuthal incident angle (radians).
     * @param az_scattered  Azimuthal scattered angle (radians).
     * @param amplitude     Reverberation scattering strength ratio (output).
     */
    void scattering(const wposition1& location,
                    const seq_vector::csptr& frequencies, double de_incident,
                    double de_scattered, double az_incident,
                    double az_scattered,
                    vector<double>* amplitude) const override {
        noalias(*amplitude) =
            scalar_vector<double>(frequencies->size(), _amplitude);
        // fast assignment of scalar to vector
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
        noalias(*amplitude) = scalar_matrix<vector<double> >(
            location.size1(), location.size2(),
            scalar_vector<double>(frequencies->size(), _amplitude));
        // fast assignment of scalar to matrix of vectors
    }

   private:
    /** Holds the reverberation scattering strength ratio. */
    double _amplitude;
};

/// @}
}  // end of namespace ocean
}  // end of namespace usml
