/**
 *  @file source_params.h
 *  Definition of the Class source_params
 *  Created on: 10-Feb-2015 12:49:09 PM
 */

#pragma once

#include <usml/usml_config.h>
#include <usml/sensors/paramsIDType.h>
#include <usml/sensors/beam_pattern_model.h>
#include <usml/sensors/beam_pattern_map.h>

namespace usml {
namespace sensors {

/**
 * @author tedburns
 * @version 1.0
 * @created 10-Feb-2015 12:49:09 PM
 */
class USML_DECLSPEC source_params
{

public:
	source_params();
	virtual ~source_params();
	void ping();

private:
	paramsIDType _sourceID;
	double _sourceStrength;
	double _transmitFrequency;
	double _initialPingTime;
	double _repeationInterval;
	beam_pattern_model* _source_beam;
	beam_pattern_map* _beam_pattern_map;

};

} // end of namespace sensors
} // end of namespace usml