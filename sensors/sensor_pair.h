///////////////////////////////////////////////////////////
//  sensor_pair.h
//  Definition of the Class sensor_pair
//  Created on:      26-Feb-2015 5:46:40 PM
//  Original author: Ted Burns, AEgis Technologies
///////////////////////////////////////////////////////////

#pragma once

#include <usml/usml_config.h>
#include <usml/sensors/sensorIDType.h>
#include <usml/sensors/sensor.h>
#include <usml/sensors/sensor_listener.h>
#include <usml/eigenverb/eigenverb_collection.h>
#include <usml/eigenverb/envelope_collection.h>
#include <usml/eigenverb/proploss_collection.h>

namespace usml {
namespace sensors {

/// @ingroup sensors
/// @{

/**
 * Storage for one instance of a sensor_pair in use by the USML.
 * The sensor pair can be a monostatic or bistatic acositic pair.
 * The sensor_pair_map stores pointers to the sensor_pair's and take's
 * ownership of the pointers. The sensor_pair_map contists of two keys one 
 * for the first_item and oone for the second_item.
 */
class USML_DECLSPEC sensor_pair : public sensor_listener
{
public:

	sensor_pair();
	virtual ~sensor_pair();

	/**
	 * 
	 * @return envelopes
	 */
    envelope_collection get_envelopes();
	/**
	 * 
	 * @return proploss
	 */
    proploss_collection get_fathometers();

	/**
	 * 
	 * @param eigenverbs
	 */
	update_eigenverbs(eigenverb_collection eigenverbs);

	/**
	 * 
	 * @param fathometers
	 */
	update_fathometers(proploss_collection fathometers);

    /**
    *
    * @param envelopes
    */
    update_envelopes(envelope_collection envelopes);

	/**
	 * 
	 * @param sensorID
	 * @param sensor
	 */
    sensor_changed(sensorIDType sensorID, sensor sensor);

private:


};

/// @}
} // end of namespace sensors
} // end of namespace usml
