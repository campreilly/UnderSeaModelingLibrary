/**
 * @file sensor_listener.h
 * Abstract interface for callbacks from sensor objects.
 */
#pragma once

#include <usml/sensors/sensor.h>

namespace usml {
namespace sensors {

class sensor;

using namespace usml::sensors ;

/// @ingroup sensors
/// @{

/**
 * Abstract interface for callbacks from sensor objects.
 * This class is part of a Observer/Subject pattern for the sensor class.
 * and allows other classes to call methods in the class that implemented it.
 * The update_fathometers and update_eigenverbs methods must be defined in
 * each class which inherits it. The primary purpose for this class was to access
 * sensor_pair data.
 */
class USML_DECLSPEC sensor_listener {
public:

	/**
	 * Data type used for reference to sensor_listener.
	 */
	typedef shared_ptr<sensor_listener> reference;

	/**
	 * Destructor.
	 */
	virtual ~sensor_listener() {
	}

	/**
	 * Notification that new fathometer data is ready.
	 *
	 * @param	from	Sensor that issued the notification.
	 */
	virtual void update_fathometers(shared_ptr<sensor>& from) = 0;

	/**
	 * Notification that new eigenverb data is ready.
	 *
	 * @param	from	Sensor that issued the notification.
	 */
	virtual void update_eigenverbs(shared_ptr<sensor>& from) = 0;


	/**
	 * Notification that a sensor is about to be deleted.
	 *
	 * @param	from	Sensor that issued the notification.
	 */
	virtual void remove_sensor(shared_ptr<sensor>& from) = 0;


	/**
	 * Queries for the sensor pair complements of this sensor.
	 *
	 * @param	from	Sensor that issued the notification.
	 */
	virtual shared_ptr<sensor> sensor_complement(shared_ptr<sensor>& from) const = 0;

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
}// end of namespace sensors
} // end of namespace usml

