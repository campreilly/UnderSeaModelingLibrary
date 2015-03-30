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
void sensor_pair::update_fathometers(sensor* the_sensor)
{

}

/**
 * Updates the sensors eigenverb_collection
 */
void sensor_pair::update_eigenverbs(sensor* the_sensor)
{
///**
// * Update eigenverbs for the sensor_pair's
// */
//void sensor_pair_manager::update_eigenverbs(sensor::id_type sensorID, xmitRcvModeType mode,
//                                            eigenverbs_shared_ptr eigenverbs)
//{
//    // Get all sensor_pairs containing sensorID and update eigenverb collections
//    sensor::id_type sourceID;
//    sensor::id_type receiverID;
//    sensor_manager_iter iter;
//
//    write_lock_guard guard(_mutex);
//    switch (mode)
//    {
//        default:
//            assert(false);
//            break;
//        case usml::sensors::SOURCE:
//        {
//            for (iter = _rcv_map.begin(); iter != _rcv_map.end(); ++iter) {
//                receiverID = *iter->first;
//                std::stringstream hash_key;
//                hash_key << sensorID << "_" <<  receiverID;
//                sensor_pair* the_sensor_pair = _sensor_pair_map.find(hash_key.str())->second;
//                the_sensor_pair->update_eigenverbs(usml::sensors::SOURCE, eigenverbs);
//            }
//            break;
//        }
//        case usml::sensors::RECEIVER:
//        {
//            for (iter = _src_map.begin(); iter != _src_map.end(); ++iter) {
//                sourceID = *iter->first;
//                std::stringstream hash_key;
//                hash_key << sourceID << "_" <<  sensorID;
//                sensor_pair* the_sensor_pair = _sensor_pair_map.find(hash_key.str())->second;
//                the_sensor_pair->update_eigenverbs(usml::sensors::RECEIVER, eigenverbs);
//            }
//            break;
//        }
//        case usml::sensors::BOTH:
//        {
//            for (iter = _rcv_map.begin(); iter != _rcv_map.end(); ++iter) {
//                receiverID = iter->first;
//                std::stringstream hash_key;
//                hash_key << sensorID << "_" <<  receiverID;
//                sensor_pair* the_sensor_pair = _sensor_pair_map.find(hash_key.str())->second;
//                the_sensor_pair->update_eigenverbs(usml::sensors::SOURCE, eigenverbs);
//            }
//            for (iter = _src_map.begin(); iter != _src_map.end(); ++iter) {
//                sourceID = *iter->first;
//                std::stringstream hash_key;
//                hash_key << sourceID << "_" <<  sensorID;
//                sensor_pair* the_sensor_pair = _sensor_pair_map.find(hash_key.str())->second;
//                the_sensor_pair->update_eigenverbs(usml::sensors::RECEIVER, eigenverbs);
//            }
//        }
//    }
//}

}

/**
 * Sends message to remove the sensor from the sensor_pair_manager
 */
void sensor_pair::remove_sensor(sensor* the_sensor)
{

}

/**
 * Get's the complement sensor's pointer
 */
sensor* sensor_pair::sensor_complement(sensor* the_sensor)
{
    return NULL;
}
