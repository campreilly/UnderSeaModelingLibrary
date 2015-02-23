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
    /**
    * Constructor
    * @param receiverID
    * @param beamList
    * @param beamMap
    */
    receiver_params(const paramsIDType receiverID, const std::list<beam_pattern_model*> beamList, beam_pattern_map* beamMap);

    // Default Constructor
    receiver_params();

    // Destructor
    virtual ~receiver_params();

    /**
    * Set method for the _receiverID attribute. The _receiverID attribute is used as the
    * key to lookup the receivee_params in the receiver_params_map.
    * @param receiverID of the paramsIDType.
    */
    void receiverID(paramsIDType receiverID)
    {
        _receiverID = receiverID;
    }

    /**
    * Get method for the receiverID attribute.
    * @return receiverID of the paramsIDType
    */
    paramsIDType receiverID()
    {
        return _receiverID;
    }

private:
    paramsIDType _receiverID;
    beam_pattern_map* _beam_pattern_map;
    std::list<beam_pattern_model*> _receiver_beams; 
};

} // end of namespace sensors
} // end of namespace sensors