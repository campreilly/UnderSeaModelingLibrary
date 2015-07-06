/**
 * @file fathometer_collection.h
 * Container for one fathometer_collection instance.
 */
#pragma once

#include <usml/sensors/sensor_model.h>
#include <usml/waveq3d/eigenray.h>

namespace usml {
namespace sensors{

using namespace waveq3d ;
using namespace eigenverb;

/// @ingroup sensors
/// @{

/**
 * Container for one fathometer_collection instance.
 * On construction takes in all source and receiver data and eigenrays
 */
class USML_DECLSPEC fathometer_collection
{
public:

    /**
     * Data type used handle a group of fathometer_collection pointers.
     */
    typedef std::vector<fathometer_collection*> fathometer_package;

    /**
     * Data type used for reference to a fathometer_collection.
     */
    typedef shared_ptr<fathometer_collection> reference;

    /**
     * Construct from all data required.
     *
     * @param    source_id      The source id for this pair.
     * @param    receiver_id    The receiver id for this pair.
     * @param    src_pos        The source position when eigenrays were obtained.
     * @param    rcv_pos        The receiver position when eigenrays were obtained.
     * @param    list           The list of eigenrays.
     */
    fathometer_collection(sensor_model::id_type source_id, sensor_model::id_type receiver_id,
                     wposition1 src_pos, wposition1 rcv_pos,  const eigenray_list& list )
        :  _source_id(source_id), _receiver_id(receiver_id),
           _source_position(src_pos), _receiver_position(rcv_pos), _eigenrays(list)
    {
        _slant_range = _receiver_position.distance(_source_position);
        // Get first eigenray arrival time
        std::list<eigenray>::const_iterator ray_iter = _eigenrays.begin();
        _initial_time = ray_iter->time;
    }

    /**
     * Destructor
     */
    ~fathometer_collection()
    {
    }

    /**
     * Gets the initial_time value.
     * @return  The initial_time.
     */
    double initial_time() {
        return _initial_time;
    }

    /**
     * Sets the initial_time value.
     * @param  The initial_time.
     */
    void initial_time(double initial_time) {
        _initial_time = initial_time;
    }

    /**
     * Gets the slant_range.
     * @return  The slant_range.
     */
    double slant_range() {
        return _slant_range;
    }

    /**
     * Gets the source sensor id.
     * @return  The source sensor id.
     */
    sensor_model::id_type source_id() {
        return _source_id;
    }

    /**
     * Gets the receiver sensor id.
     * @return  The receiver sensor id.
     */
    sensor_model::id_type receiver_id() {
        return _receiver_id;
    }

    /**
     * Gets the source position.
     * @return  The source position
     */
    wposition1 source_position() {
        return _source_position;
    }

    /**
     * Sets the source position
     * @param  position The source position
     */
    void source_position(wposition1 position) {
        _source_position = position;
    }

    /**
     * Gets the receiver position.
     * @return  The receiver position.
     */
    wposition1 receiver_position() {
        return _receiver_position;
    }

    /**
     * Sets the receiver position.
     * @param  position The receiver position.
     */
    void receiver_position(wposition1 position) {
        _receiver_position = position;
    }

    /**
     * Gets the eigenray_list for this fathometer_collection.
     * @return  eigenray_list
     */
    eigenray_list eigenrays() {
         read_lock_guard guard(_eigenrays_mutex);
         return _eigenrays;
    }

    /**
     * Updates the fathometer data with the parameters provided.
     *
     * @param delta_time    The time amount to shift the eigenrays
     * @param slant_range   The range in meters from the source and receiver.
     * @param prev_range    The previous range in meters from the source and
     *                      receiver at the start of delta_time.
     */
    void dead_reckon(double delta_time, double slant_range, double prev_range);

