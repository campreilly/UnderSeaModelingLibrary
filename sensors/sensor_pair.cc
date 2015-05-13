/**
 * @file sensor_pair.cc
 * Container for one sensor pair instance.
 */
#include <usml/sensors/sensor_pair.h>
#include <usml/waveq3d/eigenray_interpolator.h>
#include <boost/foreach.hpp>

using namespace usml::sensors;
using namespace usml::waveq3d;

/**
* Utility to build the intersecting frequencies of a sensor_pair.
*/
void sensor_pair::frequencies() {

    // Build intersecting frequencies
    _frequencies = _source->frequencies()->clip(
        _receiver->min_active_freq(), _receiver->max_active_freq());

    // Get first and last values
    double first_value = *( _frequencies->begin() );
    double last_value = *( _frequencies->end() - 1 );
   
    // Set first and last index's
    _src_freq_first = _source->frequencies()->find_index(first_value);
    _src_freq_last = _source->frequencies()->find_index(last_value);
}


/**
 * Notification that new eigenray data is ready.
 */
void sensor_pair::update_fathometer(sensor_model::id_type sensorID, eigenray_list* list)
{
    write_lock_guard guard(_fathometer_mutex);
    #ifdef USML_DEBUG
        cout << "sensor_pair: update_fathometer("
             << sensorID << ")" << endl ;
    #endif
   
    seq_vector* original_freq = NULL;

    if ( list != NULL ) {
        // If sensor that made this call is the _receiver of this pair
        //    then Swap de's, and az's
        if (sensorID == _receiver->sensorID()) {
            BOOST_FOREACH(eigenray ray, *list) {
                std::swap(ray.source_de, ray.target_de);
                std::swap(ray.source_az, ray.target_az);
            }
            // Get frequencies from receiver
            original_freq = _receiver->frequencies();
        } else {
            // Get frequencies from source
            original_freq = _source->frequencies();
        }

        // Interpolate to this sensor_pair's bounded frequencies
        eigenray_list new_eigenray_list;
        new_eigenray_list.resize(list->size());

        // Set frequencies for new eigenray_list
         BOOST_FOREACH(eigenray ray, new_eigenray_list) {
             ray.frequencies = _frequencies;
         }

         eigenray_interpolator interpolator( original_freq, _frequencies ) ;
         interpolator.interpolate( *list, &new_eigenray_list ) ;

        // Note new memory location for eigenrays is create here
        _fathometer = fathometer_model::reference ( new fathometer_model(
            _source->sensorID(),_receiver->sensorID(), _source->position(),
            _receiver->position(), new_eigenray_list));
    }
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

        if (sensor == _source) {
            write_lock_guard guard(_src_eigenverbs_mutex);
            _src_eigenverbs = sensor->eigenverbs();
        }
        if (sensor == _receiver) {
            write_lock_guard guard(_rcv_eigenverbs_mutex);
            _rcv_eigenverbs = sensor->eigenverbs();
        }

        // TODO Create and run the envelope_generator
        // run_envelope_generator();

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
