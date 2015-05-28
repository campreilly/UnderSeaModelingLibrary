/**
 * @file sensor_pair.cc
 * Container for one sensor pair instance.
 */
#include <usml/sensors/sensor_pair.h>
#include <usml/eigenverb/envelope_generator.h>
#include <usml/eigenverb/wavefront_generator.h>
#include <usml/waveq3d/eigenray_interpolator.h>
#include <boost/foreach.hpp>

using namespace usml::sensors;
using namespace usml::waveq3d;

/**
 * Utility to run the envelope_generator
 */
void sensor_pair::run_envelope_generator() {

    #ifdef USML_DEBUG
        cout << "sensor_pair: run_envelope_generator " << endl ;
    #endif

    // Kill any currently running task
    if ( _envelopes_task.get() != 0 ) {
        _envelopes_task->abort();
    }

    // Create the envelope_generator
    envelope_generator* generator = new envelope_generator (
		this, _src_freq_first, wavefront_generator::number_az );

    // Make envelope_generator a _envelopes_task, with use of shared_ptr
    _envelopes_task = thread_task::reference(generator);

    // Pass in to thread_pool
    thread_controller::instance()->run(_envelopes_task);
}

/**
* Utility to build the intersecting frequencies of a sensor_pair.
*/
void sensor_pair::compute_frequencies() {

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
void sensor_pair::update_fathometer(sensor_model::id_type sensor_id, eigenray_list* list)
{
    write_lock_guard guard(_fathometer_mutex);
    #ifdef USML_DEBUG
        cout << "sensor_pair: update_fathometer("
            << sensor_id << ")" << endl;
    #endif
   
    if ( list != NULL ) {
        seq_vector* original_freq = NULL;

        // If sensor that made this call is the _receiver of this pair
        //    then Swap de's, and az's
        if ( sensor_id == _receiver->sensorID() ) {
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

        eigenray_list new_eigenray_list;
        // Only interpolate when needed
        if ( *original_freq != *_frequencies ) {

            // Interpolate to this sensor_pair's bounded frequencies
            // Set frequencies, and space for intensity's,
            // and phase's for new eigenray_list
            size_t num_freq( _frequencies->size() ) ;
            for (int i = 0; i < list->size(); ++i) {
                eigenray new_ray;
                new_ray.frequencies = _frequencies;
                new_ray.intensity.resize( num_freq, false ) ;
                new_ray.phase.resize( num_freq, false ) ;
                new_eigenray_list.push_back(new_ray);
            }

            eigenray_interpolator interpolator( original_freq, _frequencies ) ;
            interpolator.interpolate( *list, &new_eigenray_list ) ;

        } else {
            // Just use original
            new_eigenray_list = *list;
        }
        // Note new memory location for eigenrays is create here
        _fathometer = fathometer_model::reference ( new fathometer_model(
            _source->sensorID(),_receiver->sensorID(), _source->position(),
            _receiver->position(), new_eigenray_list));
    }
}

/**
 * Updates the eigenverb_collection
 */
void sensor_pair::update_eigenverbs(double initial_time, sensor_model* sensor)
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

        if ( _src_eigenverbs.get() != NULL && _rcv_eigenverbs.get() != NULL ) {
            run_envelope_generator();
        }
	}
}

/**
 * Updates new envelope_colection
 */
void sensor_pair::update_envelopes(envelope_collection::reference& collection) {

    if (collection.get() != NULL) {
        write_lock_guard guard(_envelopes_mutex);
        #ifdef USML_DEBUG
            cout << "sensor_pair: update_envelopes src_rcv ("
                << collection.get()->source_id() << "_"
                << collection.get()->receiver_id() <<  ")" << endl ;
        #endif
        _envelopes = collection;
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