    /**
     * Write fathometer_collection data to a netCDF file using a ragged
     * array structure. This ragged array concept (see reference) stores
     * the fathometer_collection data in a one dimensional list.
     *
     * This ragged array concept is used to define the intensity, phase,
     * source_de, source_az, target_de, target_az, surface, bottom, and
     * caustic variables.
     *
     * This file structure is illustrated (for a single target with
     * direct path, surface, and bottom eigenrays) in the netCDF sample below:
     * <pre>
     *     netcdf fathometers_0 {
     *  dimensions:
     *      frequency = 4 ;
     *      eigenrays = 3 ;
     *  variables:
     *      double frequency(frequency) ;
     *              frequency:units = "Hertz" ;
     *      short source_id ;
     *      short receiver_id ;
     *      double slant_range ;
     *      double distance_from_sensor ;
     *      double depth_offset ;
     *      double source_latitude ;
     *             source_latitude:units = "degrees_north" ;
     *      double source_longitude ;
     *             source_longitude:units = "degrees_east" ;
     *      double source_altitude ;
     *             source_altitude:units = "meters" ;
     *             source_altitude:positive = "up" ;
     *      double receiver_latitude ;
     *             receiver_latitude:units = "degrees_north" ;
     *      double receiver_longitude ;
     *             receiver_longitude:units = "degrees_east" ;
     *      double receiver_altitude ;
     *             receiver_altitude:units = "meters" ;
     *             receiver_altitude:positive = "up" ;
     *      double intensity(eigenrays, frequency) ;
     *             intensity:units = "dB" ;
     *      double phase(eigenrays, frequency) ;
     *             phase:units = "radians" ;
     *      double travel_time(eigenrays) ;
     *             travel_time:units = "seconds" ;
     *      double source_de(eigenrays) ;
     *             source_de:units = "degrees" ;
     *             source_de:positive = "up" ;
     *      double source_az(eigenrays) ;
     *             source_az:units = "degrees_true" ;
     *             source_az:positive = "clockwise" ;
     *      double target_de(eigenrays) ;
     *             target_de:units = "degrees" ;
     *             target_de:positive = "up" ;
     *      double target_az(eigenrays) ;
     *             target_az:units = "degrees_true" ;
     *             target_az:positive = "clockwise" ;
     *      short  surface(eigenrays) ;
     *             surface:units = "count" ;
     *      short  bottom(eigenrays) ;
     *             bottom:units = "count" ;
     *      short  caustic(eigenrays) ;
     *             caustic:units = "count" ;
     *
     *   // global attributes:
     *               :Conventions = "COARDS" ;
     *   data:
     *
     *      frequency = 6500, 7500, 8500, 9500 ;
     *
     *      source_id = 1 ;
     *
     *      receiver_id = 1 ;
     *
     *      slant_range = 0 ;  
     *
     *      distance_from_sensor = 0 ;
     *
     *      depth_offset = 0 ;
     *      
     *      initial_time = 0.253437554251589;
     *
     *      source_latitude = 0 ;
     *
     *      source_longitude = 0 ;
     *
     *      source_altitude = 0 ;
     *
     *      receiver_latitude = 0 ;
     *
     *      receiver_longitude = 0 ;
     *
     *      receiver_altitude = 0 ;
     *   
     *      intensity =
     *          63.3717061178703, 63.371726555249, 63.3717402233806, 63.3717498117019,
     *          79.4460538046972, 79.4460621977365, 79.4460678071192, 79.4460717403834,
     *          78.2782169632696, 78.2782251811778, 78.2782306738789, 78.2782345255009 ;
     *
     *      phase =
     *          -0.0202283729735675, -0.0202283729735675, -0.0202283729735675, -0.0202283729735675,
     *          3.10113590764266, 3.10113590764266, 3.10113590764266, 3.10113590764266,
     *          -0.0404567459471346, -0.0404567459471346, -0.0404567459471346, -0.0404567459471346 ;
     *
     *      travel_time = 0.253437554251589, 0.506873828206375, 0.506873828206375 ;
     *
     *      source_de = 80.9389514923578, -77.9155534787501, 80.9389514923578 ;
     *
     *      source_az = 160, 160, 160 ;
     *
     *      target_de = 80.1830639793879, 80.1830239583339, 80.1830239583341 ;
     *
     *      target_az = 159.999999998664, 159.999999994619, 159.999999994619 ;
     *
     *      surface = 1, 1, 2 ;
     *
     *      bottom = 1, 2, 2 ;
     *
     *      caustic = 0, 0, 0 ;
     * }
     * </pre>
     * @param   filename    Name of the file to write to disk.
     * @param   long_name   Optional global attribute for identifying data-set.
     *
     * @xref "The NetCDF Users Guide - Data Model, Programming Interfaces,
     * and Format for Self-Describing, Portable Data NetCDF", Version 3.6.3,
     * Section 3.4, 7 June 2008.
     */
    void write_netcdf( const char* filename, const char* long_name = NULL );

private:

    fathometer_collection() {};

   /**
    * The time of arrival of the fastest eigenray.
    */
    double _initial_time;

    /**
     * The slant range (in meters) of the sensor when the eigenrays were obtained.
     */
    double _slant_range;

    /**
     * The source sensor id
     */
    sensor_model::id_type _source_id;

    /**
     * The receiver sensor id
     */
    sensor_model::id_type _receiver_id;

    /**
     * The position of the source sensor when the eigenrays were obtained.
     */
    wposition1 _source_position;

    /**
     * The position of the receiver sensor when the eigenrays were obtained.
     */
    wposition1 _receiver_position;

    /**
     * Eigenrays that connect source and receiver locations.
     */
    eigenray_list _eigenrays;

    /**
     * Mutex that locks during eigenray access
     */
    mutable read_write_lock _eigenrays_mutex ;

};

/// @}
} // end of namespace sensors
} // end of namespace usml
