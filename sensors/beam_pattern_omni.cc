/**
 * @file beam_pattern_omni.cc
 */

#include <usml/sensors/beam_pattern_omni.h>

using namespace usml::sensors ;

/**
 * Computes the response level in a specific DE/AZ pair and beam steering angle.
 * The return, level, is passed back in linear units.
 * @param freuencies    list of frequencies to compute beam level for
 * 
 * @param de    Depression/Elevation angle (rad)
 * @param az    Azimuthal angle (rad)
 * @param pitch    pitch in the DE dimension (rad)
 * @param yaw    yaw in the AZ dimension (rad)
 * @param frequencies
 * @param level    beam level for each frequency
 */
void beam_pattern_omni::beam_level(
        double de, double az,
        double pitch, double yaw,
        const vector<double>& frequencies,
        vector<double>* level )
{
    noalias(*level) = scalar_vector<double>( frequencies.size(), 1.0 ) ;
}

/**
 * Directivity index for an omni-directional beam pattern The gain for this type
 * of beam pattern is 0 dB.
 * 
 * @param frequencies    list of frequencies to compute DI for
 * @param level    gain for each frequency
 */
void beam_pattern_omni::directivity_index(
        const vector<double>& frequencies,
        vector<double>* level )
{
    noalias(*level) = scalar_vector<double>( frequencies.size(), 0.0 ) ;
}
