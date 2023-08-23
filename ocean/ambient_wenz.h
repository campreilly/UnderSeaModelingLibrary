/**
 * @file ambient_wenz.h
 */
#pragma once

#include <usml/ocean/ambient_model.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition1.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/vector.hpp>

namespace usml {
namespace ocean {

/// @ingroup profiles
/// @{

/**
 * Model of ambient noise in the open ocean based on measured results that
 * have been fit to polynomials in dB space.
 *
 * @xref Gordon M. Wenz, Acoustic Ambient Noise in the Ocean: Spectra
 * and Sources, J. Acous. Soc. of Am. 34, 1936 (1962).
 */
class USML_DECLSPEC ambient_wenz : public ambient_model {
   public:
    /**
     * Initialize model with environmental parameters.
     *
     * @param wind_speed    	Winds speed at ocean surface (m/sec)
     * @param shipping_level	Shipping level, 0-7.
     * @param rain_rate			Rain rate 0-3.
     */
    ambient_wenz(double wind_speed, int shipping_level, int rain_rate)
        : _wind_speed(wind_speed),
          _shipping_level(shipping_level),
          _rain_rate(rain_rate) {}

    /**
     * Computes the power spectral density of ambient noise at a specific
     * location. This implementation returns the same power spectral density
     * for all locations.
     *
     * @param location  Location at which to compute noise.
     * @param frequency Frequencies at which noise is calculated.
     * @param noise     Ambient noise power spectral density (output)
     */
    void ambient(const wposition1 &location, seq_vector::csptr frequency,
                 vector<double> *noise) const override;

   private:
    /** Wind speed (m/sec). */
    double _wind_speed;

    /** Shipping level an an enumeration from 0-7. */
    int _shipping_level;

    /** Rain rate, 0-3 for none, interim, moderate, and heavy    */
    int _rain_rate;
};

/// @}
}  // end of namespace ocean
}  // end of namespace usml
