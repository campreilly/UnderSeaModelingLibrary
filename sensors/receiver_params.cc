/**
 *  @file receiver_params.cc
 *  Implementation of the Class receiver_params
 *  Created on: 10-Feb-2015 12:49:09 PM
 */

#include <algorithm>

#include "receiver_params.h"

using namespace usml::sensors;

/**
 * Constructor
 * 
 * @param receiverID
 * @param beamList
 */
receiver_params::receiver_params(const paramsIDType receiverID, const std::list<beamIDType>& beamList)
    :   _receiverID(receiverID),
        _receiver_beams(beamList)
{
}

// Copy Constructor - Deep
receiver_params::receiver_params(const receiver_params& other)
    : _receiverID(other._receiverID)

{
    std::list<beamIDType>::iterator iter;
    //Delete previous beamID's
    _receiver_beams.clear();

    // Add in new beamID data
    std::list<beamIDType> other_beams_list = other._receiver_beams;
    for ( iter = other_beams_list.begin(); iter != other_beams_list.end(); ++iter ) {
        _receiver_beams.push_back(*iter);
    }
}

// add_beam_pattern
void receiver_params::add_beam_pattern(beamIDType beamID)
{
    _receiver_beams.push_back(beamID);
}

// remove_beam_pattern
void receiver_params::remove_beam_pattern(beamIDType beamID)
{
    std::list<beamIDType>::iterator findIter;
    findIter = std::find(_receiver_beams.begin(), _receiver_beams.end(), beamID);
    if (findIter != _receiver_beams.end()){
        _receiver_beams.erase(findIter);
    }
}

