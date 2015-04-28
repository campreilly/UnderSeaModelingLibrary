/**
 * @file sensor_pair.h
 * Container for one sensor pair instance.
 */
#pragma once

#include <usml/sensors/sensor_model.h>
#include <usml/sensors/sensor_listener.h>
#include <usml/sensors/xmitRcvModeType.h>
#include <usml/sensors/fathometer_model.h>
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
 * access to its complement sensor, and updates the eigenverbs and fathometers.
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
     * @param	frequencies	std::vector of doubles of 
     *                      intersecting frequencies for this pair.
     */
    sensor_pair(sensor_model* source, sensor_model* receiver, std::vector<double> frequencies)
        : _source(source), _receiver(receiver), _frequencies(frequencies) {};

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
     * The intersecting frequencies of both the _source and _reciever sensors.
     * @return frequencies in a std::vector of doubles.
     */
    std::vector<double> frequencies() const {
        return _frequencies;
    }

	/**
	 * Bistatic sensor_pairs are those for which the source and receiver
	 * are different.  Set to false for monostatic sensors.
	 */
	bool multistatic() const {
		return _source != _receiver ;
	}

	/**
	 * Notification that new fathometer data is ready.
	 *
	 * @param  sensorID The ID of the sensor that issued the notification.
     * @param  eigenray_list pointer std::list of eigenrays
	 */
	virtual void update_fathometer(sensor_model::id_type sensorID, eigenray_list* list) ;

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
     * Gets the shared_ptr to last fathometer update for this sensor_pair.
     * @return  fathometer_model shared_ptr
     */
     shared_ptr<fathometer_model> fathometer() {
         read_lock_guard guard(_fathometer_mutex);
         return _fathometer;
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
     * The intersecting frequencies of both the _source and _reciever sensors
     */
    const std::vector<double> _frequencies;

    /**
     * Mutex that locks sensor_pair during complement lookups.
     */
    mutable read_write_lock _complements_mutex ;

    /**
     * Fathometer that connects source and receiver locations.
     */
    shared_ptr<fathometer_model> _fathometer;

	/**
	 * Mutex that locks sensor_pair during fathometer updates.
	 */
	mutable read_write_lock _fathometer_mutex ;

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
