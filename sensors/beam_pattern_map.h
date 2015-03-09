/**
 *  @file beam_pattern_map.h
 *  Definition of the Class beam_pattern_map
 *  Created on: 10-Feb-2015 12:49:08 PM
 */

#pragma once

#include <usml/usml_config.h>

#include <usml/sensors/beams.h>
#include <usml/sensors/beamIDType.h>
#include <usml/sensors/map_template.h>

namespace usml {
namespace sensors {

/// @ingroup sensors
/// @{

/**
 * Container for all the beam_pattern_model's in use by the USML. This class inherits
 * from the map_template class. This class implements the singleton GOF pattern.
 * The map stores pointers to beam_pattern_model's and take's ownership of the pointers.
 * See usml/sensors/map_template.h A typedef of beamIDType has been defined
 * to allow for modification of the key of the map at a later time if needed.
 *
 * @author Ted Burns, AEgis Technologies Group, Inc.
 * @version 1.0
 * @updated 27-Feb-2015 3:14:26 PM
 */
class USML_DECLSPEC beam_pattern_map : public map_template <const beamIDType, const beam_pattern_model*>
{

public:

    /**
     * Singleton Constructor - Creates beam_pattern_map instance just once.
     * Accessible everywhere.
     * @return  pointer to the instance of the singleton beam_pattern_map
     */
    static beam_pattern_map* instance();

    /**
     * Destructor - See map_template destructor.
     */
    virtual ~beam_pattern_map() {}

private:

    /**
     * Default Constructor
     *   Prevent creation/access other than static instance()
     */
    beam_pattern_map() {}

    /**
     * Prevent access to copy constructor
     */
    beam_pattern_map(beam_pattern_map const&);

    /**
     * Prevent access to assignment operator
     */
    beam_pattern_map& operator=(beam_pattern_map const&);

    /**
     * The singleton access pointer.
     */
    static beam_pattern_map* _instance;
};

/// @}
} // end of namespace sensors
} // end of namespace usml
