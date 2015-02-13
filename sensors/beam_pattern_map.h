/**
 *  @file beam_pattern_map.h
 *  Definition of the Class beam_pattern_map
 *  Created on: 10-Feb-2015 12:49:08 PM
 */

#pragma once

#include <map>

#include <usml/usml_config.h>
#include <usml/sensors/beamIDType.h>
#include <usml/sensors/beam_pattern_model.h>

namespace usml {
namespace sensors {

/**
 * @author tedburns
 * @version 0.1
 * @created 10-Feb-2015 12:49:08 PM
 */
class USML_DECLSPEC beam_pattern_map : public std::map <beamIDType, beam_pattern_model>
{

public:

    /**
    * Sigleton Constructor
    *   Creates receiver_params_map instance just once, then accesable everywhere.
    */
    static beam_pattern_map* instance();

    /**
    * Destructor
    */
    virtual ~beam_pattern_map();

    /**
    *
    * @param beamID
    * @return beam_pattern_model
    */
    beam_pattern_model* find(const beamIDType beamID);

    /**
    *
    * @param beamID
    * @param beam_pattern
    */
    void insert(const beamIDType beamID, const beam_pattern_model beam_pattern);

protected:
    /**
    * Default Constructor
    *   Protected to prevent access other than instance call
    */
    beam_pattern_map();

private:
    /**
    * The singleton access pointer
    */
    static beam_pattern_map* _instance;

};

/// @}
} // end of namespace sensors
} // end of namespace usml