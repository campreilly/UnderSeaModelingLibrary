/**
 * @file reflect_loss_rough.cc
 * Models plane wave reflection on the surface.
 *
 * Created on: Feb 17, 2014
 *     Author: Ted Burns, AEgis Technologies, Inc.
 */

#include <usml/ocean/reflect_loss_rough.h>

using namespace usml::ocean ;

/**
 * Loads surface weather (wind speed) data_grid for
 * later computation on calls to reflect_loss().
 */
reflect_loss_rough::reflect_loss_rough(double windspeed, double soundspeed)
    :
    _speed_wind(windspeed),
    _speed_water(soundspeed)
{}

/**
 * Computes the broadband reflection loss and phase change.
 */
void reflect_loss_rough::reflect_loss(
    const wposition1& location,
    const seq_vector& frequencies, double angle,
    vector<double>* amplitude, vector<double>* phase )
{
    // For each frequency determine amplitude and phase
    unsigned int index = 0;
    double freq = 0.0;
    usml::types::seq_vector::const_iterator current;
    amplitude->resize(frequencies.size());
    phase->clear();
    double SR; // In dB

    for ( current = frequencies.begin(); current != frequencies.end(); current++, index++) {

        freq = (*current);

        SR = getSurfaceReflectLoss(_speed_water/freq, sin(angle), _speed_wind);

        amplitude->insert_element(index, SR);

    }  // frequencies iterator

}

/**
 * Defined as  R = kH sin theta.
 * Where k is the wave number 2PI/lamda and H is the wave height (crest to trough)
 * and theta is the grazing angle.
 *
 * "Principles of Underwater Sound, Third Edition, 1983", pp. 129.
 */

double reflect_loss_rough::getSurfaceReflectLoss(const double lambda,       // Wavelength (m)
                                                 const double sinth,        // sin(surface grazing angle)
                                                 const double windSpeed)    // Wind speed (m/s))
{
    // rms Wave Height
    const double H = getRMSWaveHeight(windSpeed);

    double R = (2.0*M_PI/lambda)*H*sinth;

    double u = exp(-R);

    return u;

}
