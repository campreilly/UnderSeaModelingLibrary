/**
 * @file receiver_params.cc
 * Sensor characteristics for the receiver behaviors of a sensor.
 */
#include <usml/sensors/receiver_params.h>
#include <usml/sensors/beam_pattern_map.h>
#include <boost/foreach.hpp>
#include <algorithm>

using namespace usml::sensors;

/**
 * Construct new class of receiver.
 */
receiver_params::receiver_params( sensor_params::id_type paramsID,
    bool multistatic, const std::list<beam_pattern_model::id_type>& beamList)
: sensor_params( paramsID, multistatic )
{
	_beam_patterns.resize( beamList.size() ) ;
	BOOST_FOREACH( beam_pattern_model::id_type beamID, beamList ) {
		_beam_patterns[beamID] = beam_pattern_map::instance()->find(beamID) ;
	}
}
