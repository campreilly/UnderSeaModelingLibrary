/**
 * @file fathometer_model.h
 * Container for one fathometer_model instance.
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
 * Container for one fathometer_model instance.
 * On construction takes in all source and receiver data and eigenrays
 */
class USML_DECLSPEC fathometer_model
{
public:

    /**
    * Data type used handle a collection of fathometer_model pointers.
    */
    typedef std::vector<fathometer_model*> fathometer_package;

	/**
	 * Data type used for reference to a fathometer_model.
	 */
    typedef shared_ptr<fathometer_model> reference;

    /**
     * Construct from all data required.
     *
     * @param	source_id   The source id for this pair.
     * @param	receiver_id	The receiver id for this pair.
     * @param	src_pos		The source position when eigenrays were obtained.
     * @param	rcv_pos	    The receiver position when eigenrays were obtained.
     * @param   eigenrays   Shared Pointer to the eigenray list.
     */
    fathometer_model(sensor_model::id_type source_id, sensor_model::id_type receiver_id,
                     wposition1 src_pos, wposition1 rcv_pos, shared_ptr<eigenray_list> list )
        : _source_id(source_id), _receiver_id(receiver_id), _slant_range(0.0), 
        _distance_from_sensor(0.0), _depth_offset_from_sensor(0.0),
        _source_position(src_pos), _receiver_position(rcv_pos), _eigenrays(list)
     {}

    /**
     * Destructor
     */
    ~fathometer_model()
    {
    }

    /**
     * Gets the source sensor id.
     * @return  The source sensor id.
     */
    sensor_model::id_type source_id() {
        return _source_id;
    }

    /**
     * Sets the source sensor id.
     * @param  The source sensor id
     */
    void source_id(sensor_model::id_type source_id) {
        _source_id = source_id;
    }

    /**
     * Gets the receiver sensor id.
     * @return  The receiver sensor id.
     */
    sensor_model::id_type receiver_id() {
        return _receiver_id;
    }

    /**
     * Sets the receiver sensor id.
     * @param  The receiver sensor id.
     */
    void receiver_id(sensor_model::id_type receiver_id) {
        _receiver_id = receiver_id;
    }

    /**
     * Sets the slant_range.
     * @param  The slant_range.
     */
    void slant_range(double slant_range) {
        _slant_range = slant_range;
    }

    /**
     * Gets the slant_range.
     * @return  The slant_range. 
     */
    double slant_range() {
        return _slant_range;
    }

    /**
     * Sets the distance_from_sensor.
     * @param  The distance_from_sensor.
     */
    void distance_from_sensor(double distance_from_sensor) {
        _distance_from_sensor = distance_from_sensor;
    }

    /**
     * Gets the distance_from_sensor.
     * @return  The distance_from_sensor.
     */
    double distance_from_sensor() {
        return _distance_from_sensor;
    }

    /**
     * Sets the depth_offset_from_sensor.
     * @param  The depth_offset_from_sensor.
     */
    void depth_offset(double depth_offset) {
        _depth_offset_from_sensor = depth_offset;
    }

