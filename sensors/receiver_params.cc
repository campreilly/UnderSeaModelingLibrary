/**
 *  @file receiver_params.cc
 *  Implementation of the Class receiver_params
 *  Created on: 10-Feb-2015 12:49:09 PM
 */

#include "receiver_params.h"

using namespace usml::sensors;

/**
* Constructor
* @param receiverID
* @param beamList
* @param beamMap
*/
receiver_params::receiver_params(const paramsIDType receiverID, const std::list<beam_pattern_model*> beamList, beam_pattern_map* beamMap)
    :   _receiverID(receiverID),
        _receiver_beams(beamList),
        _beam_pattern_map(beamMap)
{

}

// Default Constructor
receiver_params::receiver_params()
{

}

// Destructor
receiver_params::~receiver_params()
{
   
}

