/**
 *  @file sensor_map.h
 *  Definition of the Class sensor_map
 *  Created on: 12-Feb-2015 3:41:30 PM
 */

#pragma once

#include <map>

#include <usml/usml_config.h>
#include <usml/sensors/sensorIDType.h>
#include <usml/sensors/sensor.h>
#include <usml/sensors/source_params_map.h>
#include <usml/sensors/receiver_params_map.h>

namespace usml {
namespace sensors {

using namespace usml::sensors;

/**
 * @author tedburns
 * @version 0.1
 * @created 12-Feb-2015 3:41:30 PM
 */
class USML_DECLSPEC sensor_map : public std::map < sensorIDType, sensor* >
{
public:

    /**
     * Singleton Constructor
     *   Creates receiver_params_map instance just once, then accesable everywhere.
     */
    static sensor_map* instance();

    /**
     * Destructor
     */
	virtual ~sensor_map();

	/**
	 * 
	 * @param sensorID
	 * @param sensor
	 */
	void insert(const sensorIDType sensorID, const sensor* sensor);

	/**
	 * 
	 * @param sensorID
	 */
	void erase(const sensorIDType sensorID);

	/**
	 * 
	 * @param sensorID
	 * @param sensor
	 */
	void update(const sensorIDType sensorID, const sensor* sensor);

	/**
	 * 
	 * @param sensorID
	 */
    sensor* find(const sensorIDType sensorID);

protected:
    /**
     * Default Constructor
     *   Protected to prevent access other than instance call
     */
	sensor_map();

private:
    /**
     * The singleton access pointer
     */
	static sensor_map* _instance;

	source_params_map* _source_params_map;
	receiver_params_map* _receiver_params_map;

};

} // end of namespace sensors
} // end of namespace usml