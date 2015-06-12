/**
 * @file sensor_data.h
 * Instance of sensor request data for the simulation
 */
#pragma once

#include <list>
#include <usml/types/wposition1.h>
#include <usml/sensors/sensor_model.h>
#include <usml/sensors/orientation.h>
#include <usml/sensors/xmitRcvModeType.h>

namespace usml {
namespace sensors {

/// @ingroup sensors
/// @{

/**
 * Instance of sensor request data for the simulation
 * Used to request reverberation envelopes or fathometers
 */
 struct sensor_data {

    /**
     * Identification used to find this sensor instance in sensor_pair_manager.
     */
    sensor_model::id_type _sensorID;

    /**
     * Location of the sensor in world coordinates.
     */
    wposition1 _position;

    /**
     * Orientation of the sensor in world coordinates.
     */
    orientation _orient;

    /**
     * Enumerated type for the sensor transmit/receiver mode.
     */
    xmitRcvModeType _mode;
};

/*
 * Map of sensor_data structs used to request reverberation envelopes or fathometers
 */
typedef std::map<sensor_model::id_type, sensor_data> sensor_data_map;

/// @}
} // end of namespace sensors
} // end of namespace usml
