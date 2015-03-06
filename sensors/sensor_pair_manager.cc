///////////////////////////////////////////////////////////
//  @file sensor_pair_manager.cc
//  Implementation of the Class sensor_pair_manager
//  Created on:      26-Feb-2015 5:46:35 PM
//  Original author: Ted Burns, AEgis Technologies
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
proploss* sensor_pair_manager::get_fathometers()
{
    return NULL;
}

/**
 *
 */
envelope_collection* sensor_pair_manager::get_envelopes()
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

}

/**
 * Update eigenverbs for the Source, Receiver or Both
 */
void sensor_pair_manager::update_eigenverbs(sensorIDType sensorID, xmitRcvModeType mode,
    eigenverbs_shared_ptr eigenverbs)
{
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

//if ( _map.count(key_pair) == 0 )
//{
//    return ( NULL );
//}
//return _map.find(key_pair)->second;

}

/**
* Overloaded sensor_changed via the sensor_listener interface
*/
bool sensor_pair_manager::sensor_changed(sensorIDType sensorID, xmitRcvModeType mode)
{
    return false;
}

/**
 * Adds the sensor to the sensor_pair_manager and synchronizes the maps.
 */
bool sensor_pair_manager::add_sensor(sensorIDType sensorID, xmitRcvModeType mode)
{
    sensor_iter findIter; // For lookup in list(s)
    bool result = false;  // EVAR needs to return ErrorCode = 0x31000004
                          // Illegal Input Data
    switch (mode)
    {
        default:
            assert(false);
            break;
        case usml::sensors::SOURCE:
        {
            // Check for pre-existance
            findIter = std::find(_src_list.begin(), _src_list.end(), sensorID);
            if (findIter == _src_list.end()) {
                _src_list.push_back(sensorID);
                result = true;
                synch_sensor_pairs();
            }
            break;
        }
        case usml::sensors::RECEIVER:
        {
            // Check for pre-existance
            findIter = std::find(_rcv_list.begin(), _rcv_list.end(), sensorID);
            if (findIter == _src_list.end()) {
                _rcv_list.push_back(sensorID);
                result = true;
                synch_sensor_pairs();
            }
            break;
        }
        case usml::sensors::BOTH:
        {
            // Check for pre-existance
            findIter = std::find(_src_list.begin(), _src_list.end(), sensorID);
            if (findIter == _src_list.end()) {
                findIter = std::find(_rcv_list.begin(), _rcv_list.end(), sensorID);
                if (findIter == _rcv_list.end()) {
                    _src_list.push_back(sensorID);
                    _rcv_list.push_back(sensorID);
                    result = true;
                    synch_sensor_pairs();
                }
            }
        }
    }

    return result;
}

/**
 * Removes a sensor from the sensor_pair_manager
 */
bool sensor_pair_manager::remove_sensor(sensorIDType sensorID, xmitRcvModeType mode)
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
}

/**
 * Adds the source and receiver sensor pair to the sensor_pair_manager
 */
void sensor_pair_manager::add_sensor_pair(sensorIDType sourceID, sensorIDType receiverID)
{
     cout << "add_sensor_pair: created sourceID " << sourceID << " with receiverID " << receiverID << endl;

//#ifdef USML_DEBUG
//
//    outer_map_type::iterator result = _map.find(sourceID);
//    if ( result != _map.end() )
//    {
//        cout << "Map contents: " << endl;
//        for (outer_map_type::iterator oi = _map.begin(); oi != _map.end(); ++oi)
//        {
//            inner_map_type &im = oi->second;
//            for (inner_map_type::iterator ii = im.begin(); ii != im.end(); ++ii)
//            {
//                cout << "_map[" << oi->first << "][" << ii->first << "] =" << endl;
//            }
//        }
//    }
//#endif

}

/**
 * Removes the source and receiver sensor pair from the sensor_pair_manager
 */
void sensor_pair_manager::remove_sensor_pair(sensorIDType sourceID, sensorIDType receiverID)
{

}

/**
* Finds the data_collections associated with the mode, and sensorID.
*/
void sensor_pair_manager::find(sensorIDType sensorID, usml::sensors::xmitRcvModeType mode)
{
    //std::list<sensorIDType>::iterator findIter = std::find(ilist.begin(), ilist.end(), 1);
    // now variable iter either represents valid iterator pointing to the found element
    // or it will be equal to my_list.end()

    switch (mode)
    {
        default:
            assert(false);
            break;
        case usml::sensors::SOURCE:
        {
            //data = _src_data_map.find(sensorID)->second;
            break;
        }
        case usml::sensors::RECEIVER:
        {
            //data = _rcv_data_map.find(sensorID)->second;
            break;
        }
        case usml::sensors::BOTH:
        {
        }
    }
   
    //if ( _map.count(sensorID) == 0 )
    //{
    //    return ( NULL );
    //}
    //return _map.find(sensorID)->second;

}
