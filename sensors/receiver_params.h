/**
 *  @file receiver_params.h
 *  Definition of the Class receiver_params
 *  Created on: 10-Feb-2015 12:49:08 PM
 */

#pragma once

#include <list>

#include <usml/usml_config.h>
#include <usml/sensors/paramsIDType.h>
#include <usml/sensors/beam_pattern_model.h>
#include <usml/sensors/beam_pattern_map.h>

namespace usml {
namespace sensors {

/**
 * @author tedburns
 * @version 1.0
 * @created 10-Feb-2015 12:49:08 PM
 */
class USML_DECLSPEC receiver_params 
{

public:
    receiver_params();
    virtual ~receiver_params();

private:
    paramsIDType _receiverID;
    std::list<beam_pattern_model*> _receiver_beams;
    beam_pattern_map* _beam_pattern_map;

};

} // end of namespace sensors
} // end of namespace sensors