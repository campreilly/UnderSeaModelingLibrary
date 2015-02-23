/**
 *  @file source_params.cc
 *  Implementation of the Class source_params
 *  Created on: 10-Feb-2015 12:49:09 PM
 */

#include "source_params.h"

using namespace usml::sensors;

/**
* Constructor
* @param sourceID
* @param sourceStrength
* @param transmitFrequency
* @param initialPingTime
* @param repeationInterval
* @param beamList
* @param beamMap
*/
source_params::source_params(const paramsIDType sourceID, const double sourceStrength,
                            const double transmitFrequency, const double initialPingTime, const double repeationInterval,
                            beam_pattern_model* source_beam, beam_pattern_map* beamMap)
    :   _sourceID(sourceID),
        _sourceStrength(sourceStrength),
        _transmitFrequency(transmitFrequency),
        _initialPingTime(initialPingTime),
        _repeationInterval(repeationInterval),
        _source_beam(source_beam),
        _beam_pattern_map(beamMap)
{

}

// Default Constructor
source_params::source_params()
{

}

// Destructor
source_params::~source_params()
{

}

void source_params::ping()
{

}