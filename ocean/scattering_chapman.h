/**
 * @file scattering_chapman.h
 * Models diffuse scattering from ocean surface using Chapman/Harris model.
 */
#pragma once

#include <usml/ocean/scattering_model.h>
#include <usml/ublas/ublas.h>

namespace usml {
namespace ocean {

using namespace usml::ublas;
using namespace usml::types;

using boost::numeric::ublas::vector;

/// @ingroup boundaries
/// @{

/**
 * Chapman/Harris empirical fit to surface scattering strength measurements.
 * Based on a single sea trial was carried out over a 52-h period in March 1961
 * in deep water north of Bermuda. Surface reverberation measurements made with
 * 1 lbs charges of TNT and an omni-directional hydrophone at nominal depths of
 * 300, 600, and 1200 ft. Mathematically, this leads to the relationship below.
 * \f[
 * 10 log_{10}S = 3.3 \beta log_{10}(\gamma/30) - 42.4 log_{10} \beta  + 2.6
 * \f]\f[
 * \beta = 158 \left( v f^{1/3} \right)^{-0.58}
 * \f]
 * where
 *         \f$ \gamma \f$ = grazing angle (degrees),
 *         \f$ v \f$ = wind speed (knots),
 *         \f$ f \f$ = frequency (Hz).
 *
 * This model is only used for surface reverberation.
 *
 * @xref Chapman R. P., Harris J. H., "Surface Backscattering Strengths Measured
 * with Explosive Sound Sources," J. Acoust. Soc. Am. 34, 1592–1597 (1962).
 */
class USML_DECLSPEC scattering_chapman : public scattering_model {
   public:
    /**
     * Initializes ocean surface scattering using Chapman/Harris model.
     *
     * @param wind_speed    Wind_speed used to develop rough seas (m/s)
     */
    scattering_chapman(double wind_speed) : _wind_speed(wind_speed) {}

    /**
     * Computes the broadband scattering strength for a single location.
     * Averages the incident and scattered grazing angles to estimate the impact
     * of bistatic geometries.
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
        auto grazing = 0.5 * (de_incident + de_scattered) * 180.0 / M_PI;
        vector<double> freq = frequencies->data();
        auto speed = _wind_speed * 1.94384449;
        vector<double> beta = 158.0 * pow(speed * pow(freq, 1.0 / 3.0), -0.58);
        *amplitude = 2.6 - 42.4 * log10(beta) +
                     3.3 * beta * log10(grazing / 30.0 + 1e-6);
        *amplitude = pow(10.0, *amplitude / 10.0);
    }

    /**
     * Computes the broadband scattering strength for a collection of
     * scattering angles from a common incoming ray. Each scattering
     * has its own location, de_scattered, and az_scattered. Averages the
     * incident and scattered grazing angles to estimate the impact of bistatic
     * geometries. The result is a broadband reverberation scattering strength
     * for each scattering.
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
        vector<double> freq = frequencies->data();
        auto speed = _wind_speed * 1.94384449;
        vector<double> beta = 158.0 * pow(speed * pow(freq, 1.0 / 3.0), -0.58);
        for (size_t n = 0; n < location.size1(); ++n) {
            for (size_t m = 0; m < location.size2(); ++m) {
                auto grazing =
                    0.5 * (de_incident + de_scattered(n, m)) * 180.0 / M_PI;
                (*amplitude)(n, m) = 2.6 - 42.4 * log10(beta) +
                                     3.3 * beta * log10(grazing / 30.0 + 1e-6);
                (*amplitude)(n, m) = pow(10.0, (*amplitude)(n, m) / 10.0);
            }
        }
    }

   private:
    /// Wind speed (m/s).
    const double _wind_speed;
};

/// @}
}  // end of namespace ocean
}  // end of namespace usml
