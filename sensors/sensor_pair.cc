/**
 *  @file sensor_pair.cc
 *  Implementation of the Class sensor_pair
 *  Created on: 10-Mar-2015 12:49:09 PM
 */

#include "sensor_pair.h"

using namespace usml::sensors;

/**
 * Updates the sensors fathometers (eigenrays)
 */
void sensor_pair::update_fathometers(sensor* sensor_)
{
    if (sensor_ !=  NULL) {

 // TODO
//        if (sensor_ == _source) {
//            _src_fathometers = sensor_->fathometers();
//        }
//        if (sensor_ == _receiver) {
//            _rcv_fathometers = sensor_->fathometers();
//        }
    }
}

/**
 * Updates the sensors eigenverb_collection
 */
void sensor_pair::update_eigenverbs(sensor* sensor_)
{
    if (sensor_ !=  NULL) {

 // TODO
//        if (sensor_ == _source) {
//            _src_eigenverbs = sensor_->eigenverbs();
//        }
//        if (sensor_ == _receiver) {
//            _rcv_eigenverbs = sensor_->eigenverbs();
//        }
    }
}

/**
 * Sends message to remove the sensor from the sensor_pair_manager
 */
void sensor_pair::remove_sensor(sensor* sensor_)
{

}

/**
 * Get's the complement sensor's pointer
 */
sensor* sensor_pair::sensor_complement(sensor* sensor_)
{
    if (sensor_ !=  NULL) {
        if (sensor_ ==  _source) {
            return _receiver;
        } else  {
            return _source;
        }
    } else {
        return NULL;
    }
}
