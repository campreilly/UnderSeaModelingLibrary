/**
 * @file beam_pattern_VLA.cc
 */

#include <usml/sensors/beam_pattern_VLA.h>

using namespace usml::sensors ;

/**
 * Constructor
 */
beam_pattern_VLA::beam_pattern_VLA(
    double sound_speed,
    double spacing,
    size_t elements,
    double steering_angle )
    : beam_pattern_line( sound_speed, spacing,
                         elements, steering_angle,
                         beam_pattern_line::VERTICAL )
{

}

/**
 * Calculates the beam level in de, az, and frequency
 */
void beam_pattern_VLA::beam_level(
        double de, double az,
        orientation& orient,
        const vector<double>& frequencies,
        vector<double>* level )
{
    if( orient.heading() != _orient_VLA.heading() ||
        orient.pitch() != _orient_VLA.pitch() ||
        orient.roll() != _orient_VLA.roll() )
    {
        _orient_VLA.update_orientation(
                orient.heading(), -orient.pitch(), orient.roll() ) ;
    }
    beam_pattern_line::beam_level( de, az, _orient_VLA, frequencies, level ) ;
}

