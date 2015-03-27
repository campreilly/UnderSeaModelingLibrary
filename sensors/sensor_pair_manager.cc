///////////////////////////////////////////////////////////
//  @file sensor_pair_manager.cc
//  Implementation of the Class sensor_pair_manager
//  Created on:      26-Feb-2015 5:46:35 PM
//  Original author: Ted Burns, AEgis Technologies Group, Inc.
///////////////////////////////////////////////////////////

#include <algorithm>
#include <utility>

#include "sensor_pair_manager.h"

using namespace usml::sensors ;

/**
* Initialization of private static member _instance
*/
unique_ptr<sensor_pair_manager> sensor_pair_manager::_instance;


/**
 * The _mutex for the singleton pointer.
 */
read_write_lock sensor_pair_manager::_mutex;

/**
 * Singleton Constructor - Double Check Locking Pattern DCLP
 */
sensor_pair_manager* sensor_pair_manager::instance()
{
    sensor_pair_manager* tmp = _instance.get();
    if (tmp == NULL)
    {
        write_lock_guard guard(_mutex);
        tmp = _instance.get();
        if (tmp == NULL)
        {
            tmp = new sensor_pair_manager();
            _instance = tmp;
        }
    }
    return tmp;
}

/**
 * Destructor
 */
sensor_pair_manager::~sensor_pair_manager()
{
    // Clean up _sensor_pair_map
    sensor_pair_map_iter map_iter;

    for (map_iter = _sensor_pair_map.begin(); map_iter != _sensor_pair_map.end(); ++map_iter)
    {
        sensor_pair* pair = map_iter->second;
        delete pair;
    }
    _sensor_pair_map.clear();
}

//wposition sensor_pair_manager::get_targets(xmitRcvModeType mode)
//{
//    std::list<sensor::id_type> targetIDs;
//
//    // Get Targets
//    if (mode == usml::sensors::RECEIVER) {
//        // Get Sources
//        //targetIDs = _src_map;
//    } else { // SOURCE, Treat "BOTH" also source
//        // Get Receivers
//        //targetIDs = _rcv_map;
//    }
//
//    std::list<sensor::id_type>::iterator iter;
//    sensor* target;
//    wposition1 target_position;
//    wposition targets(targetIDs.size(), 1 , 0.0, 0.0, 0.0);
//
//    int row = -1;
//    for (iter = targetIDs.begin(); iter != targetIDs.end(); ++iter) {
//        ++row;
//        //target = sensor_map::instance()->find(*iter);
//        target_position = target->position();
//        targets.latitude(row, 0, target_position.latitude());
//        targets.longitude(row, 0, target_position.longitude());
//        targets.altitude(row, 0, target_position.altitude());
//    }
//
//    return targets;
//
//}

/**
 * Get Fathometers
 */
proploss* sensor_pair_manager::get_fathometers(sensor::id_type sourceID)
{
    return NULL;
}

/**
 * Get Envelopes
 */
envelope_collection* sensor_pair_manager::get_envelopes(sensor::id_type receiverID)
{
    return NULL;
}

/**
 * Adds the sensor to the sensor_pair_manager and synchronizes the map
 */
void sensor_pair_manager::add_sensor(sensor* sensor_)
{
    sensor_list_iter findIter;

    write_lock_guard guard(_mutex);
    switch (sensor_->mode())
    {
        default:
            assert(false);
            break;
        case usml::sensors::SOURCE:
        {
             findIter = std::find(_src_list.begin(), _src_list.end(), sensor_->sensorID());
             if (findIter == _src_list.end())
             {
                 _src_list.push_back(sensor_->sensorID());
                 // Add to _sensor_pair_map
                 map_insert(sensor_);
             }
            break;
        }
        case usml::sensors::RECEIVER:
        {
            findIter = std::find(_rcv_list.begin(), _rcv_list.end(), sensor_->sensorID());
            if (findIter == _rcv_list.end())
            {
                _rcv_list.push_back(sensor_->sensorID());
                // Add to _sensor_pair_map
                map_insert( sensor_);
            }
            break;
        }
        case usml::sensors::BOTH:
        {
            findIter = std::find(_src_list.begin(), _src_list.end(), sensor_->sensorID());
            if (findIter == _src_list.end())
            {
                _src_list.push_back(sensor_->sensorID());
                map_insert( sensor_);
            }
            findIter = std::find(_rcv_list.begin(), _rcv_list.end(), sensor_->sensorID());
            if (findIter == _rcv_list.end())
            {
                _rcv_list.push_back(sensor_->sensorID());
                map_insert( sensor_);
            }
        }
    }

#ifdef USML_DEBUG
    //print_sensor_pairs();
#endif

}

