/**
 *  @file sonobuoy.h
 *  Definition of the Class sonobuoy
 *  Created on: 10-Feb-2015 12:49:09 PM
 */

#pragma once

#include <usml/sensors/sensor.h>

namespace usml {
namespace sensors {

/**
 * @author tedburns
 * @version 1.0
 * @created 10-Feb-2015 12:49:09 PM
 */
class USML_DECLSPEC sonobuoy : public sensor
{

public:
	sonobuoy();
	virtual ~sonobuoy();

private:
	int _radioChannel;
	int _frequencyBand;
	int _uplinkEnabled;

};

} // end of namespace sensors
} // end of namespace usml