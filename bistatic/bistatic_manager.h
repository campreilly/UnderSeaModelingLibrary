/**
 * @file bistatic_manager.h
 * Stores and manages the active bistatic pairs in use by the simulation.
 */
#pragma once

#include <bits/exception.h>
#include <usml/bistatic/bistatic_pair.h>
#include <usml/managed/managed_obj.h>
#include <usml/managed/manager_template.h>
#include <usml/managed/update_listener.h>
#include <usml/threads/read_write_lock.h>
#include <usml/usml_config.h>

#include <list>
#include <memory>
#include <set>

namespace usml {
namespace platforms {
class sensor_model;
} /* namespace platforms */
} /* namespace usml */

namespace usml {
namespace bistatic {

using namespace usml::platforms;

/// @ingroup bistatic
/// @{

/**
 * Stores and manages the bistatic pairs in use by the simulation. Uses the
 * is_source() and is_receiver() members of the sensor_model class to
 * automatically identify all the cases where added sensors act as the source or
 * receiver in a pair. Sensors must appear in the platform_manager in order for
 * them to operate properly in the bistatic_manager.
 *
 * Uses the update_pair_notifier interface to pass bistatic_pair updates from
 * individual pairs to those listening to just the bistatic_manager.
 */
class USML_DECLSPEC bistatic_manager : public manager_template<bistatic_pair> {
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
    static bistatic_manager* instance();

    /**
     * Removes all sensors from the manager and destroys it.
     */
    static void reset();

    /**
     * Adds a sensor into the bistatic pair manager. Searches for all other
     * sensors that can be paired with the new sensor.
     *
     * @param sensor		Reference to the sensor to add.
     * @param listener      Optional update listener for bistatic_pair objects.
     * @throw missing_key   If sensor not found in platform_manager.
     */
    void add_sensor(const sensor_model::sptr& sensor,
                    update_listener<bistatic_pair>* listener = nullptr);

    /**
     * Removes a sensor into the bistatic pair manager. Searches for all
     * pairs that have this sensor as a source or receiver.
     *
     * @param sensor		Reference to the sensor to remove.
     * @param listener      Optional update listener for bistatic_pair objects.
     */
    void remove_sensor(const sensor_model::sptr& sensor,
                       update_listener<bistatic_pair>* listener = nullptr);

    /**
     * Search all pairs for ones that have this sensor as a source.
     *
     * @param keyID		ID used to lookup sensor in platform_manager.
     * @return 			List of pairs that include this sensor.
     */
    bistatic_list find_source(sensor_model::key_type keyID);

    /**
     * Search all pairs for ones that have this sensor as a receiver.
     *
     * @param keyID		ID used to lookup sensor in platform_manager.
     * @return 			List of pairs that include this sensor.
     */
    bistatic_list find_receiver(sensor_model::key_type keyID);

   private:
    /**
     * Adds a monostatic sensor pair if the sensor being added is a
     * monostatic sensor. Called within the bistatic_manager.
     *
     * @param sensor 	Monostatic sensor to be added as a monostatic pair.
     * @param listener	Update listener for bistatic_pair objects.
     */
    void add_monostatic_pair(const sensor_model::sptr& sensor,
                             update_listener<bistatic_pair>* listener);

    /**
     * Adds a multistatic source to the pair_manager and makes pairs
     * with all valid receivers. Called within the bistatic_manager.
     *
     * @param source 	Multistatic source to be paired with valid receivers.
     * @param listener	Update listener for bistatic_pair objects.
     */
    void add_multistatic_source(const sensor_model::sptr& source,
                                update_listener<bistatic_pair>* listener);

    /**
     * Add a multistatic receiver to the bistatic_manager and makes pairs
     * with all valid sources. Called within the bistatic_manager.
     *
     * @param receiver 	Multistatic receiver to be paired with valid sources.
     * @param listener	Update listener for bistatic_pair objects.
     */
    void add_multistatic_receiver(const sensor_model::sptr& receiver,
                                  update_listener<bistatic_pair>* listener);

    /**
     * Removes a monostatic pair from the bistatic_manager. Called
     * within the bistatic_manager.
     *
     * @param sensor 	Monostatic sensor to be removed.
     * @param listener	Update listener for bistatic_pair objects.
     */
    void remove_monostatic_pair(const sensor_model::sptr& sensor,
                                update_listener<bistatic_pair>* listener);

    /**
     * Removes all multistatic pairs with the provided source. Called
     * Within the bistatic_pair_manger.
     *
     * @param source 	Multistatic source to be removed.
     * @param listener	Update listener for bistatic_pair objects.
     */
    void remove_multistatic_source(const sensor_model::sptr& source,
                                   update_listener<bistatic_pair>* listener);

    /**
     * Removes all multistatic pairs with he provided receiver. Called
     * within the bistatic_manager
     *
     * @param receiver 	Multistatic receiver to be removed.
     * @param listener	Update listener for bistatic_pair objects.
     */
    void remove_multistatic_receiver(const sensor_model::sptr& receiver,
                                     update_listener<bistatic_pair>* listener);

    /**
     * The singleton access pointer.
     */
    static std::unique_ptr<bistatic_manager> _instance;

    /**
     * The mutex for the singleton pointer.
     */
    static read_write_lock _instance_mutex;

    /**
     * The mutex for the manager.
     */
    read_write_lock _mutex;

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
}  // namespace bistatic
}  // namespace usml