/**
 * Removes a sensor from the sensor_pair_manager
 */
bool sensor_pair_manager::remove_sensor(sensor* sensor_)
{

    bool result = false;  // EVAR needs to return ErrorCode = 0x36000004
                          // AssetID Not Found

    sensor_list_iter findIter; // For lookup in list(s)
    sensor_list_iter rcvfindIter; // For lookup in list(s)

    write_lock_guard guard(_mutex);
    switch (sensor_->mode())
    {
        default:
            assert(false);
            break;
        case usml::sensors::SOURCE:
        {
            // Verify existence
            findIter = std::find(_src_list.begin(), _src_list.end(), sensor_->sensorID());
            if (findIter != _src_list.end())
            {
                // Remove from _sensor_pair_map
                map_erase(sensor_);
                _src_list.erase(findIter);
                result = true;
            }
            break;
        }
        case usml::sensors::RECEIVER:
        {
            // Verify existence
            findIter = std::find(_rcv_list.begin(), _rcv_list.end(), sensor_->sensorID());
            if (findIter != _rcv_list.end())
            {
                // Remove from _sensor_pair_map
                map_erase(sensor_);
                // Remove from _rcv_list
                _rcv_list.erase(findIter);
                result = true;
            }
            break;
        }
        case usml::sensors::BOTH:
        {
            // Verify existence in BOTH
            findIter = std::find(_src_list.begin(), _src_list.end(), sensor_->sensorID());
            if (findIter != _src_list.end()) // Found in src_list
            {
                rcvfindIter = std::find(_rcv_list.begin(), _rcv_list.end(), sensor_->sensorID());
                if (rcvfindIter != _rcv_list.end()) // Found in rcv_list
                {
                    // Remove from _sensor_pair_map as sourceID
                    map_erase(sensor_);
                    // Remove from list's
                    _src_list.erase(findIter);
                     // Remove from list's
                    _rcv_list.erase(rcvfindIter);
                    result = true;
                }
            }
        }
    }

#ifdef USML_DEBUG
        //print_sensor_pairs();
#endif

    return result;

}

/**
 * Inserts the sensor into the _sensor_pair_map
 */
void sensor_pair_manager::map_insert(sensor* sensor_)
{
    sensor_list_iter iter;
    sensor_pair_map_iter map_iter;
    sensor::id_type sourceID;
    sensor::id_type receiverID;

    if (sensor_->mode() == usml::sensors::SOURCE)
    {
        sourceID = sensor_->sensorID();
        // Add to _sensor_pair_map
        for (iter = _rcv_list.begin(); iter != _rcv_list.end(); ++iter)
        {
            receiverID = *iter;
            sensor* receiver_ = sensor_manager::instance()->find(receiverID);
            std::stringstream hash_key;
            hash_key << sourceID << "_" << receiverID;
            sensor_pair* sensor_pair_ = new sensor_pair(sensor_, receiver_);
            _sensor_pair_map.insert(
                std::pair<std::string, sensor_pair*>(hash_key.str(), sensor_pair_));
            sensor_->add_sensor_listener(sensor_pair_);
        }

    } else { // usml::sensors::RECEIVER

        receiverID = sensor_->sensorID();
        // Add to _sensor_pair_map
        for (iter = _src_list.begin(); iter != _src_list.end(); ++iter)
        {
            sourceID = *iter;
            sensor* source_ = sensor_manager::instance()->find(sourceID);
            std::stringstream hash_key;
            hash_key << sourceID << "_" << receiverID;
            // Dont insert a pair that was already inserted mode BOTH
            map_iter = _sensor_pair_map.find(hash_key.str());
            if (map_iter == _sensor_pair_map.end()){
                sensor_pair* sensor_pair_ = new sensor_pair(source_, sensor_);
                _sensor_pair_map.insert(
                    std::pair<std::string, sensor_pair*>(hash_key.str(), sensor_pair_));
                sensor_->add_sensor_listener(sensor_pair_);
            }
        }
    }
}

