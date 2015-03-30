///////////////////////////////////////////////////////////
//  @file sensor_pair.h
//  Definition of the sensor_pair structure.
//  Created on:      27-Feb-2015 5:46:40 PM
//  Original author: Ted Burns, AEgis Technologies
///////////////////////////////////////////////////////////

#pragma once

#include <usml/usml_config.h>
#include <usml/sensors/sensor.h>
#include <usml/sensors/sensor_listener.h>
#include <usml/sensors/xmitRcvModeType.h>
#include <usml/waveq3d/proploss.h>
#include <usml/eigenverb/envelope_collection.h>
#include <usml/eigenverb/eigenverb_collection.h>

namespace usml {
namespace sensors{

using namespace waveq3d ;
using namespace eigenverb;

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
	 * Data type used for reference to receiver_params.
	 */
	typedef shared_ptr<sensor> reference;

    /**
     * Constructor
     */
    sensor_pair(sensor::reference source, sensor::reference receiver)
        : _source(source),_receiver(receiver) {};

    /**
     * Destructor
     */
    ~sensor_pair()
    {

    }

    /**
     * Gets a pointer to the source sensor.
     * @return  Pointer to the source sensor
     */
    sensor::reference source() const {
        return _source;
    }

     /**
     * Gets a pointer to the receiver sensor.
     * @return  Pointer to the receiver sensor
     */
    sensor::reference receiver() const {
        return _receiver;
    }

	/**
	 * Notification that new fathometer data is ready.
	 *
	 * @param	from	Sensor that issued the notification.
	 */
	virtual void update_fathometers(sensor::reference& from) ;

	/**
	 * Notification that new eigenverb data is ready.
	 *
	 * @param	from	Sensor that issued the notification.
	 */
	virtual void update_eigenverbs(sensor::reference& from) ;


	/**
	 * Notification that a sensor is about to be deleted.
	 *
	 * @param	from	Sensor that issued the notification.
	 */
	virtual void remove_sensor(sensor::reference& from) ;


	/**
	 * Queries the sensor listener for the complements of this sensor.
	 *
	 * @param	from	Sensor that issued the notification.
	 */
	virtual sensor::reference sensor_complement(sensor::reference& from) ;

private:

    sensor_pair() {};

    /**
     * Pointer to the source sensor.
     */
    sensor::reference _source;

    /**
     * Pointer to the receiver sensor.
     */
    sensor::reference _receiver;

    /**
     * src proploss - contains targets and eigenrays
     */
    proploss_shared_ptr _src_fathometers;

    /**
     * rvc proploss - contains targets and eigenrays
     */
    proploss_shared_ptr _rcv_fathometers;

    /**
     * source eigenverbs - contains all source eigenverbs
     */
    eigenverbs_shared_ptr _src_eigenverbs;

    /**
     * receiver eigenverbs - contains all receiver eigenverbs
     */
    eigenverbs_shared_ptr _rcv_eigenverbs;

    /**
     * envelopes - contains the Reverb envelopes
     */
    envelopes_shared_ptr _envelopes;

};

/// @}
} // end of namespace sensors
} // end of namespace usml
