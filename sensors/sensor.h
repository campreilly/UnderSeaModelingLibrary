/**
 *  @file sensor.h
 *  Definition of the Class sensor
 *  Created on: 10-Feb-2015 12:49:09 PM
 */

#pragma once

#include <list>

#include <usml/sensors/sensorIDType.h>
#include <usml/sensors/paramsIDType.h>
#include <usml/sensors/xmitRcvModeType.h>

#include <usml/sensors/source_params.h>
#include <usml/sensors/receiver_params.h>

#include <usml/waveq3d/wave_queue.h>

namespace usml {
namespace sensors {

using namespace usml::waveq3d;

/// @ingroup sensors
/// @{

/**
 *
 * @version 1.0
 * @created 10-Feb-2015 12:49:09 PM
 */
class USML_DECLSPEC sensor
{

public:
	sensor();
	virtual ~sensor();
	void update();
	void run_waveq3d();
	std::list<int>* get_envelope();
	std::list<int>* get_discrete_envelope();

private:
    sensorIDType _sensorID;
    paramsIDType _paramsID;
    xmitRcvModeType _xmitRcvMode;

	double _latitude;
	double _longitude;
	double _depth;
	double _tiltAngle;
	double _tiltDirection;
	double _pitch;
    double _yaw;
	
	source_params* _source;
    receiver_params* _receiver;
  
	std::list<int> _eigenverbs;
	wave_queue*    _wave;
	std::string*   _description;	
};

/// @}
}  // end of namespace sensors
}  // end of namespace usml
