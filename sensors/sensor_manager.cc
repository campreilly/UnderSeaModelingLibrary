/**
 * @file sensor_manager.cc
 * Manages the containers for all the bistatic pair's in use by the USML.
 */

#include <usml/platforms/platform_manager.h>
#include <usml/sensors/sensor_manager.h>
#include <usml/threads/read_write_lock.h>

#include <list>
#include <memory>
#include <utility>

using namespace usml::sensors;

/**
 * Initialization of private static member _instance
 */
std::unique_ptr<sensor_manager> sensor_manager::_instance;

/**
 * The _mutex for the singleton sensor_manager.
 */
read_write_lock sensor_manager::_mutex;

/**
 * Singleton Constructor - Double Check Locking Pattern DCLP
 */
sensor_manager* sensor_manager::instance() {
    sensor_manager* tmp = _instance.get();
    if (tmp == nullptr) {
        write_lock_guard guard(_mutex);
        tmp = _instance.get();
        if (tmp == nullptr) {
            tmp = new sensor_manager();
            _instance.reset(tmp);
        }
    }
    return tmp;
}

/**
 * Reset the sensor_manager instance to empty.
 */
void sensor_manager::reset() {
    write_lock_guard guard(_mutex);
    _instance.reset();
    usml::platforms::platform_manager::reset();
}

/**
 * Frequencies over which propagation is computed (Hz).
 */
seq_vector::csptr sensor_manager::frequencies() const {
    read_lock_guard guard(_mutex);
    return _frequencies;
}

/**
 * Frequencies over which propagation is computed (Hz).
 */
void sensor_manager::frequencies(const seq_vector::csptr& freq) {
    write_lock_guard guard(_mutex);
    _frequencies = freq;
}

/**
 * Adds a sensor into the bistatic pair manager.
 */
void sensor_manager::add_sensor(const sensor_model::sptr& sensor,
                                update_listener<sensor_pair>* listener) {
    write_lock_guard guard(_mutex);
    if (_frequencies == nullptr || _frequencies->size() < 1) {
        throw freq_missing();
    }
    if (sensor->time_maximum() == 0.0) {
    	throw time_maximum_missing();
    }

    // add reference to platform_manager

    auto* pmgr = platform_manager::instance();
    pmgr->add(sensor);

    // add sensor ID to the lists of active sources and receivers

    if (sensor->is_source()) {
        _src_list.insert(sensor->keyID());
    }
    if (sensor->is_receiver()) {
        _rcv_list.insert(sensor->keyID());
    }

    // add pair as required

    if (sensor->is_source() && sensor->is_receiver()) {
        add_monostatic_pair(sensor, listener);
    }

    int multistatic = sensor->multistatic();
    if (multistatic > 0) {
        if (sensor->is_source()) {
            add_multistatic_source(sensor, multistatic, listener);
        }
        if (sensor->is_receiver()) {
            add_multistatic_receiver(sensor, multistatic, listener);
        }
    }
}

/**
 * Removes a sensor into the bistatic pair manager. Searches for all pairs that
 * have this sensor as a source or receiver.
 */
void sensor_manager::remove_sensor(const sensor_model::sptr& sensor,
                                   update_listener<sensor_pair>* listener) {
    write_lock_guard guard(_mutex);

    // remove sensor from the lists of active sources and receivers

    _src_list.erase(sensor->keyID());
    _rcv_list.erase(sensor->keyID());

    // search for pairs to remove

    remove_monostatic_pair(sensor, listener);
    remove_multistatic_source(sensor, listener);
    remove_multistatic_receiver(sensor, listener);

    // remove reference from platform_manager

    platform_manager::instance()->remove(sensor->keyID());
}

/**
 * Find a specific object in the map.
 */
typename sensor_model::sptr sensor_manager::find_sensor(
    typename sensor_model::key_type keyID) {
    auto* pmgr = platform_manager::instance();
    return std::dynamic_pointer_cast<sensor_model>(pmgr->find(keyID));
}

