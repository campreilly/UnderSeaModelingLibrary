/**
 * @file sensor_pair.cc
 * Container for one sensor pair instance.
 */
#include <usml/sensors/sensor_pair.h>

using namespace usml::sensors;

/**
 * Notification that new fathometer data is ready.
 *
 * @param	sensor	Sensor that issued the notification.
 */
void sensor_pair::update_fathometers(sensor_model::reference& sensor) {

	if (sensor != NULL) {
	    write_lock_guard guard(_eigenrays_mutex);
		#ifdef USML_DEBUG
			cout << "sensor_pair::update_fathometers("
				 << sensor->sensorID() << ")" << endl ;
		#endif

		sensor_model::id_type sensorID;

		// Only Keep eigenrays for this sensor_pair
		// Get complement's sensorID
        if (sensor == _source) {
            sensorID = _receiver->sensorID();
        } else { // receiver
            sensorID = _source->sensorID();
        }
        // Find complement's sensorID's index in eigenray_collection
        const std::set<sensor_model::id_type> target_ids = sensor->target_ids();
        std::set<sensor_model::id_type>::const_iterator iter = target_ids.find(sensorID);
        if (iter != target_ids.end()) {

            // Get receiverID row's eigenray_list
            size_t row = std::distance(target_ids.begin(), iter);
            eigenray_collection::reference fathometers = sensor->fathometers();
            eigenray_collection* proploss = fathometers.get();
            eigenray_list* list = proploss->eigenrays(row, 0);

            if (sensor == _receiver) {
                // Swap de's , and az's on incoming sensor being a RECEIVER
                BOOST_FOREACH( eigenray ray, *list) {
                    std::swap(ray.source_de, ray.target_de);
                    std::swap(ray.source_az, ray.target_az);
                }
            }
            // Insert eigenray_list shared_ptr with lock;
            eigenrays(list);
        }
	}
}

/**
 * Updates the sensors eigenverb_collection
 */
void sensor_pair::update_eigenverbs(sensor_model::reference& sensor)
{
	if (sensor != NULL) {

		#ifdef USML_DEBUG
			cout << "sensor_pair::update_eigenverbs("
				 << sensor->sensorID() << ")" << endl ;
		#endif

// TODO
//        if (sensor == _source) {
//			    write_lock_guard guard(_src_eigenverbs_mutex);
//            _src_eigenverbs = from->eigenverbs();
//        }
//        if (sensor == _receiver) {
//			    write_lock_guard guard(_rcv_eigenverbs_mutex);
//            _rcv_eigenverbs = from->eigenverbs();
//        }
	}
}

/**
 * Queries for the sensor pair complements of this sensor.
 */
sensor_model::reference sensor_pair::sensor_complement(sensor_model::reference& sensor) const
{
    read_lock_guard guard(_complements_mutex);
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
