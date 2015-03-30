/**
 *  @file sonobuoy.cc
 *  Implementation of the Class sonobuoy
 *  Created on: 10-Feb-2015 12:49:09 PM
 */

#include "sonobuoy.h"

using namespace usml::sensors;

sonobuoy::sonobuoy(sensor::id_type sensorID, sensor_params::id_type paramsID,
    const std::string& description)
    : sensor(sensorID, paramsID, description),
	  _radio_channel(-1),
	  _frequency_band(-1),
	  _uplink_enabled(false)
{
}
