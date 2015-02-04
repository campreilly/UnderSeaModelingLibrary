/**
 * @file beam_pattern_omni.cc
 */

#include <usml/sensors/beam_pattern_omni.h>

using namespace usml::sensors ;

/** Calculates the beam level in de, az, and frequency **/
void beam_pattern_omni::beam_level(
        double de, double az, size_t beam,
        vector<double>* level )
{
    noalias(*level) = _level ;
}

/**
 * The user may call this function but it has no effect on the
 * beam level.
 */
void beam_pattern_omni::orient_beam( double roll,
        double pitch, double yaw )
{
    _roll = roll ;
    _pitch = pitch ;
    _yaw = yaw ;
}

/**
 * Initializes the beam pattern
 */
void beam_pattern_omni::initialize_beams( const seq_vector& frequencies )
{
    _directivity_index = scalar_vector<double>( frequencies.size(), 0.0 ) ;
    _level = scalar_vector<double>( frequencies.size(), 1.0 ) ;
    _roll = 0.0 ;
    _pitch = 0.0 ;
    _yaw = 0.0 ;
}