/**
 * Search all pairs for ones that have this sensor as a source.
 */
pair_list sensor_manager::find_source(sensor_model::key_type keyID) {
    read_lock_guard guard(_mutex);
    pair_list pair_list;
    for (int receiverID : _rcv_list) {
        auto hash_key = sensor_pair::generate_hash_key(keyID, receiverID);
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
pair_list sensor_manager::find_receiver(sensor_model::key_type keyID) {
    read_lock_guard guard(_mutex);
    pair_list pair_list;
    for (int sourceID : _src_list) {
        auto hash_key = sensor_pair::generate_hash_key(sourceID, keyID);
        auto pair = find(hash_key);
        if (pair != nullptr) {
            pair_list.push_back(pair);
        }
    }
    return pair_list;
}

/**
 * Adds a monostatic sensor pair if new sensor being added is both a source
 * and receiver. Called from sensor_manager::add_sensor().
 */
void sensor_manager::add_monostatic_pair(
    const sensor_model::sptr& sensor, update_listener<sensor_pair>* listener) {
    if (sensor->min_range() < 1e-6) {
        sensor_pair::sptr pair(new sensor_pair(sensor, sensor));
        if (listener != nullptr) {
            pair->add_listener(listener);
        }
        add(pair);
    }
}

/**
 * Creates bistatic pairs between the new source and all bistatic receivers.
 * Called from sensor_manager::add_sensor().
 */
void sensor_manager::add_multistatic_source(
    const sensor_model::sptr& source, int multistatic,
    update_listener<sensor_pair>* listener) {
    auto sourceID = source->keyID();
    for (auto receiverID : _rcv_list) {
        if (sourceID != receiverID) {
            auto receiver = find_sensor(receiverID);
            if (receiver->multistatic() == multistatic) {
                sensor_pair::sptr pair(new sensor_pair(source, receiver));
                if (listener != nullptr) {
                    pair->add_listener(listener);
                }
                add(pair);
            }
        }
    }
}

/**
 * Creates bistatic pairs between the new receiver and all bistatic sources.
 * Called from sensor_manager::add_sensor().
 */
void sensor_manager::add_multistatic_receiver(
    const sensor_model::sptr& receiver, int multistatic,
    update_listener<sensor_pair>* listener) {
    auto receiverID = receiver->keyID();
    for (auto sourceID : _src_list) {
        if (sourceID != receiverID) {
            auto source = find_sensor(sourceID);
            if (source->multistatic() == multistatic) {
                sensor_pair::sptr pair(new sensor_pair(source, receiver));
                if (listener != nullptr) {
                    pair->add_listener(listener);
                }
                add(pair);
            }
        }
    }
}

/**
 * Utility to remove a monosatic pair
 */
void sensor_manager::remove_monostatic_pair(
    const sensor_model::sptr& sensor, update_listener<sensor_pair>* listener) {
    auto sensorID = sensor->keyID();
    auto hash_key = sensor_pair::generate_hash_key(sensorID, sensorID);
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
void sensor_manager::remove_multistatic_source(
    const sensor_model::sptr& source, update_listener<sensor_pair>* listener) {
    auto sourceID = source->keyID();
    for (auto receiverID : _rcv_list) {
        if (sourceID != receiverID) {
            auto receiver = platform_manager::instance()->find(receiverID);
            auto hash_key =
                sensor_pair::generate_hash_key(sourceID, receiverID);
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
void sensor_manager::remove_multistatic_receiver(
    const sensor_model::sptr& receiver,
    update_listener<sensor_pair>* listener) {
    auto receiverID = receiver->keyID();
    for (auto sourceID : _rcv_list) {
        if (sourceID != receiverID) {
            auto source = platform_manager::instance()->find(sourceID);
            auto hash_key =
                sensor_pair::generate_hash_key(sourceID, receiverID);
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
