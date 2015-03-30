/**
 * @file sensor.h
 * Instance of an active sensor in the simulation.
 */
#pragma once

#include <list>

#include <usml/threads/thread_controller.h>
#include <usml/threads/read_write_lock.h>
#include <usml/waveq3d/wave_queue.h>
#include <usml/sensors/sensor_params.h>
#include <usml/sensors/xmitRcvModeType.h>

#include <usml/sensors/source_params.h>
#include <usml/sensors/receiver_params.h>
#include <usml/sensors/source_params_map.h>
#include <usml/sensors/receiver_params_map.h>
#include <usml/sensors/sensor_listener.h>
#include <usml/eigenverb/eigenverb_collection.h>
#include <usml/eigenverb/wavefront_generator.h>
#include <usml/eigenverb/wavefront_listener.h>
#include <usml/sensors/sensor_orientation.h>

namespace usml {
namespace sensors {

using namespace usml::waveq3d;
using namespace usml::eigenverb;
using namespace usml::threads;

/// @ingroup sensors
/// @{

/**
 * Instance of an active sensor in the simulation.
 * As the sensor moves all required attributes are updated. If the attributes
 * change beyond established thresholds a new reverb generation is started.
 */
class USML_DECLSPEC sensor: public wavefront_listener {
public:

	/**
	 * Data type used for beamId.
	 */
	typedef int id_type;

	/**
	 * Construct a new instance of a specific sensor type.
	 *
	 * @param sensorID		Identification used to find this sensor instance
	 * 						in sensor_manager.
	 * @param paramsID		Identification used to lookup sensor type data
	 * 						in source_params_map and receiver_params_map.
	 * @param description	Human readable name for this sensor instance.
	 */
	sensor( sensor::id_type sensorID, sensor_params::id_type paramsID,
			const std::string& description = std::string());

	/**
	 * Destructor
	 */
	virtual ~sensor() {}

	//**************************
	// const properties

	/**
	 * Identification used to find this sensor instance in sensor_manager.
	 */
	id_type sensorID() const {
		return _sensorID;
	}

	/**
	 * Identification used to lookup sensor type data in source_params_map and receiver_params_map.
	 */
	sensor_params::id_type paramsID() const {
		return _paramsID;
	}

	/**
	 * Human readable name for this sensor instance.
	 */
	const std::string& description() const {
		return _description;
	}

	/**
	 * Queries the sensor's ability to support source and/or receiver behaviors.
	 */
	xmitRcvModeType mode() const {
		bool has_source = _source.get() != NULL ;
		bool has_receiver = _receiver.get() != NULL ;
		xmitRcvModeType result ;
//		if ( has_source && has_receiver ) {
//			result = xmitRcvModeType::BOTH ;
//		} else if ( has_source ) {
//			result = xmitRcvModeType::SOURCE ;
//		} else if ( has_receiver ) {
//			result = xmitRcvModeType::RECEIVER ;
//		} else {
//			result = xmitRcvModeType::NONE ;
//		}
		return result ;
	}

	/**
	 * Shared pointer to the the source_params for this sensor.
	 */
	source_params::reference source() const {
		return _source;
	}

	/**
	 * Shared pointer to the the receiver_params for this sensor.
	 */
	receiver_params::reference receiver() const {
		return _receiver;
	}

	/**
	 * Location of the sensor in world coordinates.
	 */
	wposition1 position() const {
		read_lock_guard guard(_update_sensor_mutex) ;
		return _position;
	}

	/**
	 * Orientation of the sensor in world coordinates.
	 */
	sensor_orientation orientation() const {
		read_lock_guard guard(_update_sensor_mutex) ;
		return _orientation;
	}

	/**
	 * Updates the sensor data, checks position, pitch, yaw, thresholds
	 * to determine if new wave_generator needs to be run, then kicks 
	 * off the waveq3d model.
	 *
	 * @param position  	Updated position data
	 * @param orientation	Updated orientation value
	 * @param 				When true, forces update without checking thresholds.
	 */
	void update_sensor(const wposition1& position,const sensor_orientation& orientation,bool force_update = false) ;

