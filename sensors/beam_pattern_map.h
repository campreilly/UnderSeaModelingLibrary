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

/// @ingroup senosrs
/// @{

/** 
 * This class holds all the beam_pattern_model's in use by the USML. It is defined as a GOF 
 * singleton pattern and inherits the std::map that overloads the insert and find methods. 
 * std::map is a sorted associative container that contains key-value pairs with unique keys. 
 * A typedef of beamIDType has been defined to allow for modification of the Key of the map
 * at a later time if needed. 
 * @author Ted Burns, AEgis Technologies Inc.
 * @version 1.0
 * @created 10-Feb-2015 12:49:08 PM
 */
class USML_DECLSPEC beam_pattern_map : public std::map <beamIDType, beam_pattern_model*>
{

public:

    /**
    * Singleton Constructor - Creates beam_pattern_map instance just once, when then 
    * accesable everywhere.
    * @return  pointer to the instance of the beam_pattern_map.
    */
    static beam_pattern_map* instance();

    /**
    * Destructor - Noting addtional to destory.
    */
    virtual ~beam_pattern_map();

    /**
    * finds the beam_pattern_model associated with the beamID key.
    * @param beamID is the associated key. 
    * @return beam_pattern_model pointer. 
    */
    beam_pattern_model* find(const beamIDType beamID);

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

};

/// @}
} // end of namespace sensors
} // end of namespace usml