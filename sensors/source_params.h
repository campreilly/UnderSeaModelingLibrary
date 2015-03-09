/**
 *  @file source_params.h
 *  Definition of the Class source_params
 *  Created on: 10-Feb-2015 12:49:09 PM
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
 * Models all the sensor parameter characteristics which are unique to the source.
 * Contains the references to the beam_pattern and the beam_pattern_map.
 *
 * @author Ted Burns, AEgis Technologies Group, Inc.
 * @version 1.0
 * @created 10-Feb-2015 12:49:09 PM
 */
class USML_DECLSPEC source_params
{
public:
    /**
    * Constructor
    * @param sourceID
    * @param sourceStrength
    * @param transmitFrequency
    * @param initialPingTime
    * @param repeationInterval
    * @param source_beam
    */
    source_params(const paramsIDType sourceID, const double sourceStrength, 
                    const double transmitFrequency, const double initialPingTime,
                    const double repeationInterval, std::list<beamIDType>& beamList);

    /**
     * Copy Constructor - Deep
     */
    source_params(const source_params& other);

    /**
     * Destructor
     */
    virtual ~source_params() {}

    /**
    * Set method for the sourceID attribute. The sourceID attribute is used as the
    * key to lookup the source_params in the source_params_map.
    * @param sourceID of the paramsIDType.
    */
    void sourceID(paramsIDType sourceID)
    {
        _sourceID = sourceID;
    }

    /**
    * Get method for the sourceID attribute.
    * @return sourceID of the paramsIDType
    */
    paramsIDType sourceID()
    {
        return _sourceID;
    }

    /**
     * Add a beam pattern to the source parameters
     * @params beamID to be added to the list of source beams
     */
    void add_beam_pattern(beamIDType beamID);

    /**
     * Delete a beam pattern from the source list
     * @params beamID to be removed from the list of source beams
     */
    void remove_beam_pattern(beamIDType beamID);

	void ping();

private:

    /**
     * Prevent access to default constructor
     */
    source_params();

    /**
     * Prevent access to assignment operator
     */
    source_params& operator=(source_params const&);


	paramsIDType _sourceID;
	double _sourceStrength;
	double _transmitFrequency;
	double _initialPingTime;
	double _repeationInterval;
    std::list<beamIDType> _source_beams;
};

/// @}
} // end of namespace sensors
} // end of namespace usml
