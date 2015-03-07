/**
 *  @file source_params.h
 *  Definition of the Class source_params
 *  Created on: 10-Feb-2015 12:49:09 PM
 */

#pragma once

#include <usml/usml_config.h>
#include <usml/sensors/paramsIDType.h>
#include <usml/sensors/beam_pattern_model.h>

namespace usml {
namespace sensors {

/// @ingroup sensors
/// @{

/**
 * Models all the sensor parameter characteristics which are unique to the source.
 * Contains the references to the beam_pattern and the beam_pattern_map.
 */
/**
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
    * @param beamMap
    */
    source_params(const paramsIDType sourceID, const double sourceStrength, 
                    const double transmitFrequency, const double initialPingTime,
                    const double repeationInterval,beam_pattern_model* source_beam);
    /**
     * Prevent access to default constructor
     */
    source_params();


     // Copy Constructor - Deep
    source_params(const source_params& other);

    // Destructor
    virtual ~source_params();

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

	void ping();

protected:

    /**
     * Prevent access to assignment operator
     */
    source_params& operator=(source_params const&);

private:

	paramsIDType _sourceID;
	double _sourceStrength;
	double _transmitFrequency;
	double _initialPingTime;
	double _repeationInterval;
    beam_pattern_model* _source_beam;
};

/// @}
} // end of namespace sensors
} // end of namespace usml
