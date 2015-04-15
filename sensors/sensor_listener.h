/**
 * @file sensor_listener.h
 * Abstract interface for callbacks from sensor objects.
 */
#pragma once

#include <usml/sensors/sensor_model.h>

namespace usml {
namespace sensors {

class sensor_model;


using namespace usml::sensors ;
using namespace usml::waveq3d ;

/// @ingroup sensors
/// @{

/**
 * Abstract interface for callbacks from sensor objects.
 * This class is part of a Observer/Subject pattern for the sensor class.
 * and allows other classes to call methods in the class that implemented it.
 * The update_eigenrays and update_eigenverbs methods must be defined in
 * each class which inherits it. The primary purpose for this class was to access
 * sensor_pair data.
 */
class USML_DECLSPEC sensor_listener {
public:

	/**
	 * Data type used for reference to sensor_listener.
	 */
	//typedef shared_ptr<sensor_listener> reference;

	/**
	 * Destructor.
	 */
	virtual ~sensor_listener() {
	}

	/**
	 * Notification that new eigenray data is ready.
	 *
	 * @param	sensorID The ID of the sensor that issued the notification.
	 * @param   eigenray_list Shared pointer to the list of eigenrays
	 */
	virtual void update_eigenrays(int sensorID, shared_ptr<eigenray_list> list) = 0;

	/**
	 * Notification that new eigenverb data is ready.
	 *
	 * @param	sensor	Pointer to sensor that issued the notification.
	 */
	virtual void update_eigenverbs(sensor_model* sensor) = 0;

	/**
	 * Queries for the sensor pair complements of this sensor.
     *
	 * @param	sensor	Sensor that issued the notification.
     * @return  Pointer to the sensor complement of this pair.
	 */

    virtual const sensor_model* sensor_complement(const sensor_model* sensor) const = 0;

protected:

	/**
	 * Constructor - protected
	 */
	sensor_listener() {
	}

private:

	/**
	 * Disabling default copy constructor and default
	 * assignment operator.
	 */
	sensor_listener(const sensor_listener& yRef);
	sensor_listener& operator=(const sensor_listener& yRef);
};

/// @}
} // end of namespace sensors
} // end of namespace usml

