/**
 * @file beam_pattern_sine.cc
 */

#include <usml/sensors/beam_pattern_sine.h>

using namespace usml::sensors ;

/** Calculates the beam level in de, az, and frequency **/
void beam_pattern_sine::beam_level(
        double de, double az,
        const vector<double>& frequencies,
        vector<double>* level )
{
    double theta = de + M_PI_2 ;
    double phi = -az ;
    double sint = sin( 0.5 * (theta - _pitch) + 1e-10 ) ;
    double sinp = sin( 0.5 * (phi + _yaw) + 1e-10 ) ;
    double dotnorm = 1.0 - 2.0 * ( sint * sint
                     + sin(theta) * sin(_pitch) * sinp * sinp ) ;
    noalias(*level) =
            scalar_vector<double>( frequencies.size(), dotnorm ) ;
}

/**
 * Orients the beam spatially.
 */
void beam_pattern_sine::orient_beam( double roll,
        double pitch, double yaw )
{
    _roll = roll ;
    _pitch = pitch + M_PI_2 ;
    _yaw = yaw ;
}

/**
 * Computes the directivity index
 */
void beam_pattern_sine::directivity_index(
        const vector<double>& frequencies,
        vector<double>* level )
{
    noalias(*level) =
            scalar_vector<double>( frequencies.size(), 10.0*log10( 2.0 ) ) ;
}
