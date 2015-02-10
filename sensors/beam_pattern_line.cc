/**
 * @file beam_pattern_line.cc
 */

#include <usml/sensors/beam_pattern_line.h>

using namespace usml::sensors ;

/** Calculates the beam level in de, az, and frequency **/
void beam_pattern_line::beam_level(
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
    noalias(*level) = element_prod(
                element_div( sin(frequencies*_omega_n*dotnorm - frequencies*_steering_n),
                    _n*sin(frequencies*_omega*dotnorm - frequencies*_steering) ),
                element_div( sin(frequencies*_omega_n*dotnorm - frequencies*_steering_n),
                    _n*sin(frequencies*_omega*dotnorm - frequencies*_steering) )
            ) ;
}

/**
 * Initializes the beam pattern
 */
void beam_pattern_line::initialize_beams(
        double sound_speed, double spacing,
        double steering_angle )
{
        // compute omega/2 and omega/2 * n
    _omega = (M_PI * spacing / sound_speed) ;
    _omega_n = _omega * _n ;
        // compute sine of the steering angles and multiply by omega and n
    _steering = _omega * sin(steering_angle) ;
    _steering_n = _n * _steering ;
        // default to no pitch/roll/yaw
    _roll = 0.0 ;
    _pitch = 0.0 ;
    _yaw = 0.0 ;
}

/**
 * Orients the beam spatially.
 */
void beam_pattern_line::orient_beam( double roll,
        double pitch, double yaw )
{
    switch( _axis ) {
        case HORIZONTAL :
            _roll = roll ;
            _pitch = -(pitch + M_PI_2) ;
            _yaw = -yaw ;
            break ;
        default :
            _roll = roll ;
            _pitch = -pitch ;
            _yaw = -yaw ;
            break ;
    }
}
/**
 * Computes the directivity index for a list of
 * frequencies.
 */
void beam_pattern_line::directivity_index(
        const vector<double>& frequencies,
        vector<double>* level )
{
    vector<double> di( frequencies.size(), 0.0 ) ;
    vector<double> steer_plus = 2.0*(_omega+_steering)*frequencies ;
    vector<double> steer_minus = 2.0*(_omega-_steering)*frequencies ;
    for(size_t p=1; p<_n; ++p) {
        di += element_div( (_n-p)*(sin(p*steer_plus)+sin(p*steer_minus)),
                                           (p*2.0*_omega*frequencies) ) ;
    }
    noalias(*level) = 10.0*log10( _n ) - 10.0*log10( 1.0 + (1.0/_n)*di ) ;
}
