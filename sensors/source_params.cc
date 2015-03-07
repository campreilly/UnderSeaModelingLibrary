/**
 *  @file source_params.cc
 *  Implementation of the Class source_params
 *  Created on: 10-Feb-2015 12:49:09 PM
 */

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
                            const double transmitFrequency, const double initialPingTime, const double repeationInterval,
                            beam_pattern_model* source_beam)
    :   _sourceID(sourceID),
        _sourceStrength(sourceStrength),
        _transmitFrequency(transmitFrequency),
        _initialPingTime(initialPingTime),
        _repeationInterval(repeationInterval),
        _source_beam(source_beam)
{
}

// Default Constructor
source_params::source_params()
{
}

// Copy Constructor - Deep
source_params::source_params(const source_params& other)
    : _sourceID(other._sourceID),
      _sourceStrength(other._sourceStrength),
      _transmitFrequency(other._transmitFrequency),
      _initialPingTime(other._initialPingTime),
      _repeationInterval(other._repeationInterval),
      _source_beam(other._source_beam->clone())
{
}

// Destructor
source_params::~source_params()
{
    if (_source_beam != NULL)
        delete _source_beam;
}

void source_params::ping()
{

}
