/**
 * @file beam_pattern_HLA.cc
 */

#include <usml/sensors/beam_pattern_HLA.h>

using namespace usml::sensors ;

/**
 * Constructor
 */
beam_pattern_HLA::beam_pattern_HLA(
    double sound_speed,
    double spacing,
    size_t elements,
    double steering_angle )
    : beam_pattern_line( sound_speed, spacing,
                         elements, steering_angle,
                         beam_pattern_line::HORIZONTAL )
{

}

/**
 * Calculates the beam level in de, az, and frequency
 */
void beam_pattern_HLA::beam_level(
        double de, double az,
        orientation& orient,
        const vector<double>& frequencies,
        vector<double>* level )
{
    if( orient.heading() != _orient_HLA.heading() ||
        orient.pitch() != _orient_HLA.pitch() ||
        orient.roll() != _orient_HLA.roll() )
    {
        _orient_HLA.update_orientation(
                orient.heading(), -orient.pitch(), orient.roll() ) ;
    }
    beam_pattern_line::beam_level( de, az, _orient_HLA, frequencies, level ) ;
}

