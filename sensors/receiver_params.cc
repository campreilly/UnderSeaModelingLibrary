/**
 *  @file receiver_params.cc
 *  Implementation of the Class receiver_params
 *  Created on: 10-Feb-2015 12:49:09 PM
 */

#include "receiver_params.h"

using namespace usml::sensors;

/**
 * Constructor
 * 
 * @param receiverID
 * @param beamList
 */
receiver_params::receiver_params(const paramsIDType receiverID, const std::list<beam_pattern_model*> beamList)
    :   _receiverID(receiverID),
        _receiver_beams(beamList)
{

}

// Default Constructor
receiver_params::receiver_params()
{
}

// Copy Constructor - Deep
receiver_params::receiver_params(const receiver_params& other)
    : _receiverID(other._receiverID)

{
    std::list<beam_pattern_model*>::iterator iter;
    //Delete previous beam_pattern_model pointer data
    for ( iter = _receiver_beams.begin(); iter != _receiver_beams.end(); ++iter ) {
       delete (*iter);
    }
    _receiver_beams.clear();

    // Add in new beam_pattern_model data
    std::list<beam_pattern_model*> other_beams_list = other._receiver_beams;
    for ( iter = other_beams_list.begin(); iter != other_beams_list.end(); ++iter ) {
        _receiver_beams.push_back((beam_pattern_model*)(*iter)->clone());
    }
}

// Destructor
receiver_params::~receiver_params()
{
    std::list<beam_pattern_model*>::iterator iter;
    //Delete previous beam_pattern_model pointer data
    for ( iter = _receiver_beams.begin(); iter != _receiver_beams.end(); ++iter )
    {
       delete (*iter);
    }
    _receiver_beams.clear();
}

// add_beam_pattern
void receiver_params::add_beam_pattern(beam_pattern_model* beam_pattern)
{
    _receiver_beams.push_back(beam_pattern);
}

// remove_beam_pattern
void receiver_params::remove_beam_pattern(beam_pattern_model* beam_pattern)
{
    std::list<beam_pattern_model*>::iterator iter;
    for ( iter = _receiver_beams.begin(); iter != _receiver_beams.end(); ++iter )
    {
       beam_pattern_model* test_model = (*iter);
       if (test_model == beam_pattern) {
             delete (*iter);
             _receiver_beams.erase(iter);
       }
    }
}

