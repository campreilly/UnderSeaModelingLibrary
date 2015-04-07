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

public:

    // Data type used to query the a random group of sensorID's and mode's
    typedef std::map<sensor_model::id_type,xmitRcvModeType> sensor_query_map;

	/**
	 * Singleton Constructor - Creates sensor_pair_manager instance just once.
	 * Accessible everywhere.
	 * @return  pointer to the instance of the sensor_pair_manager.
	 */
	static sensor_pair_manager* instance();

	/**
	 * Default destructor.
	 */
	virtual ~sensor_pair_manager() {
	}

	/**
	 * Gets the fathometers for the list of sensors provided
	 * @param sensors contains sensor_query_map sensorID, and sensor xmitRcvModeType
	 * @return fathometer_model::fathometer_package contains a collection of fathometer_model pointers
	 */
    fathometer_model::fathometer_package get_fathometers(const sensor_query_map sensors);

	/**
	 * Gets the envelopes for the receiverID requested
	 * @param receiverID ID for the receiver
	 * @return envelopes in the envelope_collection pointer
	 */
//	envelope_collection* get_envelopes(sensor_model::id_type receiverID);

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
	 * @param	sensor	Sensor that is being added.
	 */
	void add_sensor(sensor_model::reference& sensor);

	/**
	 * Removes existing sensor_pair objects in reaction to notification
	 * that a sensor is about to be deleted.
	 *
	 * @param	sensor	Sensor that is being removed.
     * @return  false if Sensor was not in the manager.
	 */
	bool remove_sensor(sensor_model::reference& sensor);

private:

	/**
	 * Utility to build a monostatic pair
	 *
	 * @param	sensor		Source/receiver combo for this pair
	 */
	void add_monostatic_pair(sensor_model::reference& sensor);

	/**
	 * Utility to build a multistatic pair from the source.
	 * Excludes monostatic case where sourceID == receiverID.
	 * Excludes sensors that don't support multi-static behaviors.
	 *
	 * @param	source		Sensor that needs to be paired with receivers.
	 */
	void add_multistatic_source(sensor_model::reference& source);

	/**
	 * Utility to delete a multistatic pair from the receiver.
	 * Excludes monostatic case where sourceID == receiverID.
	 * Excludes sensors that don't support multi-static behaviors.
	 * Also used to support multistatic sensors where mode() is BOTH.
	 *
	 * @param	sensor		Sensor that needs to be paired with sources.
	 */
	void add_multistatic_receiver(sensor_model::reference& receiver);

	/**
	 * Utility to build a monostatic pair
	 *
	 * @param	sensor		Source/receiver combo for this pair
	 */
	void remove_monostatic_pair(sensor_model::reference& sensor);

	/**
	 * Utility to delete a multistatic pair from the source.
	 * Excludes monostatic case where sourceID == receiverID.
	 * Excludes sensors that don't support multi-static behaviors.
	 *
	 * @param	source		Sensor that needs to be paired with receivers.
	 */
	void remove_multistatic_source(sensor_model::reference& source);

	/**
	 * Utility to delete a multistatic pair from the receiver.
	 * Excludes monostatic case where sourceID == receiverID.
	 * Excludes sensors that don't support multi-static behaviors.
	 * Also used to support multistatic sensors where mode() is BOTH.
	 *
	 * @param	sensor		Sensor that needs to be paired with sources.
	 */
	void remove_multistatic_receiver(sensor_model::reference& receiver);

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
	 * Payload is a shared pointer to sensor_pair object.
	 */
	sensor_map_template<std::string, sensor_pair::reference> _map ;
};

/// @}
}// end of namespace sensors
}  // end of namespace usml
