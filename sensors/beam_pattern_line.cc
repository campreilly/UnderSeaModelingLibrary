/**
 * @file beam_pattern_line.cc
 */

#include <usml/sensors/beam_pattern_line.h>

using namespace usml::sensors ;

/** Calculates the beam level in de, az, and frequency **/
void beam_pattern_line::beam_level(
        double de, double az, size_t beam,
        vector<double>* level )
{
    double theta = de + M_PI_2 ;
    double phi = -az ;
    double sint = sin( 0.5 * (theta - _pitch) + 1e-10 ) ;
    double sinp = sin( 0.5 * (phi + _yaw) + 1e-10 ) ;
    double dotnorm = 1.0 - 2.0 * ( sint * sint
                     + sin(theta) * sin(_pitch) * sinp * sinp ) ;
    noalias(*level) = element_prod(
                element_div( sin(_omega_n*dotnorm - _steering_n[beam]),
                    _n*sin(_omega*dotnorm - _steering[beam]) ),
                element_div( sin(_omega_n*dotnorm - _steering_n[beam]),
                    _n*sin(_omega*dotnorm - _steering[beam]) )
            ) ;
}

/**
 * Initializes the beam pattern
 */
void beam_pattern_line::initialize_beams(
        double sound_speed, double spacing,
        const seq_vector& frequencies,
        vector<double>& steering_angles )
{
        // compute omega/2 and omega/2 * n
    _omega = frequencies ;
    _omega *= (M_PI * spacing / sound_speed) ;
    _omega_n = _omega * _n ;
        // compute sine of the steering angles and multiply by omega and n
    _steering = element_prod( _omega, sin(steering_angles) ) ;
    _steering_n = _n * _steering ;
        // compute the directivity index
    _directivity_index = scalar_vector<double>( frequencies.size(), 0.0 ) ;
    vector<double> steer_plus = element_prod( 2.0*_omega, (1 + sin(steering_angles)) ) ;
    vector<double> steer_minus = element_prod( 2.0*_omega, (1 - sin(steering_angles)) ) ;
    for(size_t p=1; p<_n; ++p) {
        _directivity_index += element_div( (_n-p)*(sin(p*steer_plus)+sin(p*steer_minus)), p*2.0*_omega ) ;
    }
    _directivity_index = 10.0*log10( _n ) - 10.0*log10( 1.0 + (1.0/_n)*_directivity_index ) ;
        // default to no pitch/roll/yaw
    _roll = 0.0 ;
    _pitch = 0.0 ;
    _yaw = 0.0 ;
}
