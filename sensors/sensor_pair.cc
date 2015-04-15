/**
 * @file sensor_pair.cc
 * Container for one sensor pair instance.
 */
#include <usml/sensors/sensor_pair.h>

using namespace usml::sensors;

/**
 * Notification that new eigenray data is ready.
 */
void sensor_pair::update_eigenrays(sensor_model::id_type sensorID, shared_ptr<eigenray_list> list)
{
    write_lock_guard guard(_eigenrays_mutex);
    #ifdef USML_DEBUG
        cout << "sensor_pair: update_eigenrays("
             << sensorID << ")" << endl ;
    #endif

    // If sensor that made this call is the _receiver of this pair
    //    then Swap de's, and az's
    if (sensorID == _receiver->sensorID()) {

        eigenray_list* eigenrays = list.get();
        BOOST_FOREACH( eigenray ray, *eigenrays) {
            std::swap(ray.source_de, ray.target_de);
            std::swap(ray.source_az, ray.target_az);
        }
    }
    #ifdef NOOP
    cout << "sensor_pair: update_eigenrays inserting eigenray_list" << endl ;
    #endif
    _eigenrays = list;
}

/**
 * Updates the sensors eigenverb_collection
 */
void sensor_pair::update_eigenverbs(sensor_model* sensor)
{
	if (sensor != NULL) {

		#ifdef USML_DEBUG
			cout << "sensor_pair: update_eigenverbs("
				 << sensor->sensorID() << ")" << endl ;
		#endif

// TODO
//        if (sensor == _source.get()) {
//            write_lock_guard guard(_src_eigenverbs_mutex);
//            _src_eigenverbs = sensor->eigenverbs();
//        }
//        if (sensor == _receiver.get()) {
//            write_lock_guard guard(_rcv_eigenverbs_mutex);
//            _rcv_eigenverbs = sensor->eigenverbs();
//        }
	}
}

/**
 * Queries for the sensor pair complements of this sensor.
 */
const sensor_model* sensor_pair::sensor_complement(const sensor_model* sensor) const
{
    read_lock_guard guard(_complements_mutex);
	if (sensor != NULL) {
		if (sensor == _source) {
			return _receiver;
		} else {
			return _source;
		}
	} else {
		return NULL;
	}
}
