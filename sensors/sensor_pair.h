/**
 * @file sensor_pair.h
 * Container for one sensor pair instance.
 */
#pragma once

#include <usml/sensors/sensor_model.h>
#include <usml/sensors/sensor_listener.h>
#include <usml/sensors/xmitRcvModeType.h>
#include <usml/waveq3d/eigenray_collection.h>
#include <usml/eigenverb/envelope_collection.h>
#include <usml/eigenverb/eigenverb_collection.h>

namespace usml {
namespace sensors{

using namespace waveq3d ;
using namespace eigenverb ;

/// @ingroup sensors
/// @{

/**
 * Container for one sensor pair instance.
 * On construction a pointer to the source and receiver sensor are obtained.
 * Inherits the sensor_listener interface so a sensor instance can get
 * access to its complement sensor, and updates the eigenverbs and eigenrays.
 */
class USML_DECLSPEC sensor_pair : public sensor_listener
{
public:

    /**
     * Construct from references to source and receiver.
     * The source and receiver will be equal for monostatic sensors.
     *
     * @param	source		Pointer to the source for this pair.
     * @param	receiver	Pointer to the receiver for this pair.
     */
    sensor_pair(sensor_model* source, sensor_model* receiver)
        : _source(source),_receiver(receiver) {};

    /**
     * Default Destructor
     */
    virtual ~sensor_pair() {}

    /**
     * Gets a pointer to the source sensor.
     * @return  Pointer to the source sensor
     */
    const sensor_model* source() const {
        return _source;
    }

     /**
     * Gets a pointer to the receiver sensor.
     * @return  Pointer to the receiver sensor
     */
    const sensor_model* receiver() const {
        return _receiver;
    }

	/**
	 * Bistatic sensor_pairs are those for which the source and receiver
	 * are different.  Set to false for monostatic sensors.
	 */
	bool multistatic() const {
		return _source != _receiver ;
	}

	/**
	 * Notification that new eigenray data is ready.
	 *
	 * @param  sensorID The ID of the sensor that issued the notification.
     * @param  eigenray_list Shared pointer to the list of eigenrays
	 */
	virtual void update_eigenrays(sensor_model::id_type sensorID, eigenray_list* list) ;

	/**
	 * Notification that new eigenverb data is ready.
	 *
	 * @param	sensor	Pointer to sensor that issued the notification.
	 */
	virtual void update_eigenverbs(sensor_model* sensor) ;

	/**
	 * Queries for the sensor pair complements of this sensor.
	 * @param	sensor	Const sensor_model pointer Sensor that requested the complement.
     * @return  Pointer to the complement sensor of the pair.
	 */
	virtual const sensor_model* sensor_complement(const sensor_model* sensor) const ;

	/**
     * Gets the shared_ptr to last eigenray_list update for this sensor_pair.
     * @return  eigenray_list shared_ptr
     */
     boost::shared_ptr<eigenray_list> eigenrays() {
         read_lock_guard guard(_eigenrays_mutex);
         return _eigenrays;
     }

private:

    sensor_pair() {};

    /**
     * Pointer to the source sensor.
     * The source and receiver will be equal for monostatic sensors.
     */
    const sensor_model* _source;

    /**
     * Pointer to the receiver sensor.
     * The source and receiver will be equal for monostatic sensors.
     */
    const sensor_model* _receiver;

    /**
     * Mutex that locks sensor_pair during complement lookups.
     */
    mutable read_write_lock _complements_mutex ;

    /**
     * Eigenrays that connect source and receiver locations.
     */
    shared_ptr<eigenray_list> _eigenrays;

	/**
	 * Mutex that locks sensor_pair during eigenray updates.
	 */
	mutable read_write_lock _eigenrays_mutex ;

    /**
     * Interface collisions for wavefront emanating from the source.
     */
    eigenverb_collection::reference _src_eigenverbs;

	/**
	 * Mutex to that locks sensor_pair during source eigenverb updates.
	 */
	mutable read_write_lock _src_eigenverbs_mutex ;

    /**
     * Interface collisions for wavefront emanating from the receiver.
     */
    eigenverb_collection::reference _rcv_eigenverbs;

	/**
	 * Mutex to that locks sensor_pair during receiver eigenverb updates.
	 */
	mutable read_write_lock _rcv_eigenverbs_mutex ;

    /**
     * envelopes - contains the Reverb envelopes
     */
    envelope_collection::reference _envelopes;

	/**
	 * Mutex to that locks sensor_pair during _envelope updates.
	 */
	mutable read_write_lock _envelopes_mutex ;

};

/// @}
} // end of namespace sensors
} // end of namespace usml
