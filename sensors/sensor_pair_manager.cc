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

}

/**
 *
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
 * Update Reverb Envelopes for the Receiver provided.
 */
void sensor_pair_manager::update_envelopes(sensorIDType receiverID, envelopes_shared_ptr envelopes)
{
    // TODO: set mutex write guard on receiver data map
    receiver_data data;
    data = _rcv_data_map.find(receiverID)->second;
    data.envelopes = envelopes;
}

/**
 * Update eigenverbs for the Source, Receiver or Both
 */
void sensor_pair_manager::update_eigenverbs(sensorIDType sensorID, xmitRcvModeType mode,
    eigenverbs_shared_ptr eigenverbs)
{
     // TODO: set mutex write guard(s) on source or receivers data map
    switch (mode)
    {
        default:
            assert(false);
            break;
        case usml::sensors::SOURCE:
        {
            source_data data;
            data = _src_data_map.find(sensorID)->second;
            data.eigenverbs = eigenverbs;
            break;
        }
        case usml::sensors::RECEIVER:
        {
            receiver_data data;
            data = _rcv_data_map.find(sensorID)->second;
            data.eigenverbs = eigenverbs;
            break;
        }
        case usml::sensors::BOTH:
        {
            source_data data;
            data = _src_data_map.find(sensorID)->second;
            data.eigenverbs = eigenverbs;
            receiver_data rcv_data;
            rcv_data = _rcv_data_map.find(sensorID)->second;
            rcv_data.eigenverbs = eigenverbs;
        }
    }
}

/**
 * Synchronizes the source and receiver maps to
 * the active source and receiver lists.
 */
void sensor_pair_manager::synch_sensor_pairs()
{
    // TODO: set mutex write guard(s) on source or receivers data map

    sensor_list_iter iter;
    sensorIDType sourceID;
    sensorIDType receiverID;

    // Update _src_rcv_pair_map
    for (iter = _src_list.begin(); iter != _src_list.end(); ++iter) {
        sourceID = *iter;
         _src_rcv_pair_map[sourceID] = _rcv_list;
    }

    // Update _rcv_src_pair_map
    for (iter = _rcv_list.begin(); iter != _rcv_list.end(); ++iter) {
        receiverID = *iter;
        _rcv_src_pair_map[receiverID] = _src_list;
    }
}

/**
 * Initializes the source and receiver data maps based
 * on the active source and receiver lists.
 */
void sensor_pair_manager::init_sensor_data()
{
    sensor_list_iter iter;
    sensorIDType sourceID;
    sensorIDType receiverID;

    // Update _src_data_map
    for (iter = _src_list.begin(); iter != _src_list.end(); ++iter) {
        sourceID = *iter;
        _src_data_map[sourceID];
    }

    // Update _rcv_data_map
    for (iter = _rcv_list.begin(); iter != _rcv_list.end(); ++iter) {
        receiverID = *iter;
        _rcv_data_map[receiverID];
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
    switch (mode)
    {
        default:
            assert(false);
            break;
        case usml::sensors::SOURCE:
        {
            findIter = std::find(_src_list.begin(), _src_list.end(), sensorID);
            if (findIter != _src_list.end()) {
                _src_list.erase(findIter);
            }
            break;
        }
        case usml::sensors::RECEIVER:
        {
            findIter = std::find(_rcv_list.begin(), _rcv_list.end(), sensorID);
            if (findIter != _src_list.end()) {
                _rcv_list.erase(findIter);
            }
            break;
        }
        case usml::sensors::BOTH:
        {
            findIter = std::find(_src_list.begin(), _src_list.end(), sensorID);
            if (findIter != _src_list.end()) {
                _src_list.erase(findIter);
            }
            findIter = std::find(_rcv_list.begin(), _rcv_list.end(), sensorID);
            if (findIter != _src_list.end()) {
                _rcv_list.erase(findIter);
            }
        }
    }

    if (result) {
        synch_sensor_pairs();

        init_sensor_data();

        #ifdef USML_DEBUG
            print_sensor_pairs();
        #endif
    }
    return result;
}

/**
 * Adds the sensor to the sensor_pair_manager and synchronizes the maps.
 */
void sensor_pair_manager::add_sensor(sensorIDType sensorID, xmitRcvModeType mode)
{
    sensor_list_iter findIter; // For lookup in list(s)
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
             }
            break;
        }
        case usml::sensors::RECEIVER:
        {
            findIter = std::find(_rcv_list.begin(), _rcv_list.end(), sensorID);
            if (findIter == _rcv_list.end())
            {
                _rcv_list.push_back(sensorID);
            }
            break;
        }
        case usml::sensors::BOTH:
        {
            findIter = std::find(_src_list.begin(), _src_list.end(), sensorID);
            if (findIter == _src_list.end())
            {
                _src_list.push_back(sensorID);
            }
            findIter = std::find(_rcv_list.begin(), _rcv_list.end(), sensorID);
            if (findIter == _rcv_list.end())
            {
                _rcv_list.push_back(sensorID);
            }
        }
    }

    synch_sensor_pairs();

    init_sensor_data();

