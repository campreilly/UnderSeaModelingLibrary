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
read_write_lock sensor_manager::_instance_mutex;

/**
 * Singleton Constructor sensor_manager
 */
sensor_manager* sensor_manager::instance()
{
    sensor_manager* tmp = _instance.get();
    if (tmp == NULL)
    {
        write_lock_guard guard(_instance_mutex);
        tmp = _instance.get();
        if (tmp == NULL)
        {
            tmp = new sensor_manager();
            _instance.reset(tmp);
        }
    }
    return tmp;
}

bool sensor_manager::insert(const sensor::id_type sensorID, sensor* sensor_)
{
    // Insert in the map
    return sensor_map_template<const sensor::id_type, sensor*>::insert(sensorID, sensor_);
}

bool sensor_manager::erase(const sensor::id_type sensorID)
{
    // erase from the map
    return sensor_map_template<const sensor::id_type, sensor*>::erase(sensorID);
}

bool sensor_manager::update(const sensor::id_type sensorID,
		const wposition1& position, const sensor_orientation& orientation,
		bool force_update)
{
	sensor* current_sensor = find(sensorID);
	if (current_sensor != 0) {
		current_sensor->update_sensor(position,orientation,force_update);
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
