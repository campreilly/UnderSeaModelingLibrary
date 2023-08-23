/**
 * @file scattering_lambert.h
 * Models diffuse scattering from ocean bottom using Lambert/Mackenzie model.
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
 * Lambert's Law defines the scattering strength from an ideal diffuse
 * reflector. In optics, the radiant intensity (power per unit solid angle)
 * is constant at all observed angle for ideal diffuse reflectors.
 * Mathematically, this leads to the relationship below.
 * \f[
 *   \frac{I_{scat}}{I_{inc}} = \mu \: sin(\gamma_{scat})
 *                             * sin(\gamma_{inc}) \: \delta A
 * \f]
 * where
 *         \f$ \gamma_{inc} \f$ = incident grazing angle,
 *         \f$ \gamma_{scat} \f$ = scattered grazing angle,
 *         \f$ \mu \f$ = scattering strength coefficient, and
 *         \f$ \delta A \f$ = ensonified area.
 *
 * In underwater acoustics, this is often referred to as the Mackenzie model,
 * based on at-sea measurements which indicated that indicated that
 * Lambert's Law was also a good fit for ocean bottom backscattering strength.
 * This model is only used for bottom reverberation.
 *
 * @xref Mackenzie K. V., "Bottom reverberation for 530 and 1030 cps
 * Sound in Deep Water," J. Acoust. Soc. Am. 33:1596 (1961).
 */
class USML_DECLSPEC scattering_lambert : public scattering_model {
   public:
    /**
     * Initializes scattering strength model with a Mackenzie coefficient.
     * Defaults to a value of the scattering strength coefficient
     * such that \f$10 log(\mu) = -27 dB\f$.
     *
     * @param    bss        Bottom scattering strength coefficient (dB)
     */
    scattering_lambert(double bss = -27.0) : _coeff(pow(10.0, bss / 10.0)) {}

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
    void scattering(const wposition1& location, seq_vector::csptr frequencies,
                    double de_incident, double de_scattered, double az_incident,
                    double az_scattered,
                    vector<double>* amplitude) const override {
        noalias(*amplitude) = scalar_vector<double>(
            frequencies->size(),
            abs(_coeff * sin(de_incident) * sin(de_scattered)));
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
    void scattering(const wposition& location, seq_vector::csptr frequencies,
                    double de_incident, matrix<double> de_scattered,
                    double az_incident, matrix<double> az_scattered,
                    matrix<vector<double> >* amplitude) const override {
        for (size_t n = 0; n < location.size1(); ++n) {
            for (size_t m = 0; m < location.size2(); ++m) {
                (*amplitude).operator()(n, m) = scalar_vector<double>(
                    frequencies->size(),
                    abs(_coeff * sin(de_incident) * sin(de_scattered(n, m))));
            }
        }
    }

   private:
    /**
     * Bottom scattering strength coefficient in linear units.
     */
    double _coeff;
};

/// @}
}  // end of namespace ocean
}  // end of namespace usml
