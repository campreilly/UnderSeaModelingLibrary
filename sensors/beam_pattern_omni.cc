/**
 * @file beam_pattern_omni.cc
 */

#include <usml/sensors/beam_pattern_omni.h>

using namespace usml::sensors ;

/**
 * Default constructor sets the beamID to OMNI
 */
beam_pattern_omni::beam_pattern_omni()
{
    _beamID = beam_pattern_model::OMNI ;
}

/**
 * Destructor
 */
beam_pattern_omni::~beam_pattern_omni()
{

}


/** Calculates the beam level in de, az, and frequency **/
void beam_pattern_omni::beam_level(
        double de, double az,
        orientation& orient,
        const vector<double>& frequencies,
        vector<double>* level )
{
	write_lock_guard(_mutex);
    noalias(*level) = scalar_vector<double>( frequencies.size(), 1.0 ) ;
}

/**
 * The user may call this function but it has no effect on the
 * beam level.
 */
void beam_pattern_omni::directivity_index(
        const vector<double>& frequencies,
        vector<double>* level )
{
	write_lock_guard(_mutex);
    noalias(*level) = scalar_vector<double>( frequencies.size(), 0.0 ) ;
}
