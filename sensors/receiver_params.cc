/**
 * @file receiver_params.cc
 * Sensor characteristics for the receiver behaviors of a sensor.
 */
#include <usml/sensors/receiver_params.h>

using namespace usml::sensors;

/**
 * Construct new class of receiver.
 */
receiver_params::receiver_params(sensor_params::id_type paramsID, 
    const seq_vector& frequencies, 
    const std::list<beam_pattern_model::id_type>& beamList, bool multistatic)
    : sensor_params(paramsID, frequencies, multistatic), _beam_list(beamList)
{

}

/**
 * Searchs the beam pattern list for a specific beam pattern
 * with the requested ID
 */
beam_pattern_model::reference receiver_params::beam_pattern(
        beam_pattern_model::id_type beamID ) const
{
    if( std::find(_beam_list.begin(), _beam_list.end(), beamID)
        == _beam_list.end() )
    {
        return beam_pattern_model::reference() ;
    }
    return beam_pattern_map::instance()->find(beamID) ;
}
