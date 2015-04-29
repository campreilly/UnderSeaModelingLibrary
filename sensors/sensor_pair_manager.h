/**
 * @file sensor_pair_manager.h
 * Stores and manages the active sensor pairs in use by the simulation.
 */
#pragma once

#include <set>

#include <usml/sensors/sensor_model.h>
#include <usml/sensors/sensor_pair.h>
#include <usml/sensors/sensor_map_template.h>
#include <usml/sensors/fathometer_model.h>
#include <usml/threads/read_write_lock.h>
#include <usml/threads/smart_ptr.h>


namespace usml {
namespace sensors {

/// @ingroup sensors
/// @{

/**
 * Stores and manages the active sensor pairs in use by the simulation.
 * A sensor pair contains a source, receiver acoustic pair and it's
 * associated data. The each sensor_pair uses boost::shared_ptrs to the data
 * required. The sensor_pair_manager has a sensor_pair_map, that uses a 
 * std::string type hash key which created using the generate_hash_key call.
 * The payload of the sensor_pair_map is a shared pointer to the sensor_pair data.
 */
class USML_DECLSPEC sensor_pair_manager {

    friend class sensor_manager;

public:

    // Data type used to query the a random group of sensorID's and mode's
    typedef std::map<sensor_model::id_type,xmitRcvModeType> sensor_query_map ;
    typedef std::pair<sensor_model::id_type, xmitRcvModeType> query_type ;

	/**
	 * Singleton Constructor - Creates sensor_pair_manager instance just once.
	 * Accessible everywhere.
	 * @return  pointer to the instance of the sensor_pair_manager.
	 */
	static sensor_pair_manager* instance();

	/**
	 * Default destructor.
	 */
	virtual ~sensor_pair_manager();

    /**
     * Reset the sensor_pair_manager unique pointer to empty.
     */
    static void reset();

	/**
	 * Gets the fathometers for the list of sensors provided
	 * @param sensors contains sensor_query_map sensorID, and sensor xmitRcvModeType
	 * @return fathometer_model::fathometer_package contains a collection of fathometer_model pointers
	 */
    fathometer_model::fathometer_package get_fathometers(const sensor_query_map sensors);