/**
 * Erases from the sensor_pair_map
 */
void sensor_pair_manager::map_erase(sensor* sensor_)
{
    sensor_list_iter iter;
    sensor_pair_map_iter map_iter;

    sensor::id_type sourceID;
    sensor::id_type receiverID;

    if (sensor_->mode() == usml::sensors::SOURCE)
    {
        sourceID = sensor_->sensorID();
        for (iter = _rcv_list.begin(); iter != _rcv_list.end(); ++iter)
        {
            receiverID = *iter;
            std::stringstream hash_key;
            hash_key << sourceID << "_" << receiverID;
            map_iter = _sensor_pair_map.find(hash_key.str());
            if (map_iter != _sensor_pair_map.end()) {
                sensor_pair* pair = map_iter->second;
                sensor_->remove_sensor_listener(pair);
                delete pair;
                _sensor_pair_map.erase(hash_key.str());
            }
        }

    } else { // usml::sensors::RECEIVER

        receiverID = sensor_->sensorID();
        for (iter = _src_list.begin(); iter != _src_list.end(); ++iter)
        {
            sourceID = *iter;
            std::stringstream hash_key;
            hash_key << sourceID << "_" << receiverID;
            map_iter = _sensor_pair_map.find(hash_key.str());
            if (map_iter != _sensor_pair_map.end()) {
                sensor_pair* pair = map_iter->second;
                sensor_->remove_sensor_listener(pair);
                delete pair;
                _sensor_pair_map.erase(hash_key.str());
            }
        }
    }
}

///**
//* Overloaded sensor_changed via the sensor_listener interface
//*/
//bool sensor_pair_manager::sensor_changed(sensor::id_type sensorID, xmitRcvModeType mode)
//{
//    sensor_map_iter findIter; // For lookup in list(s)
//    bool result = false;  // EVAR needs to return ErrorCode = 0x36000004
//                          // AssetID Not Found
//
//    write_lock_guard guard(_mutex);
//    switch (mode)
//    {
//        default:
//            assert(false);
//            break;
//        case usml::sensors::SOURCE:
//        {
//            findIter = std::find(_src_map.begin(), _src_map.end(), sensorID);
//            if (findIter != _src_map.end()) {
//                map_update( sensorID, usml::sensors::SOURCE);
//            }
//            break;
//        }
//        case usml::sensors::RECEIVER:
//        {
//            findIter = std::find(_rcv_map.begin(), _rcv_map.end(), sensorID);
//            if (findIter != _src_map.end()) {
//                map_update( sensorID, usml::sensors::RECEIVER);
//            }
//            break;
//        }
//        case usml::sensors::BOTH:
//        {
//            findIter = std::find(_src_map.begin(), _src_map.end(), sensorID);
//            if (findIter != _src_map.end())
//            {
//                findIter = std::find(_rcv_map.begin(), _rcv_map.end(), sensorID);
//                if (findIter != _rcv_map.end())
//                {
//                    map_update(sensorID, usml::sensors::SOURCE);
//                    map_update(sensorID, usml::sensors::RECEIVER);
//                }
//            }
//        }
//    }
//
//#ifdef USML_DEBUG
//    //print_sensor_pairs();
//#endif
//
//    return result;
//}

///**
// * Updates the sensor_pair_map
// */
//void sensor_pair_manager::map_update(sensor* sensor_)
//{
//
//}

/**
 * Prints to console the all source and receiver maps
 */
void sensor_pair_manager::print_sensor_pairs()
{
    cout << "******* print_sensor_pairs ******************" << endl;

    sensor_pair_map_iter iter;
    for (iter = _sensor_pair_map.begin(); iter != _sensor_pair_map.end(); ++iter)
    {
        cout << "_sensor_pair_map[" << iter->first << "] = ";
        sensor_pair* data = iter->second;
        cout <<  "  sourceID: " << data->source()->sensorID();
        cout <<  "  receiverID: " << data->receiver()->sensorID();
    }
    cout << endl;
}
