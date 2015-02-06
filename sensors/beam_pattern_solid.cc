/**
 * @file beam_pattern_solid.cc
 */

#include <usml/sensors/beam_pattern_solid.h>

using namespace usml::sensors ;

/** Calculates the beam level in de, az, and frequency **/
void beam_pattern_solid::beam_level(
        double de, double az, size_t beam,
        vector<double>* level )
{
    vector<double> result = scalar_vector<double>( _directivity_index.size(), 0.0 ) ;
    if( (de <= _up) && (de >= _down) ) {
        if( (az <= _right) && (az >= _left) ) {
            result = scalar_vector<double>( _directivity_index.size(), 1.0 ) ;
        }
    }
    noalias(*level) = result ;
}

/**
 * The user may call this function but it has no effect on the
 * beam level.
 */
void beam_pattern_solid::orient_beam( double roll,
        double pitch, double yaw )
{
    _roll = roll ;
    _up = _max_de - pitch ;
    _down = _min_de - pitch ;
    _left = _min_az + yaw ;
    _right = _max_az + yaw ;
}

/**
 * Initializes the beam pattern
 */
void beam_pattern_solid::initialize_beams( const seq_vector& frequencies )
{
    _max_de *= M_PI / 180.0 ;
    _min_de *= M_PI / 180.0 ;
    _max_az *= M_PI / 180.0 ;
    _min_az *= M_PI / 180.0 ;
    double solid = (_max_az - _min_az)*(sin(_max_de) - sin(_min_de)) ;
    _directivity_index =
            scalar_vector<double>( frequencies.size(), 10.0*log10( (4.0*M_PI) / solid ) ) ;
    _roll = 0.0 ;
    _up = _max_de ;
    _down = _min_de ;
    _left = _min_az ;
    _right = _max_az ;
}