    /**
     * Writes the fathometers provided to a NetCDF file.
     * @param fathometrs fathometer_model::fathometer_package contains
     *                    a collection of fathometer_model pointers
     * @params filename The name of the file to write the fathometers.
     *
     * Write fathometers data to a netCDF file using a ragged
     * array structure. This ragged array concept (see reference) stores
     * the fathometer_model data in a one dimensional list.
     *
     * This ragged array concept is used to define the intensity, phase,
     * source_de, source_az, target_de, target_az, surface, bottom, and
     * caustic variables.
     *
     * This file structure is illustrated (for a single target with
     * direct path, surface, and bottom eigenrays) in the netCDF sample below:
     * <pre>
     * netcdf fathometers {
     * 	dimensions:
     *         fathometers = 1 ;
     *         eigenrays = 3 ;
     *         frequency = 4 ;
     * 	variables:
     *         int fathometer_index(fathometers) ;
     *                 fathometer_index:units = "count" ;
     *         int source_id ;
     *         int receiver_id ;
     *         double slant_range ;
     *         double distance_from_sensor ;
     *         double depth_offset ;
     *         double source_latitude ;
     *                 source_latitude:units = "degrees_north" ;
     *         double source_longitude ;
     *                 source_longitude:units = "degrees_east" ;
     *         double source_altitude ;
     *                 source_altitude:units = "meters" ;
     *                 source_altitude:positive = "up" ;
     *         double receiver_latitude ;
     *                 receiver_latitude:units = "degrees_north" ;
     *         double receiver_longitude ;
     *                 receiver_longitude:units = "degrees_east" ;
     *         double receiver_altitude ;
     *                 receiver_altitude:units = "meters" ;
     *                 receiver_altitude:positive = "up" ;
     *         int eigenray_num(eigenrays) ; 
     *                 eigenray_num:units = "count" ;
     *         double frequency(frequency) ;
     *         double intensity(eigenrays, frequency) ;
     *                 intensity:units = "dB" ;       
     *         double phase(eigenrays, frequency) ;
     *                 phase:units = "radians" ;
     *         double travel_time(eigenrays) ;
     *                 travel_time:units = "seconds" ;
     *         double source_de(eigenrays) ;
     *                 source_de:units = "degrees" ;
     *                 source_de:positive = "up" ;
     *         double source_az(eigenrays) ;
     *                 source_az:units = "degrees_true" ;
     *                 source_az:positive = "clockwise" ;
     *         double target_de(eigenrays) ;
     *                 target_de:units = "degrees" ;
     *                 target_de:positive = "up" ;
     *         double target_az(eigenrays) ;
     *                 target_az:units = "degrees_true" ;
     *                 target_az:positive = "clockwise" ;
     *         short surface(eigenrays) ;
     *                 surface:units = "count" ;
     *         short bottom(eigenrays) ; 
     *                 bottom:units = "count" ;
     *         short caustic(eigenrays) ;
     *                 caustic:units = "count" ;
     * 
     * 	// global attributes:
     *                 :Conventions = "COARDS" ;
     * 	data:
     * 
     *      fathometer_index = 0 ;
     * 
     *      source_id = 1 ;
     * 
     *      receiver_id = 1 ;
     * 
     *      slant_range = 0 ;
     * 
     *      distance_from_sensor = 0 ;
     * 
     * 		depth_offset = 0 ;
     * 
     * 		source_latitude = 0 ;
     * 
     * 		source_longitude = 0 ;
     * 
     *  	source_altitude = 0 ;
     * 
     *  	receiver_latitude = 0 ;
     * 
     *  	receiver_longitude = 0 ; 
     * 
     *  	receiver_altitude = 0 ;
     * 
     * 		eigenray_num = 0, 1, 2 ;
     * 
     *  	frequency = 6500, 7500, 8500, 9500 ;
     * 
     *      intensity =
     *   		63.3717061178703, 63.371726555249, 63.3717402233806, 63.3717498117019,
     *   		79.4460538046972, 79.4460621977365, 79.4460678071192, 79.4460717403834,
     *   		78.2782169632696, 78.2782251811778, 78.2782306738789, 78.2782345255009 ;
     * 
     *  	phase =
     *   		-0.0202283729735675, -0.0202283729735675, -0.0202283729735675, -0.0202283729735675,
     *   		3.10113590764266, 3.10113590764266, 3.10113590764266, 3.10113590764266,
     *   		-0.0404567459471346, -0.0404567459471346, -0.0404567459471346, -0.0404567459471346 ;
     * 
     *  	travel_time = 0.253437554251589, 0.506873828206375, 0.506873828206375 ;
     * 
     *  	source_de = 80.9389514923578, -77.9155534787501, 80.9389514923578 ;
     * 
     *  	source_az = 160, 160, 160 ;
     * 
     * 		target_de = 80.1830639793879, 80.1830239583339, 80.1830239583341 ;
     * 
     * 		target_az = 159.999999998664, 159.999999994619, 159.999999994619 ;
     * 
     * 		surface = 1, 1, 2 ;
     * 
     *  	bottom = 1, 2, 2 ;
     * 
     *  	caustic = 0, 0, 0 ;
     * }
     * </pre>
     *
     * @xref "The NetCDF Users Guide - Data Model, Programming Interfaces,
     * and Format for Self-Describing, Portable Data NetCDF", Version 3.6.3,
     * Section 3.4, 7 June 2008.
     */
    void write_fathometers(fathometer_model::fathometer_package fathometers, const char* filename);

	/**
	 * Gets the envelopes for the receiverID requested
	 * @param receiverID ID for the receiver
	 * @return envelopes in the envelope_collection pointer
	 */
//	envelope_collection* get_envelopes(sensor_model::id_type receiverID);

protected:

	/**
	 * Builds new sensor_pair objects in reaction to notification
	 * that a sensor is being added.  First, this routine adds the sensorID
	 * of this object to the lists of active sources and receivers, based
	 * on the value if its mode() property.  Then, it builds pairs for
	 * each instance of the complementary type: source build pairs using
	 * receivers, receivers build pairs using sources.  This pairing relies
	 * on lists of active sources and receivers. Monostatic pairs,
	 * where the source is the same object as the receiver, are built
	 * of the mode() of the sensor is BOTH. Multistatic pairs are built
	 * if both the source and receiver have set their multistatic()
	 * property to true, and the source is not the same as the receiver.
	 *
	 * @param	sensor	Pointer to sensor_model that is being added.
	 */
	void add_sensor(sensor_model* sensor);

	/**
	 * Removes existing sensor_pair objects in reaction to notification
	 * that a sensor is about to be deleted.
	 *
	 * @param	sensor	Pointer to sensor_model that is being removed.
     * @return  false if sensor_model was not in the manager.
	 */
	bool remove_sensor(sensor_model* sensor);

private:

	/**
	 * Utility to build a monostatic pair.
	 *
	 * @param	sensor  Pointer to source/receiver combo for this pair
	 */
	void add_monostatic_pair(sensor_model* sensor);

	/**
	 * Utility to build a multistatic pair from the source.
	 * Excludes monostatic case where sourceID == receiverID.
	 * Excludes sensors that don't support multi-static behaviors.
	 *
	 * @param	source  Pointer to the sensor_model that needs to be paired with receivers.
	 */
	void add_multistatic_source(sensor_model* source);

	/**
	 * Utility to delete a multistatic pair from the receiver.
	 * Excludes monostatic case where sourceID == receiverID.
	 * Excludes sensors that don't support multi-static behaviors.
	 * Also used to support multistatic sensors where mode() is BOTH.
	 *
	 * @param	sensor	Pointer to the sensor_model that needs to be paired with sources.
	 */
	void add_multistatic_receiver(sensor_model* receiver);

	/**
	 * Utility to build a monostatic pair
	 *
	 * @param	sensor  Pointer to the source/receiver combo for this pair
	 */
	void remove_monostatic_pair(sensor_model* sensor);

	/**
	 * Utility to delete a multistatic pair from the source.
	 * Excludes monostatic case where sourceID == receiverID.
	 * Excludes sensors that don't support multi-static behaviors.
	 *
	 * @param	source  Pointer to the sensor_model that needs to be paired with receivers.
	 */
	void remove_multistatic_source(sensor_model* source);

	/**
	 * Utility to delete a multistatic pair from the receiver.
	 * Excludes monostatic case where sourceID == receiverID.
	 * Excludes sensors that don't support multi-static behaviors.
	 * Also used to support multistatic sensors where mode() is BOTH.
	 *
	 * @param	sensor  Pointer to the sensor_model that needs to be paired with sources.
	 */
	void remove_multistatic_receiver(sensor_model* receiver);

   /**
    * Utility to generate a hash key for the sensor_pair _map
    * @param	src_id   The source id used to generate the hash_key
    * @param	rcv_id   The receiver id used to generate the hash_key
    * @return   string   containing the generated hash_key.
    */
    std::string generate_hash_key(const sensor_model::id_type src_id,
                                    const sensor_model::id_type rcv_id)
    {
        std::stringstream key;
        key << src_id << '_' << rcv_id;
        return key.str();
    }

    /**
     * Utility to find the sensor_pair keys that are provided in the 
     * sensor_query_map parameter.
     * @param	sensors Map of sensorID and modes that needs to be found
     * @return   list of hash keys in the _map of found sensor_query
     */
    std::set<std::string> find_pairs(sensor_query_map& sensors);

    /**
     * Utility to determine if two frequency ranges overlap
     * Used to determine of a sensor_pair needs to be created.
     * @param	src_freq  frequency range for source
     * @param	rcv_freq  frequenc range for receiver
     * @return  true when frequency ranges overlap
     */
    bool frequencies_overlap(const seq_vector* src_freq, const seq_vector* rcv_freq);

	/**
	 * Hide access to default constructor.
	 */
	sensor_pair_manager() {
	}

	/**
	 * Hide access to copy constructor
	 */
	sensor_pair_manager(sensor_pair_manager const&);

	/**
	 * Hide access to assignment operator
	 */
	sensor_pair_manager& operator=(sensor_pair_manager const&);

	/**
	 * The singleton access pointer.
	 */
	static unique_ptr<sensor_pair_manager> _instance;

	/**
	 * The mutex for the singleton pointer.
	 */
	static read_write_lock _instance_mutex;

	/**
	 * The mutex for adding and removing pairs in manager.
	 */
	mutable read_write_lock _manager_mutex;

    /**
     * List of all active source sensor IDs.  Used by add_sensor() to find
     * find the receivers that may need to be paired with each incoming source.
     */
    std::set <sensor_model::id_type> _src_list;

    /**
     * List of all active receiver sensor IDs.  Used by add_sensor() to find
     * find the sources that may need to be paired with each incoming receiver.
     */
    std::set <sensor_model::id_type> _rcv_list;

	/**
	 * Container for storing the sensor pair objects.
	 * Key is a string concatenation of "sourceID" + "_" + receiverID"
     * See generate_hash_key method.
	 * Payload is a pointer to sensor_pair object.
	 */
	sensor_map_template<std::string,sensor_pair*> _map ;
};

/// @}
} // end of namespace sensors
} // end of namespace usml
