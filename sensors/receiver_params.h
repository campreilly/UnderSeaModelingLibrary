/**
 *  @file receiver_params.h
 *  Definition of the Class receiver_params
 *  Created on: 10-Feb-2015 12:49:08 PM
 */

#pragma once

#include <list>

#include <usml/usml_config.h>
#include <usml/sensors/beamIDType.h>
#include <usml/sensors/paramsIDType.h>

namespace usml {
namespace sensors {

/// @ingroup sensors
/// @{

/**
 * Models all the sensor parameter characteristics which are unique to the receiver.
 * Contains the references to the beam_pattern and the beam_pattern_map.
 */
/**
 * @author Ted Burns, AEgis Technologies Group, Inc.
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
     */
    receiver_params(const paramsIDType receiverID, const std::list<beamIDType>& beamList);

    /**
     * Copy Constructor - Deep
     */
    receiver_params(const receiver_params& other);

    /**
     * Destructor
     */
    virtual ~receiver_params() {}

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

    /**
     * Add a beam pattern to the receiver parameters
     * @params beamID to be added to the list of receiver beams
     */
    void add_beam_pattern(beamIDType beamID);

    /**
     * Delete a beam pattern from the receivers list
     * @params beamID to be removed from the list of receiver beams
     */
    void remove_beam_pattern(beamIDType beamID);

private:

    /**
     * Prevent access to default constructor
     */
    receiver_params();

    /**
     * Prevent access to assignment operator
     */
    receiver_params& operator=(receiver_params const&);

    paramsIDType _receiverID;
    std::list<beamIDType> _receiver_beams;
};

/// @}
} // end of namespace sensors
} // end of namespace sensors
