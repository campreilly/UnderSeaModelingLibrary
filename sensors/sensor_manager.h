/**
 * @file sensor_manager.h
 * Stores and manages the active bistatic sensor pairs in use by the simulation.
 */
#pragma once

#include <bits/exception.h>
#include <usml/managed/manager_template.h>
#include <usml/managed/update_listener.h>
#include <usml/sensors/sensor_model.h>
#include <usml/sensors/sensor_pair.h>
#include <usml/threads/read_write_lock.h>
#include <usml/types/seq_vector.h>
#include <usml/usml_config.h>

#include <memory>
#include <set>

namespace usml {
namespace sensors {

using namespace usml::managed;
using namespace usml::threads;
using namespace usml::types;

/// @ingroup sensors
/// @{

/**
 * Stores and manages the bistatic sensor pairs in use by the simulation. Uses
 * the is_source() and is_receiver() members of the sensor_model class to
 * automatically identify all the cases where added sensors act as the source or
 * receiver in a pair.
 */
class USML_DECLSPEC sensor_manager : public manager_template<sensor_pair> {
   public:
    /// Exception thrown if keyID not found.
    struct missing_key : public std::exception {
        const char* what() const throw() { return "missing key"; }
    };

    /**
     * Singleton constructor, implemented using double-checked locking
     * pattern.
     *
     * @return Pointer to the singleton,
     */
    static sensor_manager* instance();

    /**
     * Removes all sensors from the manager and destroys it.
     * Also destroys the platform_manager as a side effect.
     */
    static void reset();

    /**
     * Frequencies over which propagation is computed (Hz). Making this common
     * to all the sensors controlled by this manager avoids the problem of
     * having to compute the frequency overlap between sources and receivers.
     */
    seq_vector::csptr frequencies() const;

    /**
     * Frequencies over which propagation is computed (Hz).
     */
    void frequencies(seq_vector::csptr freq);

    /**
     * Adds a sensor into the bistatic pair manager. Searches for all other
     * sensors that can be paired with the new sensor. Uses the presence or
     * absence of source and receiver beam patterns to determine if the new
     * sensor has transmit/receive capabilities. Adds a monostatic pair if the
     * new sensor can act as both a source and receiver.
     *
     * @param sensor		Reference to the sensor to add.
     * @param listener      Optional update listener for sensor_pair objects.
     * @throw missing_key   If sensor not found in platform_manager.
     */
    void add_sensor(const sensor_model::sptr& sensor,
                    update_listener<sensor_pair>* listener = nullptr);

    /**
     * Removes a sensor into the bistatic pair manager. Searches for all
     * pairs that have this sensor as a source or receiver.
     *
     * @param sensor		Reference to the sensor to remove.
     * @param listener      Optional update listener for sensor_pair objects.
     */
    void remove_sensor(const sensor_model::sptr& sensor,
                       update_listener<sensor_pair>* listener = nullptr);

    /**
     * Find a specific sensor_model in the platform_manager. Manages casting the
     * shared pointer into a form that supports access to sensor_model
     * attributes and methods.
     *
     * @param keyID 	Identification used to find this sensor_model.
     * @return    		nullptr if not found.
     */
    static typename sensor_model::sptr find_sensor(
        typename sensor_model::key_type keyID);

    /**
     * Search all pairs for ones that have this sensor as a source.
     *
     * @param keyID		ID used to lookup sensor in platform_manager.
     * @return 			List of pairs that include this sensor.
     */
    pair_list find_source(sensor_model::key_type keyID);

    /**
     * Search all pairs for ones that have this sensor as a receiver.
     *
     * @param keyID		ID used to lookup sensor in platform_manager.
     * @return 			List of pairs that include this sensor.
     */
    pair_list find_receiver(sensor_model::key_type keyID);

   private:
    /**
     * Adds a monostatic sensor pair if new sensor being added is both a source
     * and receiver. Called from sensor_manager::add_sensor().
     *
     * @param sensor 	Monostatic sensor to be added as a monostatic pair.
     * @param listener	Update listener for sensor_pair objects.
     */
    void add_monostatic_pair(const sensor_model::sptr& sensor,
                             update_listener<sensor_pair>* listener);

    /**
     * Creates bistatic pairs between the new source and all bistatic receivers.
     * Called from sensor_manager::add_sensor().
     *
     * @param source 		Multistatic source to be paired with valid
     * receivers.
     * @param multistatic 	Multistatic group for this sensor.
     * @param listener		Update listener for sensor_pair objects.
     */
    void add_multistatic_source(const sensor_model::sptr& source,
                                int multistatic,
                                update_listener<sensor_pair>* listener);

    /**
     * Creates bistatic pairs between the new receiver and all bistatic sources.
     * Called from sensor_manager::add_sensor().
     *
     * @param receiver 		Multistatic receiver to be paired with valid
     * sources.
     * @param multistatic 	Multistatic group for this sensor.
     * @param listener		Update listener for sensor_pair objects.
     */
    void add_multistatic_receiver(const sensor_model::sptr& receiver,
                                  int multistatic,
                                  update_listener<sensor_pair>* listener);

    /**
     * Removes a monostatic pair from the sensor_manager. Called
     * within the sensor_manager.
     *
     * @param sensor 	Monostatic sensor to be removed.
     * @param listener	Update listener for sensor_pair objects.
     */
    void remove_monostatic_pair(const sensor_model::sptr& sensor,
                                update_listener<sensor_pair>* listener);

    /**
     * Removes all multistatic pairs with the provided source. Called
     * Within the sensor_pair_manger.
     *
     * @param source 	Multistatic source to be removed.
     * @param listener	Update listener for sensor_pair objects.
     */
    void remove_multistatic_source(const sensor_model::sptr& source,
                                   update_listener<sensor_pair>* listener);

    /**
     * Removes all multistatic pairs with he provided receiver. Called
     * within the sensor_manager
     *
     * @param receiver 	Multistatic receiver to be removed.
     * @param listener	Update listener for sensor_pair objects.
     */
    void remove_multistatic_receiver(const sensor_model::sptr& receiver,
                                     update_listener<sensor_pair>* listener);

    /**
     * The singleton access pointer.
     */
    static std::unique_ptr<sensor_manager> _instance;

    /// Mutex for singleton access.
    static read_write_lock _mutex;

    /// Frequencies over which propagation is computed (Hz).
    seq_vector::csptr _frequencies;

    /**
     * List of all active source sensor IDs.  Used by insert() to
     * find the receivers that may need to be paired with each incoming
     * source.
     */
    std::set<int> _src_list;

    /**
     * List of all active receiver sensor IDs.  Used by insert() to
     * find the sources that may need to be paired with each incoming
     * receiver.
     */
    std::set<int> _rcv_list;
};

/// @}
}  // namespace sensors
}  // namespace usml
