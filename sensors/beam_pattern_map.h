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

/// @ingroup sensors
/// @{

/** 
 * This class holds all the beam_pattern_model's in use by the USML. It is defined as a GOF 
 * singleton pattern, that has a std::map which implements the insert and find methods.
 * std::map is a sorted associative container that contains key-value pairs with unique keys. 
 * A typedef of beamIDType has been defined to allow for modification of the Key of the map
 * at a later time if needed.
 *
 * Design Rational
 *  The beam_pattern_map was designed to contain a std::map vs inheriting from a std::map.
 *  Reasons are as follows:
 *      General OOD principal to constrain the public API, std::map is wide open.
 *      Limited API provides for easily extendible child classes.
 *      Unwrapping std:map calls will all be internal, allowing for cleaner beam_pattern_map calls.
 *
 * @author Ted Burns, AEgis Technologies Inc.
 * @version 1.0
 * @created 10-Feb-2015 12:49:08 PM
 */
class USML_DECLSPEC beam_pattern_map
{

public:

    /**
    * Singleton Constructor - Creates beam_pattern_map instance just once, then
    * Accessible everywhere.
    * @return  pointer to the instance of the beam_pattern_map.
    */
    static beam_pattern_map* instance();

    /**
    * Destructor - Deletes pointers to beam_pattern_model's
    */
    virtual ~beam_pattern_map();

    /**
    * finds the beam_pattern_model associated with the beamID key.
    * @param beamID is the associated key. 
    * @return const beam_pattern_model pointer.
    */
    const beam_pattern_model* find(const beamIDType beamID);

    /**
    * Inserts the supplied beam_pattern_model into the map with the 
    * key provided.
    * @param beamID is the associated key to the beam_pattern_model.
    * @param beam_pattern_model to be inserted.
    */
    void insert(const beamIDType beamID, const beam_pattern_model* beam_pattern);

protected:
    /**
    * Default Constructor
    *   Protected to prevent access.
    */
    beam_pattern_map();

private:
    /**
    * The singleton access pointer.
    */
    static beam_pattern_map* _instance;

    /**
     * The singleton access pointer.
     */
    std::map <const beamIDType, const beam_pattern_model*> _map;
};

/// @}
} // end of namespace sensors
} // end of namespace usml
