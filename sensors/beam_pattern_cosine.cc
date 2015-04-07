/**
 * @file beam_pattern_cosine.cc
 */

#include <usml/sensors/beam_pattern_cosine.h>

using namespace usml::sensors ;

/**
 * Constructor
 */
beam_pattern_cosine::beam_pattern_cosine()
{
    _reference_axis(0) = 0.0 ;
    _reference_axis(1) = 1.0 ;
    _reference_axis(2) = 0.0 ;
}

/** Calculates the beam level in de, az, and frequency **/
void beam_pattern_cosine::beam_level(
        double de, double az,
        double theta, double phi,
        const vector<double>& frequencies,
        vector<double>* level )
{
	write_lock_guard(_mutex);
    double theta_prime = M_PI_2 - de ;
    double sint = sin( 0.5 * (theta_prime - theta) + 1e-10 ) ;
    double sinp = sin( 0.5 * (az + phi) + 1e-10 ) ;
    double dotnorm = 1.0 - 2.0 * ( sint * sint
                     + sin(theta_prime) * sin(theta) * sinp * sinp ) ;
    noalias(*level) =
            scalar_vector<double>( frequencies.size(), dotnorm ) ;
}

/**
 * Initializes the beam pattern
 */
void beam_pattern_cosine::directivity_index(
        const vector<double>& frequencies,
        vector<double>* level )
{
	write_lock_guard(_mutex);
    noalias(*level) =
            scalar_vector<double>( frequencies.size(), 10.0*log10( 2.0 ) ) ;
}