    /**
     * Gets the depth_offset_from_sensor.
     * @return  The depth_offset_from_sensor.
     */
    double depth_offset() {
        return _depth_offset_from_sensor;
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
     * @param  The source position
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
    * @param  The receiver position
    */
    void receiver_position(wposition1 position) {
        _receiver_position = position;
    }

	/**
     * Gets the shared_ptr to last eigenray_list update for this fathometer_model.
     * @return  eigenray_list shared_ptr
     */
    boost::shared_ptr<eigenray_list> eigenrays() {
         read_lock_guard guard(_eigenrays_mutex);
         return _eigenrays;
    }

    /**
     * Sets the shared_ptr to eigenray_list for this fathometer_model.
     * @return  eigenray_list shared_ptr to the eigenray_list
     */
    void eigenrays(eigenray_list* list) {
         write_lock_guard guard(_eigenrays_mutex);
         _eigenrays = boost::shared_ptr<eigenray_list>(list);
    }

    /**
     * Write fathometer_model data to a netCDF file using a ragged
     * array structure. This ragged array concept (see reference) stores
     * the fathometer_model data in a one dimensional list and uses an externally
     * defined index to lookup the appropriate elements for each target.
     *
     * This ragged array concept is used to define the intensity, phase,
     * source_de, source_az, target_de, target_az, surface, bottom, and
     * caustic variables. The proploss_index variable defines the lookup index
     * into these arrays for the summed fathometer_model for each target.  The
     * eigenray_index variable defines a similar index for the beginning of the
     * eigenray list.  Subsequent eigenrays for this target immediately follow
     * the 1st eigenray.  The eigenray_number variable defines the number of
     * eigenrays for each target.
     *
     * This file structure is illustrated (for a single target with
     * direct path, surface, and bottom eigenrays) in the netCDF sample below:
     * <pre>
     *     netcdf eigenray_basic {
     *     dimensions:
     *      frequency = 1 ;
     *      rows = 1 ;
     *      cols = 1 ;
     *      eigenrays = 4 ;
     *      launch_de = 25 ;
     *      launch_az = 5 ;
     *     variables:
     *      double source_latitude ;
     *          source_latitude:units = "degrees_north" ;
     *      double source_longitude ;
     *          source_longitude:units = "degrees_east" ;
     *      double source_altitude ;
     *          source_altitude:units = "meters" ;
     *          source_altitude:positive = "up" ;
     *      double launch_de(launch_de) ;
     *          launch_de:units = "degrees" ;
     *          launch_de:positive = "up" ;
     *      double launch_az(launch_az) ;
     *          launch_az:units = "degrees_true" ;
     *          launch_az:positive = "clockwise" ;
     *      double time_step ;
     *          time_step:units = "seconds" ;
     *      double frequency(frequency) ;
     *          frequency:units = "hertz" ;
     *      double latitude(rows, cols) ;
     *          latitude:units = "degrees_north" ;
     *      double longitude(rows, cols) ;
     *          longitude:units = "degrees_east" ;
     *      double altitude(rows, cols) ;
     *          altitude:units = "meters" ;
     *          altitude:positive = "up" ;
     *      short proploss_index(rows, cols) ;
     *          proploss_index:units = "count" ;
     *      short eigenray_index(rows, cols) ;
     *          eigenray_index:units = "count" ;
     *      short eigenray_num(rows, cols) ;
     *          eigenray_num:units = "count" ;
     *      double intensity(eigenrays, frequency) ;
     *          intensity:units = "dB" ;
     *      double phase(eigenrays, frequency) ;
     *          phase:units = "radians" ;
     *      double travel_time(eigenrays) ;
     *          travel_time:units = "seconds" ;
     *      double source_de(eigenrays) ;
     *          source_de:units = "degrees" ;
     *          source_de:positive = "up" ;
     *      double source_az(eigenrays) ;
     *          source_az:units = "degrees_true" ;
     *          source_az:positive = "clockwise" ;
     *      double target_de(eigenrays) ;
     *          target_de:units = "degrees" ;
     *          target_de:positive = "up" ;
     *      double target_az(eigenrays) ;
     *          target_az:units = "degrees_true" ;
     *          target_az:positive = "clockwise" ;
     *      short surface(eigenrays) ;
     *          surface:units = "count" ;
     *      short bottom(eigenrays) ;
     *          bottom:units = "count" ;
     *      short caustic(eigenrays) ;
     *          caustic:units = "count" ;
     *
     *     // global attributes:
     *          :long_name = "eigenray_basic test" ;
     *          :Conventions = "COARDS" ;
     *     data:
     *      source_latitude = 45 ;
     *      source_longitude = -45 ;
     *      source_altitude = -1000 ;
     *      launch_de = -60, -55, -50, -45, -40, -35, -30, -25, -20, -15, -10, -5, 0, 5,
     *         10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60 ;
     *      launch_az = -2, -1, 0, 1, 2 ;
     *      time_step = 0.1 ;
     *      frequency = 100000 ;
     *      latitude = 45.02 ;
     *      longitude = -45 ;
     *      altitude = -1000 ;
     *      proploss_index = 0 ;
     *      eigenray_index = 1 ;
     *      eigenray_num = 3 ;
     *      intensity = 64.1427331557955, 66.9694875322853, 69.5617366839523, 77.9679676720843 ;
     *      phase = 0.082693193293971, 0, -3.14159265358979, 0 ;
     *      travel_time = 1.88788322659623, 1.48401881064351, 1.99622641373789, 3.03542140949814 ;
     *      source_de = 6.86829906648868, -0.00988717967364023, 41.9270555318522, -61.0113452826929 ;
     *      source_az = 0, 0, 0, 0 ;
     *      target_de = -6.86819212315001, 0.0101128206074365, -41.9270291816411, 61.0112432784064 ;
     *      target_az = 0, 0, 0, 0 ;
     *      surface = 0, 0, 1, 0 ;
     *      bottom = 0, 0, 0, 1 ;
     *      caustic = 0, 0, 0, 0 ;
     *     }
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

    fathometer_model() {};

    /**
     * The source sensor id
     */
    sensor_model::id_type _source_id;

    /**
     * The receiver sensor id
     */
    sensor_model::id_type _receiver_id;

    /**
     * The slant range (in meters) of the sensor when the eigenrays where obtained.
     */
    double _slant_range;

    /**
     * The distance (in meters) from the sensor when the eigenrays where obtained.
     */
    double _distance_from_sensor;

    /**
     * The depth offset (in meters) from the sensor when the eigenrays where obtained.
     */
    double _depth_offset_from_sensor;

    /**
     * The position of the source sensor when the eigenrays where obtained.
     */
    wposition1 _source_position;

    /**
     * The position of the receiver sensor when the eigenrays where obtained.
     */
    wposition1 _receiver_position;

    /**
     * Eigenrays that connect source and receiver locations.
     */
    boost::shared_ptr<eigenray_list> _eigenrays;

	/**
	 * Mutex that locks during eigenray access
	 */
	mutable read_write_lock _eigenrays_mutex ;

};

/// @}
} // end of namespace sensors
} // end of namespace usml
