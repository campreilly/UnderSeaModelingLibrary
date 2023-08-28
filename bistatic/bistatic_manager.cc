/**
 * @file bistatic_manager.cc
 * Manages the containers for all the bistatic pair's in use by the USML.
 */

#include <usml/bistatic/bistatic_manager.h>
#include <usml/platforms/platform_manager.h>
#include <usml/threads/read_write_lock.h>

#include <list>
#include <memory>

using namespace usml::bistatic;
using namespace usml::platforms;

/**
 * Initialization of private static member _instance
 */
std::unique_ptr<bistatic_manager> bistatic_manager::_instance;

/**
 * The _mutex for the singleton bistatic_manager.
 */
read_write_lock bistatic_manager::_instance_mutex;

/**
 * Singleton Constructor - Double Check Locking Pattern DCLP
 */
bistatic_manager* bistatic_manager::instance() {
    bistatic_manager* tmp = _instance.get();
    if (tmp == nullptr) {
        write_lock_guard guard(_instance_mutex);
        tmp = _instance.get();
        if (tmp == nullptr) {
            tmp = new bistatic_manager();
            _instance.reset(tmp);
        }
    }
    return tmp;
}

/**
 * Reset the bistatic_manager instance to empty.
 */
void bistatic_manager::reset() {
    write_lock_guard guard(_instance_mutex);
    _instance.reset();
}

/**
 * Adds a sensor into the bistatic pair manager.
 */
void bistatic_manager::add_sensor(const sensor_model::sptr& sensor,
                                  update_listener<bistatic_pair>* listener) {
    write_lock_guard guard(_mutex);

    // add sensor ID to the lists of active sources and receivers

    const auto* sensor_ptr = dynamic_cast<const sensor_model*>(sensor.get());
    if (sensor_ptr->is_source()) {
        _src_list.insert(sensor->keyID());
    }
    if (sensor_ptr->is_receiver()) {
        _rcv_list.insert(sensor->keyID());
    }

    // add pair as required

    if (sensor_ptr->is_source() && sensor_ptr->is_receiver()) {
        add_monostatic_pair(sensor, listener);
        if (sensor_ptr->multistatic()) {
            if (sensor_ptr->is_source()) {
                add_multistatic_source(sensor, listener);
            }
            if (sensor_ptr->is_receiver()) {
                add_multistatic_receiver(sensor, listener);
            }
        }
    } else if (sensor_ptr->multistatic()) {
        if (sensor_ptr->is_source()) {
            add_multistatic_source(sensor, listener);
        }
        if (sensor_ptr->is_receiver()) {
            add_multistatic_receiver(sensor, listener);
        }
    }
}

/**
 * Removes a sensor into the bistatic pair manager. Searches for all pairs that
 * have this sensor as a source or receiver.
 */
void bistatic_manager::remove_sensor(const sensor_model::sptr& sensor,
                                     update_listener<bistatic_pair>* listener) {
    write_lock_guard guard(_mutex);

    // remove sensor from the lists of active sources and receivers

    _src_list.erase(sensor->keyID());
    _rcv_list.erase(sensor->keyID());

    // search for pairs to remove

    remove_monostatic_pair(sensor, listener);
    remove_multistatic_source(sensor, listener);
    remove_multistatic_receiver(sensor, listener);
}

/**
 * Search all pairs for ones that have this sensor as a source.
 */
bistatic_list bistatic_manager::find_source(
    sensor_model::key_type keyID) {
    read_lock_guard guard(_mutex);
    bistatic_list pair_list;
    for (int receiverID : _rcv_list) {
        auto hash_key = bistatic_pair::generate_hash_key(keyID, receiverID);
        auto pair = find(hash_key);
        if (pair != nullptr) {
            pair_list.push_back(pair);
        }
    }
    return pair_list;
}

/**
 * Search all pairs for ones that have this sensor as a receiver.
 */
