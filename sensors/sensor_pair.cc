/**
 *  @file sensor_pair.cc
 *  Implementation of the Class sensor_pair
 *  Created on: 10-Mar-2015 12:49:09 PM
 */

#include "sensor_pair.h"

using namespace usml::sensors;

/**
 * Notification that new fathometer data is ready.
 *
 * @param	from	Sensor that issued the notification.
 */
void sensor_pair::update_fathometers(sensor::reference& from)
{
    if (from !=  NULL) {

 // TODO
//        if (from == _source) {
//            _src_fathometers = from->fathometers();
//        }
//        if (from == _receiver) {
//            _rcv_fathometers = from->fathometers();
//        }
    }
}

/**
 * Updates the sensors eigenverb_collection
 */
void sensor_pair::update_eigenverbs(sensor::reference& from)
{
    if (from !=  NULL) {
// TODO
//        if (from == _source) {
//            _src_eigenverbs = from->eigenverbs();
//        }
//        if (from == _receiver) {
//            _rcv_eigenverbs = from->eigenverbs();
//        }
    }
}

/**
 * Sends message to remove the sensor from the sensor_pair_manager
 */
void sensor_pair::remove_sensor(sensor::reference& from)
{

}

/**
 * Get's the complement sensor's pointer
 */
sensor::reference sensor_pair::sensor_complement(sensor::reference& from)
{
    if (from.get() !=  NULL) {
        if (from ==  _source) {
            return _receiver;
        } else  {
            return _source;
        }
    } else {
        return sensor::reference();
    }
}
