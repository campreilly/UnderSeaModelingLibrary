/**
 * @file beam_pattern_cosine.cc
 */

#include <usml/sensors/beam_pattern_cosine.h>

using namespace usml::sensors ;

/** Calculates the beam level in de, az, and frequency **/
void beam_pattern_cosine::beam_level(
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
void beam_pattern_cosine::orient_beam( double roll,
        double pitch, double yaw )
{
    _roll = roll ;
    _pitch = M_PI_2 + pitch ;
    _yaw = M_PI_2 - yaw ;
}

/**
 * Initializes the beam pattern
 */
void beam_pattern_cosine::directivity_index(
        const vector<double>& frequencies,
        vector<double>* level )
{
    noalias(*level) =
            scalar_vector<double>( frequencies.size(), 10.0*log10( 2.0 ) ) ;
}
