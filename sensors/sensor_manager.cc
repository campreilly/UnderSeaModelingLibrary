/**
 *  @file sensor_manager.cc
 *  Implementation of the sensor_manager
 *  Created on: 12-Feb-2015 3:41:31 PM
 */

#include "sensor_manager.h"

using namespace usml::sensors ;

/**
 * Initialization of private static member _instance
 */
unique_ptr<sensor_manager> sensor_manager::_instance;

/**
 * The _mutex for the singleton sensor_manager.
 */
read_write_lock sensor_manager::_mutex;

/**
 * Singleton Constructor sensor_manager
 */
sensor_manager* sensor_manager::instance()
{
    sensor_manager* tmp = _instance.get();
    if (tmp == NULL)
    {
        write_lock_guard guard(_mutex);
        tmp = _instance.get();
        if (tmp == NULL)
        {
            tmp = new sensor_manager();
            _instance.reset(tmp);
        }
    }
    return tmp;
}

bool sensor_manager::add_sensor(const sensor::id_type sensorID, const paramsIDType paramsID,
                const xmitRcvModeType mode, const wposition1 position,
                const double pitch, const double yaw, const double roll,
                const std::string description)
{
    // Ensure sensor does not already exist
    if ( find(sensorID) != 0) {
        return false;
    }
    // Create the sensor
    sensor* sensor_ = new sensor( sensorID, paramsID, mode, position,
                                  pitch, yaw, roll, description);
    return insert(sensor_);
}


bool sensor_manager::insert(sensor* sensor_)
{
    bool result = false;
    // Insert in sensor_manager map
    result = sensor_map_template<const sensor::id_type, sensor*>::insert(sensor_->sensorID(), sensor_);

    if (result != false) {
        //Add to the sensor_pair_manager
        sensor_pair_manager::instance()->add_sensor(sensor_);
    }
    return result;
}

bool sensor_manager::erase(const sensor::id_type sensorID)
{
   // Remove from sensor_pair_manager
    if (sensor_pair_manager::instance()->remove_sensor(find(sensorID)))
    {
        // Call destructor to kill any wavefront generator processes
        delete find (sensorID);
        // erase from the map
        return sensor_map_template<const sensor::id_type, sensor*>::erase(sensorID);

    } else {
        return false;
    }
}

bool sensor_manager::remove_sensor(const sensor::id_type sensorID)
{
    // Ensure sensor already exist
    if ( find(sensorID) == 0) {
        return false;
    }

    return erase(sensorID);
}

bool sensor_manager::update_sensor(sensor* sensor_)
{
	// Input sensor does not contain "all" sensor data
	// Get current data
	sensor* current_sensor = find(sensor_->sensorID());
	
	// Ensure pre-existance
	if (current_sensor != 0) {
		
		current_sensor->update_sensor(sensor_->position(), sensor_->pitch(), sensor_->yaw());
		return true;
	}

    return false;
}

//wposition sensor_manager::get_targets(sensorIDType sensorID, xmitRcvModeType mode)
//{
//    std::list<sensorIDType> targetIDs;
//    wposition1 sensor_position = find(sensorID)->position();
//
//    // Get Targets
//    if (mode == usml::sensors::RECEIVER) {
//        // Get Sources
//        targetIDs = _sensor_pair_manager->source_list();
//    } else { // SOURCE, Treat "BOTH" also source
//        // Get Receivers
//        targetIDs = _sensor_pair_manager->receiver_list();
//    }
//
//    std::list<sensorIDType>::iterator iter;
//    sensor* target;
//    wposition1 target_position;
//    wposition targets(targetIDs.size(), 1 ,
//        sensor_position.latitude(),
//        sensor_position.longitude(),
//        sensor_position.altitude());
//    int row = -1;
//    for (iter = targetIDs.begin(); iter != targetIDs.end(); ++iter) {
//        ++row;
//        target = find(*iter);
//        target_position = target->position();
//        targets.latitude(row, 0, target_position.latitude());
//        targets.longitude(row, 0, target_position.longitude());
//        targets.altitude(row, 0, target_position.altitude());
//    }
//
//    return targets;
//}
