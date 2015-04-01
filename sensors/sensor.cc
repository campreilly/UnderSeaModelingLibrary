/**
 *  @file sensor.cc
 *  Implementation of the Class sensor
 *  Created on: 10-Feb-2015 12:49:09 PM
 */
#include <usml/sensors/sensor.h>
#include <usml/sensors/source_params_map.h>
#include <usml/sensors/receiver_params_map.h>
#include <usml/eigenverb/wavefront_generator.h>
#include <usml/ocean/ocean_shared.h>
#include <boost/foreach.hpp>

using namespace usml::sensors;

/**
 * Construct a new instance of a specific sensor type.
 */
sensor::sensor(sensor::id_type sensorID, sensor_params::id_type paramsID,
	const std::string& description)
	: _sensorID(sensorID), _paramsID(paramsID), _description(description),
	  _position(NAN, NAN, NAN), _orientation(NAN, NAN, NAN)
{
	_source = source_params_map::instance()->find(paramsID);
	_receiver = receiver_params_map::instance()->find(paramsID);
}

/**
 * Removes a sensor instance from simulation.
 */
sensor::~sensor() {
	if ( _wavefront_task.get() != 0 ) {
		_wavefront_task->abort();
	}
}

/**
 * Queries the sensor's ability to support source and/or receiver behaviors.
 */
xmitRcvModeType sensor::mode() const {
	bool has_source = _source.get() != NULL;
	bool has_receiver = _receiver.get() != NULL;
	xmitRcvModeType result;
	if (has_source && has_receiver) {
		result = BOTH;
	} else if (has_source) {
		result = SOURCE;
	} else if (has_receiver) {
		result = RECEIVER;
	} else {
		result = NONE;
	}
	return result;
}

/**
 * Location of the sensor in world coordinates.
 */
wposition1 sensor::position() const {
	read_lock_guard guard(_update_sensor_mutex);
	return _position;
}

/**
 * Orientation of the sensor in world coordinates.
 */
sensor_orientation sensor::orientation() const {
	read_lock_guard guard(_update_sensor_mutex);
	return _orientation;
}

/**
 * Checks to see if new position and orientation have changed enough
 * to require a new WaveQ3D run.
 */
void sensor::update_sensor(const wposition1& position,
		const sensor_orientation& orientation, bool force_update) {
	write_lock_guard guard(_update_sensor_mutex);
	if (!force_update) {
		if (!check_thresholds(position, orientation)) {
			return;
		}
	}
	#ifdef USML_DEBUG
		cout << "sensor: update_sensor(" << sensorID() << ")" << endl ;
	#endif
	_position = position;
	_orientation = orientation;
	run_wave_generator();
}

/**
 * Last set of fathometers computed for this sensor.
 * Blocks during updates from the wavefront task.
 */
eigenray_collection::reference sensor::fathometers() const {
	read_lock_guard guard(_update_fathometers_mutex);
	return _fathometers;
}

/**
 * Asynchronous update of fathometer data from the wavefront task.
 */
void sensor::update_fathometers(shared_ptr<eigenray_collection>& fathometers) {
	write_lock_guard guard(_update_fathometers_mutex);
	#ifdef USML_DEBUG
		cout << "sensor: update_fathometers(" << sensorID() << ")" << endl ;
	#endif
	_fathometers = fathometers;
	sensor::reference from(this) ;
	BOOST_FOREACH( sensor_listener::reference listener, _sensor_listeners ) {
		listener->update_fathometers(from);
	}
}

/**
 * Last set of eigenverbs computed for this sensor.
 */
eigenverb_collection::reference sensor::eigenverbs() const {
	read_lock_guard guard(_update_eigenverbs_mutex);
	return _eigenverbs;
}

/**
 * Asynchronous update of eigenverbs data from the wavefront task.
 * Passes this data onto all sensor listeners.
 * Blocks until update is complete.
 */
void sensor::update_eigenverbs( eigenverb_collection::reference& eigenverbs ) {
	write_lock_guard guard(_update_eigenverbs_mutex);
	#ifdef USML_DEBUG
		cout << "sensor: update_eigenverbs(" << sensorID() << ")" << endl ;
	#endif
	_eigenverbs = eigenverbs;
	sensor::reference from(this) ;
	BOOST_FOREACH( sensor_listener::reference listener, _sensor_listeners ) {
		listener->update_eigenverbs(from);
	}
}

/**
 * Add a sensor_listener to the _sensor_listeners list
 */
void sensor::add_sensor_listener(sensor_listener::reference listener) {
	write_lock_guard guard(_sensor_listeners_mutex);
	_sensor_listeners.push_back(listener);
}

/**
 * Remove a sensor_listener from the _sensor_listeners list
 */
void sensor::remove_sensor_listener(sensor_listener::reference listener) {
	write_lock_guard guard(_sensor_listeners_mutex);
	_sensor_listeners.remove(listener);
}

/**
 * Checks to see if new position and orientation have changed enough
 * to require a new WaveQ3D run.
 *
 * @todo using dummy values for prototyping
 */
bool sensor::check_thresholds(const wposition1& position,
		const sensor_orientation& orientation)
{
	// force update if old values not valid

	if (isnan(_position.rho()) || isnan(_position.theta())
			|| isnan(_position.phi()) || isnan(_orientation.heading())
			|| isnan(_orientation.pitch()) || isnan(_orientation.roll())) {
		return true;
	}

	// check difference between old and new values

	return true;// using dummy values for prototyping
}

/**
 * Queries the current list of sensor listener for the complements of this sensor.
 */
wposition sensor::sensor_targets() {
	read_lock_guard guard(_sensor_listeners_mutex);

	// query the listeners for the complements of this sensor.

	std::list<sensor::reference> complements;
	sensor::reference from(this) ;
	BOOST_FOREACH( sensor_listener::reference listener, _sensor_listeners ) {
		complements.push_back( listener->sensor_complement(from) );
	}

	// build list of targets from the complements of this sensor.

	wposition target_pos(complements.size(), 1);
	int row = -1;
	BOOST_FOREACH( sensor::reference target, complements ){
		++row;
		wposition1 pos = target->position();
		target_pos.latitude( row, 0, pos.latitude());
		target_pos.longitude(row, 0, pos.longitude());
		target_pos.altitude( row, 0, pos.altitude());
	}
	return target_pos ;
}

/**
 * Run the wave_generator thread task to start the waveq3d model.
 *
 * @todo wave_generator stubbed out until after sensor_pairs tested
 */
void sensor::run_wave_generator() {
	#ifdef USML_DEBUG
		cout << "sensor: run_wave_generator(" << sensorID() << ")" << endl ;
	#endif

    // Create the wavefront_generator
    wavefront_generator* generator = new wavefront_generator();

    // Populate waveq3d settings
    generator->wavefront_listener(this);
    generator->sensor_position(_position);

//    // Get the targets
//    wposition targets = sensor_targets();
//    generator->targets(targets);
//
//    // Get the frequencies
//    generator->frequencies(_source->frequencies());
//    generator->ocean(ocean_shared::current());
//
//    // Make wavefront_generator a wavefront_task, with use of shared_ptr
//    _wavefront_task = thread_task::reference(generator);
//
//    // Pass in to thread_pool
//    thread_controller::instance()->run(_wavefront_task);
}
