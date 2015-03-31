/**
 * @file sensor.h
 * Instance of an active sensor in the simulation.
 */
#pragma once

#include <usml/eigenverb/eigenverb_collection.h>
#include <usml/eigenverb/wavefront_listener.h>
#include <usml/sensors/receiver_params.h>
#include <usml/sensors/sensor_listener.h>
#include <usml/sensors/sensor_orientation.h>
#include <usml/sensors/source_params.h>
#include <usml/sensors/xmitRcvModeType.h>
#include <usml/threads/read_write_lock.h>
#include <usml/threads/thread_task.h>
#include <usml/waveq3d/proploss.h>

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
	 * Data type used for sensorID.
	 */
	typedef int id_type;

	/**
	 * Data type used for reference to sensor.
	 */
	typedef shared_ptr<sensor> reference;

	/**
	 * Construct a new instance of a specific sensor type.
	 * Sets the position and orientation values to NAN.
	 * These values are not set until the update_sensor()
	 * is invoked for the first time.
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
	 * Removes a sensor instance from simulation.
	 * Automatically aborts wavefront task if one exists.
	 */
	virtual ~sensor() ;

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
	xmitRcvModeType mode() const ;

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
	wposition1 position() const ;

	/**
	 * Orientation of the sensor in world coordinates.
	 */
	sensor_orientation orientation() const ;

	/**
	 * Updates the position and orientation of sensor.
	 * If the object has changed by more than the threshold amount,
	 * this update kicks off a new set of propagation calculations.
	 * At the end of those calculations, the fathometers and eigen
     * Passes this data onto all sensor listeners.
	 * Blocks until update is complete.
	 *
	 * @param position  	Updated position data
	 * @param orientation	Updated orientation value
	 * @param force_update	When true, forces update without checking thresholds.
	 */
	void update_sensor(const wposition1& position,
			const sensor_orientation& orientation, bool force_update = false);

    /**
     * Last set of fathometers computed for this sensor.
	 * Blocks during updates from the wavefront task.
     * @todo migrate to shared pointer.
     */
	proploss_shared_ptr fathometers() const ;

	/**
	 * Asynchronous update of fathometer data from the wavefront task.
     * Passes this data onto all sensor listeners.
	 * Blocks until update is complete.
	 */
	virtual void update_fathometers( proploss_shared_ptr& fathometers ) ;

    /**
     * Last set of eigenverbs computed for this sensor.
	 * Blocks during updates from the wavefront task.
     * @todo migrate to shared pointer.
     */
	eigenverbs_shared_ptr eigenverbs() const ;

	/**
	 * Asynchronous update of eigenverbs data from the wavefront task.
	 * Passes this data onto all sensor listeners.
	 * Blocks until update is complete.
	 */
	virtual void update_eigenverbs( eigenverbs_shared_ptr& eigenverbs) ;

	/**
	 * Add a sensor_listener to the _sensor_listeners list
	 */
	void add_sensor_listener(sensor_listener::reference listener);

	/**
	 * Remove a sensor_listener from the _sensor_listeners list
	 */
	void remove_sensor_listener(sensor_listener::reference listener);

private:

	/**
	 * Checks to see if new position and orientation have changed enough
	 * to require a new WaveQ3D run.
	 *
	 * @param position  	Updated position data
	 * @param orientation	Updated orientation value
	 * @return 				True when thresholds exceeded, requiring a
	 * 						rerun of the model for this sensor.
	 */
	bool check_thresholds( const wposition1& position,
			const sensor_orientation& orientation );

	/**
	 * Queries the current list of sensor listeners for the complements
	 * of this sensor. Assumes that these listeners act like
	 * sensor_pair objects.
	 */
	wposition sensor_targets() ;

	/**
	 * Run the wave_generator thread task to start the waveq3d model.
	 */
	void run_wave_generator();

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
     * For each sensor_listener in the _sensor_listeners list call the
     * update_eigenverbs method of each registered class, ie sensor_pair
     * Last set of fathometers computed for this sensor.
     * @todo migrate to shared pointer.
     */
    proploss_shared_ptr _fathometers;

	/**
	 * Mutex to that locks sensor during update_fathometers.
	 */
	mutable read_write_lock _update_fathometers_mutex ;

    /**
     * Last set of eigenverbs computed for this sensor.
     * @todo migrate to shared pointer.
     */
	shared_ptr<eigenverb_collection> _eigenverbs;

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
	std::list<sensor_listener::reference> _sensor_listeners;

	/**
	 * Mutex to that locks sensor during add/remove sensor_listeners.
	 */
	mutable read_write_lock _sensor_listeners_mutex ;
};

/// @}
}// end of namespace sensors
}  // end of namespace usml
