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
sensor_pair_manager* sensor_pair_manager::_instance = NULL;

/**
 * The _mutex for the singleton pointer.
 */
read_write_lock sensor_pair_manager::_mutex;

/**
 * Singleton Constructor - Double Check Locking Pattern DCLP
 */
sensor_pair_manager* sensor_pair_manager::instance()
{
    sensor_pair_manager* tmp = _instance;
    // TODO: insert memory barrier.
    if (tmp == NULL)
    {
        write_lock_guard guard(_mutex);
        tmp = _instance;
        if (tmp == NULL)
        {
            tmp = new sensor_pair_manager();
            // TODO: insert memory barrier
            _instance = tmp;
        }
    }
    return tmp;
}

/**
* Singleton Destructor
*/
void sensor_pair_manager::destroy()
{
    write_lock_guard guard(_mutex);
    if ( _instance != NULL )
    {
        delete _instance;
        _instance = NULL;
    }
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

/**
 * Get Fathometers
 */
proploss* sensor_pair_manager::get_fathometers(sensorIDType sourceID)
{
    return NULL;
}

/**
 *
 */
envelope_collection* sensor_pair_manager::get_envelopes(sensorIDType receiverID)
{
    return NULL;
}

/**
 *  Update Proploss for the Source provided
 */
void sensor_pair_manager::update_fathometers(sensorIDType sourceID, proploss_shared_ptr fathometers)
{

}

/**
 * Update eigenverbs for the sensor_pair's
 */
void sensor_pair_manager::update_eigenverbs(sensorIDType sensorID, xmitRcvModeType mode,
                                            eigenverbs_shared_ptr eigenverbs)
{
    // Get all sensor_pairs containing sensorID and update eigenverb collections
    sensorIDType sourceID;
    sensorIDType receiverID;
    sensor_list_iter iter;

    write_lock_guard guard(_mutex);
    switch (mode)
    {
        default:
            assert(false);
            break;
        case usml::sensors::SOURCE:
        {
            for (iter = _rcv_list.begin(); iter != _rcv_list.end(); ++iter) {
                receiverID = *iter;
                std::stringstream hash_key;
                hash_key << sensorID << "_" <<  receiverID;
                sensor_pair* the_sensor_pair = _sensor_pair_map.find(hash_key.str())->second;
                the_sensor_pair->update_eigenverbs(usml::sensors::SOURCE, eigenverbs);
            }
            break;
        }
        case usml::sensors::RECEIVER:
        {
            for (iter = _src_list.begin(); iter != _src_list.end(); ++iter) {
                sourceID = *iter;
                std::stringstream hash_key;
                hash_key << sourceID << "_" <<  sensorID;
                sensor_pair* the_sensor_pair = _sensor_pair_map.find(hash_key.str())->second;
                the_sensor_pair->update_eigenverbs(usml::sensors::RECEIVER, eigenverbs);
            }
            break;
        }
        case usml::sensors::BOTH:
        {
            for (iter = _rcv_list.begin(); iter != _rcv_list.end(); ++iter) {
                receiverID = *iter;
                std::stringstream hash_key;
                hash_key << sensorID << "_" <<  receiverID;
                sensor_pair* the_sensor_pair = _sensor_pair_map.find(hash_key.str())->second;
                the_sensor_pair->update_eigenverbs(usml::sensors::SOURCE, eigenverbs);
            }
            for (iter = _src_list.begin(); iter != _src_list.end(); ++iter) {
                sourceID = *iter;
                std::stringstream hash_key;
                hash_key << sourceID << "_" <<  sensorID;
                sensor_pair* the_sensor_pair = _sensor_pair_map.find(hash_key.str())->second;
                the_sensor_pair->update_eigenverbs(usml::sensors::RECEIVER, eigenverbs);
            }
        }
    }
}

/**
* Overloaded sensor_changed via the sensor_listener interface
*/
bool sensor_pair_manager::sensor_changed(sensorIDType sensorID, xmitRcvModeType mode)
{
    sensor_list_iter findIter; // For lookup in list(s)
    bool result = false;  // EVAR needs to return ErrorCode = 0x36000004
                          // AssetID Not Found

    write_lock_guard guard(_mutex);
    switch (mode)
    {
        default:
            assert(false);
            break;
        case usml::sensors::SOURCE:
        {
            findIter = std::find(_src_list.begin(), _src_list.end(), sensorID);
            if (findIter != _src_list.end()) {
                map_update( sensorID, usml::sensors::SOURCE);
            }
            break;
        }
        case usml::sensors::RECEIVER:
        {
            findIter = std::find(_rcv_list.begin(), _rcv_list.end(), sensorID);
            if (findIter != _src_list.end()) {
                map_update( sensorID, usml::sensors::RECEIVER);
            }
            break;
        }
        case usml::sensors::BOTH:
        {
            findIter = std::find(_src_list.begin(), _src_list.end(), sensorID);
            if (findIter != _src_list.end())
            {
                findIter = std::find(_rcv_list.begin(), _rcv_list.end(), sensorID);
                if (findIter != _rcv_list.end())
                {
                    map_update(sensorID, usml::sensors::SOURCE);
                    map_update(sensorID, usml::sensors::RECEIVER);
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
 * Adds the sensor to the sensor_pair_manager and synchronizes the map
 */
void sensor_pair_manager::add_sensor(sensorIDType sensorID, xmitRcvModeType mode)
{
    sensor_list_iter findIter;

    write_lock_guard guard(_mutex);
    switch (mode)
    {
        default:
            assert(false);
            break;
        case usml::sensors::SOURCE:
        {
             findIter = std::find(_src_list.begin(), _src_list.end(), sensorID);
             if (findIter == _src_list.end())
             {
                 _src_list.push_back(sensorID);
                 // Add to _sensor_pair_map
                 map_insert( sensorID, usml::sensors::SOURCE);
             }
            break;
        }
        case usml::sensors::RECEIVER:
        {
            findIter = std::find(_rcv_list.begin(), _rcv_list.end(), sensorID);
            if (findIter == _rcv_list.end())
            {
                _rcv_list.push_back(sensorID);
                // Add to _sensor_pair_map
                map_insert( sensorID, usml::sensors::RECEIVER);
            }
            break;
        }
        case usml::sensors::BOTH:
        {
            findIter = std::find(_src_list.begin(), _src_list.end(), sensorID);
            if (findIter == _src_list.end())
            {
                _src_list.push_back(sensorID);
                map_insert( sensorID, usml::sensors::SOURCE);
            }
            findIter = std::find(_rcv_list.begin(), _rcv_list.end(), sensorID);
            if (findIter == _rcv_list.end())
            {
                _rcv_list.push_back(sensorID);
                map_insert( sensorID, usml::sensors::RECEIVER);
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
bool sensor_pair_manager::remove_sensor(sensorIDType sensorID, xmitRcvModeType mode)
{

    bool result = false;  // EVAR needs to return ErrorCode = 0x36000004
                          // AssetID Not Found

    sensor_list_iter findIter; // For lookup in list(s)
    sensor_list_iter rcvfindIter; // For lookup in list(s)

    write_lock_guard guard(_mutex);
    switch (mode)
    {
        default:
            assert(false);
            break;
        case usml::sensors::SOURCE:
        {
            // Verify existence
            findIter = std::find(_src_list.begin(), _src_list.end(), sensorID);
            if (findIter != _src_list.end())
            {
                // Remove from _sensor_pair_map
                map_erase(sensorID, usml::sensors::SOURCE);
                _src_list.erase(findIter);
                result = true;
            }
            break;
        }
        case usml::sensors::RECEIVER:
        {
            // Verify existence
            findIter = std::find(_rcv_list.begin(), _rcv_list.end(), sensorID);
            if (findIter != _rcv_list.end())
            {
                // Remove from _sensor_pair_map
                map_erase(sensorID, usml::sensors::RECEIVER);
                // Remove from _rcv_list
                _rcv_list.erase(findIter);
                result = true;
            }
            break;
        }
        case usml::sensors::BOTH:
        {
            // Verify existence in BOTH
            findIter = std::find(_src_list.begin(), _src_list.end(), sensorID);
            if (findIter != _src_list.end()) // Found in src_list
            {
                rcvfindIter = std::find(_rcv_list.begin(), _rcv_list.end(), sensorID);
                if (rcvfindIter != _rcv_list.end()) // Found in rcv_list
                {
                    // Remove from _sensor_pair_map as sourceID
                    map_erase(sensorID, usml::sensors::SOURCE);
                    // Remove from list's
                    _src_list.erase(findIter);
                    // Remove from _sensor_pair_map as receiverID
                    map_erase(sensorID, usml::sensors::RECEIVER);
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
void sensor_pair_manager::map_insert(sensorIDType sensorID, xmitRcvModeType mode)
{
    sensor_list_iter iter;
    sensor_pair_map_iter map_iter;
    sensorIDType sourceID;
    sensorIDType receiverID;

    if (mode == usml::sensors::SOURCE)
    {
        sourceID = sensorID;
        // Add to _sensor_pair_map
        for (iter = _rcv_list.begin(); iter != _rcv_list.end(); ++iter)
        {
            receiverID = *iter;
            std::stringstream hash_key;
            hash_key << sourceID << "_" << receiverID;
            _sensor_pair_map.insert(
                std::pair<std::string, sensor_pair*>(hash_key.str(),
                    new sensor_pair(sourceID, receiverID)));
        }

    } else { // usml::sensors::RECEIVER

        receiverID = sensorID;
        // Add to _sensor_pair_map
        for (iter = _src_list.begin(); iter != _src_list.end(); ++iter)
        {
            sourceID = *iter;
            std::stringstream hash_key;
            hash_key << sourceID << "_" << receiverID;
            // Dont insert a pair that was already inserted mode BOTH
            map_iter = _sensor_pair_map.find(hash_key.str());
            if (map_iter == _sensor_pair_map.end()){
                _sensor_pair_map.insert(
                    std::pair<std::string, sensor_pair*>(hash_key.str(),
                    new sensor_pair(sourceID, receiverID)));
            }
        }
    }
}

/**
 * Erases from the sensor_pair_map
 */
void sensor_pair_manager::map_erase(sensorIDType sensorID, xmitRcvModeType mode)
{
    sensor_list_iter iter;
    sensor_pair_map_iter map_iter;

    sensorIDType sourceID;
    sensorIDType receiverID;

    if (mode == usml::sensors::SOURCE)
    {
        sourceID = sensorID;
        for (iter = _rcv_list.begin(); iter != _rcv_list.end(); ++iter)
        {
            receiverID = *iter;
            std::stringstream hash_key;
            hash_key << sourceID << "_" << receiverID;
            map_iter = _sensor_pair_map.find(hash_key.str());
            if (map_iter != _sensor_pair_map.end()) {
                sensor_pair* pair = map_iter->second;
                delete pair;
                _sensor_pair_map.erase(hash_key.str());
            }
        }

    } else { // usml::sensors::RECEIVER

        receiverID = sensorID;
        for (iter = _src_list.begin(); iter != _src_list.end(); ++iter)
        {
            sourceID = *iter;
            std::stringstream hash_key;
            hash_key << sourceID << "_" << receiverID;
            map_iter = _sensor_pair_map.find(hash_key.str());
            if (map_iter != _sensor_pair_map.end()) {
                sensor_pair* pair = map_iter->second;
                delete pair;
                _sensor_pair_map.erase(hash_key.str());
            }
        }
    }
}

/**
 * Updates the sensor_pair_map
 */
void sensor_pair_manager::map_update(sensorIDType sourceID, xmitRcvModeType mode)
{

}

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
        cout <<  "  sourceID: " << data->sourceID();
        cout <<  "  receiverID: " << data->receiverID();
        cout <<  "  proploss: " << data->proploss();
        cout <<  "  src eigenverbs: " << data->src_eigenverbs();
        cout <<  "  rcv eigenverbs: " << data->rcv_eigenverbs();
        cout <<  "  envelopes: " << data->envelopes() << endl;
    }
    cout << endl;
}