bistatic_list bistatic_manager::find_receiver(
    sensor_model::key_type keyID) {
    read_lock_guard guard(_mutex);
    bistatic_list pair_list;
    for (int sourceID : _src_list) {
        auto hash_key = bistatic_pair::generate_hash_key(sourceID, keyID);
        auto pair = find(hash_key);
        if (pair != nullptr) {
            pair_list.push_back(pair);
        }
    }
    return pair_list;
}

/**
 * Utility to build a monostatic pair
 */
void bistatic_manager::add_monostatic_pair(
    const sensor_model::sptr& sensor,
    update_listener<bistatic_pair>* listener) {
    bistatic_pair::sptr pair(new bistatic_pair(sensor, sensor));
    if (listener != nullptr) {
        pair->add_listener(listener);
    }
    add(pair);
}

/**
 * Utility to build a multistatic pair with the given sensor being the source
 */
void bistatic_manager::add_multistatic_source(
    const sensor_model::sptr& source,
    update_listener<bistatic_pair>* listener) {
    auto sourceID = source->keyID();
    for (auto receiverID : _rcv_list) {
        if (sourceID != receiverID) {
            auto receiver = platform_manager::instance()->find(receiverID);
            bistatic_pair::sptr pair(new bistatic_pair(source, receiver));
            if (listener != nullptr) {
                pair->add_listener(listener);
            }
            add(pair);
        }
    }
}

/**
 * Utility to build a multi-static pair with the given sensor being the
 * receiver
 */
void bistatic_manager::add_multistatic_receiver(
    const sensor_model::sptr& receiver,
    update_listener<bistatic_pair>* listener) {
    auto receiverID = receiver->keyID();
    for (auto sourceID : _src_list) {
        if (sourceID != receiverID) {
            auto source = platform_manager::instance()->find(sourceID);
            bistatic_pair::sptr pair(new bistatic_pair(source, receiver));
            if (listener != nullptr) {
                pair->add_listener(listener);
            }
            add(pair);
        }
    }
}

/**
 * Utility to remove a monosatic pair
 */
void bistatic_manager::remove_monostatic_pair(
    const sensor_model::sptr& sensor,
    update_listener<bistatic_pair>* listener) {
    auto sensorID = sensor->keyID();
    auto hash_key = bistatic_pair::generate_hash_key(sensorID, sensorID);
    auto pair = find(hash_key);
    if (pair != nullptr) {
        remove(hash_key);
        if (listener != nullptr) {
            pair->remove_listener(listener);
        }
    }
}

/**
 * Utility to remove a multistatic pair with the given sensor being the source
 */
void bistatic_manager::remove_multistatic_source(
    const sensor_model::sptr& source,
    update_listener<bistatic_pair>* listener) {
    auto sourceID = source->keyID();
    for (auto receiverID : _rcv_list) {
        if (sourceID != receiverID) {
            auto receiver = platform_manager::instance()->find(receiverID);
            auto hash_key =
                bistatic_pair::generate_hash_key(sourceID, receiverID);
            auto pair = find(hash_key);
            if (pair != nullptr) {
                remove(hash_key);
                if (listener != nullptr) {
                    pair->remove_listener(listener);
                }
            }
        }
    }
}

/**
 * Utility to remove a multistatic pair with the given sensor being the
 * receiver
 */
void bistatic_manager::remove_multistatic_receiver(
    const sensor_model::sptr& receiver,
    update_listener<bistatic_pair>* listener) {
    auto receiverID = receiver->keyID();
    for (auto sourceID : _rcv_list) {
        if (sourceID != receiverID) {
            auto source = platform_manager::instance()->find(sourceID);
            auto hash_key =
                bistatic_pair::generate_hash_key(sourceID, receiverID);
            auto pair = find(hash_key);
            if (pair != nullptr) {
                remove(hash_key);
                if (listener != nullptr) {
                    pair->remove_listener(listener);
                }
            }
        }
    }
}
