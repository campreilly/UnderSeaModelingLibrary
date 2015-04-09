/**
 * @file beam_pattern_line.cc
 */

#include <usml/sensors/beam_pattern_line.h>

using namespace usml::sensors ;

/**
 * Constructor
 */
beam_pattern_line::beam_pattern_line(
    double sound_speed, double spacing,
    size_t elements, double steering_angle,
    orientation_axis axis )
    : _axis(axis)
{
    switch( _axis ) {
        case HORIZONTAL :
            _n = elements ;
            _reference_axis(0) = 0.0 ;
            _reference_axis(1) = 1.0 ;
            _reference_axis(2) = 0.0 ;
            initialize_beams( sound_speed, spacing, (M_PI_2 + steering_angle) ) ;
            break ;
        default :
            _n = elements ;
            _reference_axis(0) = 0.0 ;
            _reference_axis(1) = 0.0 ;
            _reference_axis(2) = 1.0 ;
           initialize_beams( sound_speed, spacing, steering_angle ) ;
            break ;
    }
}

/** Calculates the beam level in de, az, and frequency **/
void beam_pattern_line::beam_level(
        double de, double az,
        orientation& orient,
        const vector<double>& frequencies,
        vector<double>* level )
{
	write_lock_guard(_mutex);
    double theta_prime = M_PI_2 + de ;
    double sint = sin( 0.5 * (orient.theta() - theta_prime) ) ;
    double sinp = sin( 0.5 * (az - orient.phi()) ) ;
    double dotnorm = 1.0 - 2.0 * ( sint * sint
                     + sin(theta_prime) * sin(orient.theta()) * sinp * sinp ) ;
    noalias(*level) = element_prod(
                element_div( sin(_n*(frequencies*_omega*dotnorm - frequencies*_steering + 1e-10)),
                    _n*sin(frequencies*_omega*dotnorm - frequencies*_steering + 1e-10) ),
                element_div( sin(_n*(frequencies*_omega*dotnorm - frequencies*_steering + 1e-10)),
                    _n*sin(frequencies*_omega*dotnorm - frequencies*_steering + 1e-10) )
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
        // compute sine of the steering angles and multiply by omega
    _steering = _omega * sin(steering_angle) ;
}

/**
 * Computes the directivity index for a list of
 * frequencies.
 */
void beam_pattern_line::directivity_index(
        const vector<double>& frequencies,
        vector<double>* level )
{
	write_lock_guard(_mutex);
    vector<double> di( frequencies.size(), 0.0 ) ;
    vector<double> steer_plus = 2.0*(_omega+_steering)*frequencies ;
    vector<double> steer_minus = 2.0*(_omega-_steering)*frequencies ;
    for(size_t p=1; p<_n; ++p) {
        di += element_div( (_n-p)*(sin(p*steer_plus)+sin(p*steer_minus)),
                                           (p*2.0*_omega*frequencies) ) ;
    }
    noalias(*level) = 10.0*log10( _n ) - 10.0*log10( 1.0 + (1.0/_n)*di ) ;
}
