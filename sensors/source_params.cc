/**
 *  @file source_params.cc
 *  Implementation of the Class source_params
 *  Created on: 10-Feb-2015 12:49:09 PM
 */

#include <algorithm>

#include "source_params.h"

using namespace usml::sensors;

/**
 * Constructor
 * 
 * @param sourceID
 * @param sourceStrength
 * @param transmitFrequency
 * @param initialPingTime
 * @param repeationInterval
 * @param source_beam
 */
source_params::source_params(const paramsIDType sourceID, const double sourceStrength,
                            const double transmitFrequency, const double initialPingTime,
                            const double repeationInterval, std::list<beamIDType>& beamList)
    :   _sourceID(sourceID),
        _sourceStrength(sourceStrength),
        _transmitFrequency(transmitFrequency),
        _initialPingTime(initialPingTime),
        _repeationInterval(repeationInterval),
        _source_beams(beamList)
{
}

// Copy Constructor - Deep
source_params::source_params(const source_params& other)
    : _sourceID(other._sourceID),
      _sourceStrength(other._sourceStrength),
      _transmitFrequency(other._transmitFrequency),
      _initialPingTime(other._initialPingTime),
      _repeationInterval(other._repeationInterval),
      _source_beams(other._source_beams)
{
}

// add_beam_pattern
void source_params::add_beam_pattern(beamIDType beamID)
{
    _source_beams.push_back(beamID);
}

// remove_beam_pattern
void source_params::remove_beam_pattern(beamIDType beamID)
{
    std::list<beamIDType>::iterator findIter;
    findIter = std::find(_source_beams.begin(), _source_beams.end(), beamID);
    if (findIter != _source_beams.end()){
        _source_beams.erase(findIter);
    }
}

void source_params::ping()
{

}