    /**
     * Last set of fathometers computed for this sensor.
	 * Blocks during updates from the wavefront task.
     * @todo migrate to shared pointer.
     */
	proploss* fathometers() {
		read_lock_guard guard(_update_fathometers_mutex) ;
		return _fathometers;
	}

	/**
	 * Asynchronous update of fathometer data from the wavefront task.
	 * Blocks until update is complete.
	 */
	virtual void update_fathometers(proploss* fathometers) {
		write_lock_guard guard(_update_fathometers_mutex) ;
		_fathometers = fathometers;
		update_fathometer_listeners();
	}

    /**
     * Last set of eigenverbs computed for this sensor.
	 * Blocks during updates from the wavefront task.
     * @todo migrate to shared pointer.
     */
	eigenverb_collection* eigenverbs() {
		read_lock_guard guard(_update_eigenverbs_mutex) ;
		return _eigenverbs;
	}

	/**
	 * Asynchronous update of eigenverbs data from the wavefront task.
	 * Blocks until update is complete.
	 */
	virtual void update_eigenverbs(eigenverb_collection* eigenverbs) {
		write_lock_guard guard(_update_eigenverbs_mutex) ;
		_eigenverbs = eigenverbs;
		update_eigenverb_listeners();
	}

	/**
	 * Add a sensor_listener to the _sensor_listeners list
	 */
	bool add_sensor_listener(sensor_listener* listener);

	/**
	 * Remove a sensor_listener to the _sensor_listeners list
	 */
	bool remove_sensor_listener(sensor_listener* listener);

private:

	/**
	 * Initialize the wave_generator thread  to start the waveq3d model.
	 */
	void init_wave_generator();

	/**
	 * Checks to see if new position and orientation have changed enough
	 * to require a new WaveQ3D run.
	 *
	 * @param position  	Updated position data
	 * @param orientation	Updated orientation value
	 * @return 				True when thresholds exceeded, requiring a
	 * 						rerun of the model for this sensor.
	 */
	bool check_thresholds(wposition1 position, const sensor_orientation& orientation );

	/**
	 * For each sensor_listener in the _sensor_listeners list call the
	 * update_eigenverbs method of each registered class.
	 */
	bool update_eigenverb_listeners();

	/**
	 * For each sensor_listener in the _sensor_listeners list call the
	 * update_fathometers method of each registered class.
	 */
	bool update_fathometer_listeners();

	/**
	 * Identification used to find this sensor instance in sensor_manager.
	 */
	const id_type _sensorID;

	/**
	 * Identification used to lookup sensor type data in source_params_map and receiver_params_map.
	 */
	const sensor_params::id_type _paramsID;

	/**
	 * Human readable name for this sensor instance.
	 */
	const std::string _description;

	/**
	 * Shared pointer to the the source_params for this sensor.
	 */
	source_params::reference _source;

	/**
	 * Shared pointer to the the receiver_params for this sensor.
	 */
	receiver_params::reference _receiver;

	/**
	 * Location of the sensor in world coordinates.
	 */
	wposition1 _position;

	/**
	 * Orientation of the sensor in world coordinates.
	 */
	sensor_orientation _orientation ;

	/**
	 * Mutex to that locks sensor during update_sensor.
	 */
    mutable read_write_lock _update_sensor_mutex ;

    /**
     * Last set of fathometers computed for this sensor.
     * @todo migrate to shared pointer.
     */
	proploss* _fathometers;

	/**
	 * Mutex to that locks sensor during update_fathometers.
	 */
	mutable read_write_lock _update_fathometers_mutex ;

    /**
     * Last set of eigenverbs computed for this sensor.
     * @todo migrate to shared pointer.
     */
	eigenverb_collection* _eigenverbs;

	/**
	 * Mutex to that locks sensor during update_eigenverbs.
	 */
	mutable read_write_lock _update_eigenverbs_mutex ;

    /**
     * reference to the task that is computing fathometers and eigenverbs.
     */
	thread_task::reference _wavefront_task;

	/**
	 * List containing the references of objects that will be used to
	 * update classes that require sensor data.
	 * These classes must implement sensor_changed method.
	 */
	std::list<sensor_listener*> _sensor_listeners;
};

/// @}
}// end of namespace sensors
}  // end of namespace usml
