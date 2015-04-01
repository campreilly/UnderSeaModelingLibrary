/**
 * @file sensor_pair_manager.h
 * Stores and manages the active sensor pairs in use by the simulation.
 */
#pragma once

#include <usml/sensors/sensor.h>
#include <usml/sensors/sensor_pair.h>
#include <usml/sensors/sensor_map_template.h>
#include <usml/threads/read_write_lock.h>
#include <usml/threads/smart_ptr.h>
#include <set>

namespace usml {
namespace sensors {

/// @ingroup sensors
/// @{

/**
 * Stores and manages the active sensor pairs in use by the simulation.
 * The sensor
 * A sensor pair contains a source, receiver acoustic pair and it's
 * associated data. The each sensor_pair uses boost::shared_ptrs to the data
 * required. The sensor_pair_manager has a std::map, sensor_pair_map, that
 * uses a key that is a std::string type and consists of the sourceID + "_" + receiverID.
 * The payload of the sensor_pair_map is a pointer to the sensor_pair data.
 */
class USML_DECLSPEC sensor_pair_manager {

public:

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
	 * Gets the fathometers for the sourceID requested
	 * @param sourceID ID for the source
	 * @return eigenray_collection pointer
	 */
//	eigenray_collection* get_fathometers(sensor::id_type sourceID);

	/**
	 * Gets the envelopes for the receiverID requested
	 * @param receiverID ID for the receiver
	 * @return envelopes in the envelope_collection pointer
	 */
//	envelope_collection* get_envelopes(sensor::id_type receiverID);

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
	 * @param	from	Sensor that is being added.
	 */
	void add_sensor(sensor::reference& from);

	/**
	 * Removes existing sensor_pair objects in reaction to notification
	 * that a sensor is about to be deleted.
	 *
	 * @param	from	Sensor that is being removed.
	 */
	void remove_sensor(sensor::reference& from);

private:

	/**
	 * Utility to build a monostatic pair
	 *
	 * @param	sensor		Source/receiver combo for this pair
	 */
	void add_monostatic_pair(sensor::reference& sensor);

	/**
	 * Utility to build a multistatic pair from the source.
	 * Excludes monostatic case where sourceID == receiverID.
	 * Excludes sensors that don't support multi-static behaviors.
	 *
	 * @param	source		Sensor that needs to be paired with receivers.
	 */
	void add_multistatic_source(sensor::reference& source);

	/**
	 * Utility to delete a multistatic pair from the receiver.
	 * Excludes monostatic case where sourceID == receiverID.
	 * Excludes sensors that don't support multi-static behaviors.
	 * Also used to support multistatic sensors where mode() is BOTH.
	 *
	 * @param	sensor		Sensor that needs to be paired with sources.
	 */
	void add_multistatic_receiver(sensor::reference& receiver);

	/**
	 * Utility to build a monostatic pair
	 *
	 * @param	sensor		Source/receiver combo for this pair
	 */
	void remove_monostatic_pair(sensor::reference& sensor);

	/**
	 * Utility to delete a multistatic pair from the source.
	 * Excludes monostatic case where sourceID == receiverID.
	 * Excludes sensors that don't support multi-static behaviors.
	 *
	 * @param	source		Sensor that needs to be paired with receivers.
	 */
	void remove_multistatic_source(sensor::reference& source);

	/**
	 * Utility to delete a multistatic pair from the receiver.
	 * Excludes monostatic case where sourceID == receiverID.
	 * Excludes sensors that don't support multi-static behaviors.
	 * Also used to support multistatic sensors where mode() is BOTH.
	 *
	 * @param	sensor		Sensor that needs to be paired with sources.
	 */
	void remove_multistatic_receiver(sensor::reference& receiver);

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
    std::set <sensor::id_type> _src_list;

    /**
     * List of all active receiver sensor IDs.  Used by add_sensor() to find
     * find the sources that may need to be paired with each incoming receiver.
     */
    std::set <sensor::id_type> _rcv_list;

	/**
	 * Container for storing the sensor pair objects.
	 * Key is a string concatenation of "sourceID" + "_" + receiverID"
	 * Payload is a shared pointer to sensor_pair object.
	 */
	sensor_map_template<std::string, sensor_pair::reference> _map ;
};

/// @}
}// end of namespace sensors
}  // end of namespace usml
