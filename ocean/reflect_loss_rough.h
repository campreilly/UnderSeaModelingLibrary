/**
 * @file reflect_loss_rough.h
 * Models plane wave reflection on the surface.
 *
 * Created on: Feb 17, 2014
 *     Author: Ted Burns, AEgis Technologies, Inc.
 */

#ifndef USML_OCEAN_REFLECT_LOSS_ROUGH_H
#define USML_OCEAN_REFLECT_LOSS_ROUGH_H

#include <usml/ocean/reflect_loss_model.h>

namespace usml {
namespace ocean {

using boost::numeric::ublas::vector;

/// @ingroup boundaries
/// @{

/**
 * Models plane wave reflection loss on the surface based on the criterion for the
 * roughness or smoothness of the surface given by the Rayleigh parameter.
 * Defined as  R = kH sin theta.
 * Where k is the wave number 2PI/lamda and H is the wave height (crest to trough)
 * and theta is the grazing angle.
 *
 * "Principles of Underwater Sound, Third Edition, 1983", pp. 129.
 */
class USML_DECLSPEC reflect_loss_rough : public reflect_loss_model {

public:

    /**
      * Constructor - Loads surface weather wind speed and sound speed
      * for later computation on calls to reflect_loss().
      *
      * @param wind_speed (m/s)
      * @param sound_speed Defaults to 1500 m/s when not specified.
      */

    reflect_loss_rough(double windspeed, double soundspeed=1500.0);

    /**
     * Computes the broadband reflection loss and phase change.
     *
     * @param location      Location at which to compute attenuation.
     * @param frequencies   Frequencies over which to compute loss. (Hz)
     * @param angle         Reflection angle relative to the normal (radians).
     * @param amplitude     Change in ray strength in dB (output).
     * @param phase         Change in ray phase in radians (output).
     *                      Phase change not computed if this is NULL.
     */
    virtual void reflect_loss(
        const wposition1& location,
        const seq_vector& frequencies, double angle,
        vector<double>* amplitude, vector<double>* phase=NULL ) ;

  private:

    /**
     * Computes the reflection loss based on the Rayleigh Parameter.
     * Defined as  R = kH sin theta. Where k is the wave number 2PI/lambda and H
     * is the wave height (crest to trough) and theta is the grazing angle.
     * u = exp(-R)
     * "Principles of Underwater Sound, Third Edition, 1983", pp. 129.
     *
     *  @param Wavelength (m)
     *  @param sin(surface grazing angle)
     *  @param Wind speed (m/s))
     *  @return  reflection_loss     Reflection loss amplitude in dB.
     */
    double getSurfaceReflectLoss(const double lambda, const double sinth, const double windSpeed);

    /**
     * Computes the rms crest-to-trough surface wave height (m)
     * with the wind speed provided.
     *
     * @param   windSpeed      Wind speed (m/s)
     * @return  rmsWaveHeight  rmsWaveHeight in meters
     *
     * "Fundamentals of Acoustical Oceanography," Herman
     * Medwin and Clarence S. Clay, Academic Press, New York
     * (1998) ISBN 0-12-487570-X; p. 583, Eq. 13.1.12.
     *
     * (Note: rms wave height h, sometimes denoted by sigma,
     * is related to significant wave height H by h = H/4.)
     */
    inline double getRMSWaveHeight(const double windSpeed){

        return 0.0053*windSpeed*windSpeed;
    }

    /** Speed of Wind (m/s) */
    const double _speed_wind;

    /** Speed of sound in water (m/s). */
    const double _speed_water;

} ;

/// @}
}  // end of namespace ocean
}  // end of namespace usml

#endif
