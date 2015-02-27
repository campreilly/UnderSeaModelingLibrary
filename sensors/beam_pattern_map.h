/**
 *  @file beam_pattern_map.h
 *  Definition of the Class beam_pattern_map
 *  Created on: 10-Feb-2015 12:49:08 PM
 */

#pragma once

#include <usml/usml_config.h>
#include <usml/sensors/singleton_map.h>
#include <usml/sensors/beamIDType.h>
#include <usml/sensors/beam_pattern_model.h>

namespace usml {
namespace sensors {

/// @ingroup sensors
/// @{

/**
 * Storage for all the beam_pattern_model's in use by the USML. This class
 * inherits from the templated singleton_map class. The map stores pointers to
 * beam_pattern_model and take's ownership of the pointers. See
 * usml/sensors/singleton_map.h A typedef of beamIDType has been defined to allow
 * for modification of the key of the map at a later time if needed.
 *
 * @author Ted Burns, AEgis Technologies Inc.
 * @version 1.0
 * @updated 27-Feb-2015 3:14:26 PM
 */
class USML_DECLSPEC beam_pattern_map : public singleton_map <const beamIDType, const beam_pattern_model*>
{

public:
    /**
    * Destructor - See singleton_map destructor.
    */
    virtual ~beam_pattern_map();


protected:
    /**
    * Default Constructor
    *   Protected to prevent access.
    */
    beam_pattern_map();

private:
    /**
     * Prevent access to copy constructor
     */
    beam_pattern_map(beam_pattern_map const&);

    /**
     * Prevent access to assignment operator
     */
    beam_pattern_map& operator=(beam_pattern_map const&);
};

/// @}
} // end of namespace sensors
} // end of namespace usml
