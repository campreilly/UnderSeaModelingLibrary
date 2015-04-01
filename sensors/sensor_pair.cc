/**
 * @file sensor_pair.cc
 * Container for one sensor pair instance.
 */
#include <usml/sensors/sensor_pair.h>

using namespace usml::sensors;

/**
 * Notification that new fathometer data is ready.
 *
 * @param	from	Sensor that issued the notification.
 */
void sensor_pair::update_fathometers(sensor_model::reference& sensor) {
	if (sensor != NULL) {
		#ifdef USML_DEBUG
			cout << "sensor_pair::update_fathometers("
				 << sensor->sensorID() << ")" << endl ;
		#endif

		// TODO
//        if (sensor == _source) {
//            _src_fathometers = from->fathometers();
//        }
//        if (sensor == _receiver) {
//            _rcv_fathometers = from->fathometers();
//        }
	}
}

/**
 * Updates the sensors eigenverb_collection
 */
void sensor_pair::update_eigenverbs(sensor_model::reference& sensor) {
	if (sensor != NULL) {
		#ifdef USML_DEBUG
			cout << "sensor_pair::update_eigenverbs("
				 << sensor->sensorID() << ")" << endl ;
		#endif

// TODO
//        if (sensor == _source) {
//            _src_eigenverbs = from->eigenverbs();
//        }
//        if (sensor == _receiver) {
//            _rcv_eigenverbs = from->eigenverbs();
//        }
	}
}

/**
 * Sends message to remove the sensor from the sensor_pair_manager
 */
void sensor_pair::remove_sensor(sensor_model::reference& sensor) {
	#ifdef USML_DEBUG
		cout << "sensor_pair::remove_sensor("
			 << sensor->sensorID() << ")" << endl ;
	#endif
}

/**
 * Queries for the sensor pair complements of this sensor.
 */
sensor_model::reference sensor_pair::sensor_complement(sensor_model::reference& sensor) const {
	if (sensor.get() != NULL) {
		if (sensor == _source) {
			return _receiver;
		} else {
			return _source;
		}
	} else {
		return sensor_model::reference();
	}
}
