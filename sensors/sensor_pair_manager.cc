///////////////////////////////////////////////////////////
//  @file sensor_pair_manager.cc
//  Implementation of the Class sensor_pair_manager
//  Created on:      26-Feb-2015 5:46:35 PM
//  Original author: Ted Burns, AEgis Technologies Group, Inc.
///////////////////////////////////////////////////////////

#include <algorithm>

#include "sensor_pair_manager.h"

using namespace usml::sensors ;

/**
* Initialization of private static member _instance
*/
sensor_pair_manager* sensor_pair_manager::_instance = NULL;

/**
 * Singleton Constructor
 */
sensor_pair_manager* sensor_pair_manager::instance()
{
    if (_instance == NULL)
        _instance = new sensor_pair_manager();
    return _instance;
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

}

/**
* Overloaded sensor_changed via the sensor_listener interface
*/
bool sensor_pair_manager::sensor_changed(sensorIDType sensorID, xmitRcvModeType mode)
{
    sensor_iter findIter; // For lookup in list(s)
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
            _rcv_list.erase(findIter);
            break;
        }
        case usml::sensors::BOTH:
        {
            findIter = std::find(_src_list.begin(), _src_list.end(), sensorID);
            _src_list.erase(findIter);
            findIter = std::find(_rcv_list.begin(), _rcv_list.end(), sensorID);
            _rcv_list.erase(findIter);
        }
    }

    synch_sensor_pairs();

    return result;
}

/**
 * Adds the sensor to the sensor_pair_manager and synchronizes the maps.
 */
void sensor_pair_manager::add_sensor(sensorIDType sensorID, xmitRcvModeType mode)
{
    switch (mode)
    {
        default:
            assert(false);
            break;
        case usml::sensors::SOURCE:
        {
            _src_list.push_back(sensorID);
            break;
        }
        case usml::sensors::RECEIVER:
        {
            _rcv_list.push_back(sensorID);
            break;
        }
        case usml::sensors::BOTH:
        {
            _src_list.push_back(sensorID);
            _rcv_list.push_back(sensorID);
        }
    }

    synch_sensor_pairs();
}

/**
 * Removes a sensor from the sensor_pair_manager
 */
void sensor_pair_manager::remove_sensor(sensorIDType sensorID, xmitRcvModeType mode)
{
    sensor_iter findIter; // For lookup in list(s)
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
            if (findIter != _rcv_list.end()) {
                _rcv_list.erase(findIter);
            }
            break;
        }
        case usml::sensors::BOTH:
        {
            findIter = std::find(_src_list.begin(), _src_list.end(), sensorID);
            _src_list.erase(findIter);
            findIter = std::find(_rcv_list.begin(), _rcv_list.end(), sensorID);
            _rcv_list.erase(findIter);
        }
    }

    synch_sensor_pairs();
}

/**
 * Adds the source and receiver sensor pair to the sensor_pair_manager
 */
void sensor_pair_manager::add_sensor_pair(sensorIDType sourceID, sensorIDType receiverID)
{

    _src_list.push_back(sourceID);
    _rcv_list.push_back(receiverID);

    synch_sensor_pairs();
}

/**
 * Removes the source and receiver sensor pair from the sensor_pair_manager
 */
void sensor_pair_manager::remove_sensor_pair(sensorIDType sourceID, sensorIDType receiverID)
{
    sensor_iter findIter; // For lookup in list(s)

    findIter = std::find(_src_list.begin(), _src_list.end(), sourceID);
    if (findIter != _src_list.end()) {
        _src_list.erase(findIter);
    }
    findIter = std::find(_rcv_list.begin(), _rcv_list.end(), receiverID);
    if (findIter != _src_list.end()) {
        _rcv_list.erase(findIter);
    }

    synch_sensor_pairs();
}