#ifdef USML_DEBUG
    print_sensor_pairs();
#endif

}

/**
 * Removes a sensor from the sensor_pair_manager
 */
bool sensor_pair_manager::remove_sensor(sensorIDType sensorID)
{
    sensor_list_iter findIter; // For lookup in list(s)
    bool result = false;  // EVAR needs to return ErrorCode = 0x36000004
                          // AssetID Not Found

    findIter = std::find(_src_list.begin(), _src_list.end(), sensorID);
    if (findIter != _src_list.end())
    {
        sensorIDType sensorID = *findIter;
        _src_rcv_pair_map.erase(sensorID);
        _src_data_map.erase(sensorID);
        _src_list.erase(findIter);

        result = true;
    }
    findIter = std::find(_rcv_list.begin(), _rcv_list.end(), sensorID);
    if (findIter != _rcv_list.end())
    {
        sensorIDType sensorID = *findIter;
        _rcv_src_pair_map.erase(sensorID);
        _rcv_data_map.erase(sensorID);
        _rcv_list.erase(findIter);
        result = true;
    }

    if (result)
    {
        synch_sensor_pairs();
        #ifdef USML_DEBUG
            print_sensor_pairs();
        #endif
    }
    return result;
}

/**
 * Prints to console the all source and receiver maps
 */
void sensor_pair_manager::print_sensor_pairs()
{
    cout << "******* print_sensor_pairs ******************" << endl;

    sensor_list_iter iter;
    sensorIDType sourceID;
    sensorIDType receiverID;

    sensor_map_iter map_iter;
    for (map_iter = _src_rcv_pair_map.begin(); map_iter != _src_rcv_pair_map.end(); ++map_iter)
    {
        cout << "_src_rcv_pair_map[" << map_iter->first << "] = " ;

        std::list<sensorIDType> rcv_list = map_iter->second;
        for (iter = rcv_list.begin(); iter != rcv_list.end(); ++iter) {
            receiverID = *iter;
            cout <<  receiverID << " " ;
        }
        cout << endl;
    }
    cout << endl;


    for (map_iter = _rcv_src_pair_map.begin(); map_iter != _rcv_src_pair_map.end(); ++map_iter)
    {
        cout << "_rcv_src_pair_map[" << map_iter->first << "] = " ;

        std::list<sensorIDType> src_list = map_iter->second;
        for (iter = src_list.begin(); iter != src_list.end(); ++iter) {
            sourceID = *iter;
            cout <<  sourceID << " " ;
        }
        cout << endl;
    }
    cout << endl;

    source_data_iter data_iter;
    for (data_iter = _src_data_map.begin(); data_iter != _src_data_map.end(); ++data_iter)
    {
        cout << "_src_data_map[" << data_iter->first << "] = ";
        source_data data = data_iter->second;
        cout <<  "proploss: " << data.proploss;
        cout <<  " eigenverbs: " << data.eigenverbs << endl;
    }
    cout << endl;

    receiver_data_iter rdata_iter;
    for (rdata_iter = _rcv_data_map.begin(); rdata_iter != _rcv_data_map.end(); ++rdata_iter)
    {
        cout << "_rcv_data_map[" << rdata_iter->first << "] = ";
        receiver_data data = rdata_iter->second;
        cout <<  "eigenverbs: " << data.eigenverbs;
        cout <<  " envelopes: " << data.envelopes << endl;
    }
    cout << endl;
}
