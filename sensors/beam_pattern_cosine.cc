/**
 * @file beam_pattern_cosine.cc
 */

#include <usml/sensors/beam_pattern_cosine.h>

using namespace usml::sensors ;

/**
 * Computes the response level in a specific DE/AZ pair and beam steering angle.
 * The return, level, is passed back in linear units.
 * 
 * @param de    Depression/Elevation angle (rad)
 * @param az    Azimuthal angle (rad)
 * @param pitch    pitch in the DE dimension (rad)
 * @param yaw    yaw in the AZ dimension (rad)
 * @param frequencies    list of frequencies to compute beam level for
 * @param level    beam level for each frequency
 */
void beam_pattern_cosine::beam_level(
        double de, double az,
        double pitch, double yaw,
        const vector<double>& frequencies,
        vector<double>* level )
{
    double _pitch = M_PI_2 + pitch ;
    double _yaw = M_PI_2 - yaw ;
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
 * Directivity index for an cosine-directional beam pattern
 * 
 * @param frequencies    list of frequencies to compute DI for
 * @param level    gain for each frequency
 */
void beam_pattern_cosine::directivity_index(
        const vector<double>& frequencies,
        vector<double>* level )
{
    noalias(*level) =
            scalar_vector<double>( frequencies.size(), 10.0*log10( 2.0 ) ) ;
}
