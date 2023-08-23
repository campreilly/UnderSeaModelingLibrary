/**
 * @file manager_template.h
 * Template for a thread-safe map to store and manage dynamic objects.
 */
#pragma once

#include <bits/exception.h>
#include <usml/managed/manager_listener.h>
#include <usml/threads/read_write_lock.h>
#include <usml/usml_config.h>

#include <list>
#include <map>
#include <set>

namespace usml {
namespace managed {

using namespace usml::threads;

/// @ingroup managed
/// @{

/**
 * Template for a thread-safe map to store and manage dynamic objects that
 * inherit from the managed_obj class. Searches for these entries using the
 * keyID field of the object to be found. Duplicate keys are not allowed. Event
 * listeners are notified when objects are added to or removed from the manager.
 * This implementation does not include commonly used map iterator functions
 * like size(), begin(), and end() because the state of the map can not be
 * guaranteed between innovations.
 *
 * @param obj_type  Type of object stored in the manager.
 */
template <typename obj_type>
class USML_DLLEXPORT manager_template {
   public:
    /// Exception thrown if keyID already exists.
    struct duplicate_key : public std::exception {
        const char* what() const throw() { return "duplicate key"; }
    };

    /**
     * Add a managed listener to this object.
     */
    void add_listener(manager_listener<obj_type>* listener) {
        write_lock_guard guard(_mutex);
        _listeners.insert(listener);
    }

    /**
     * Remove a managed listener to this object.
     */
    void remove_listener(manager_listener<obj_type>* listener) {
        write_lock_guard guard(_mutex);
        _listeners.erase(listener);
    }

    /**
     * Adds a new object to the manager. Notifies listeners after the
     * object has been added to this manager.
     *
     * @param object         Reference to this object.
     * @return                  Key used to store this object.
     * @throw duplicate_key     If keyID already exists.
     */
    typename obj_type::key_type add(typename obj_type::sptr object) {
        write_lock_guard guard(_mutex);
        if (_object_map.count(object->keyID()) > 0) throw duplicate_key();

        // add object to map
        _object_map[object->keyID()] = object;

        // notify listeners after add
        for (auto listener : _listeners) {
            listener->notify_add(object.get());
        }
        return object->keyID();
    }

    /**
     * Removes an existing object from the manager. Leaves the map unchanged if
     * ID is not in the map. Notifies listeners before the object is removed
     * from this manager.
     *
     * @param keyID  Identification used to find this object.
     * @return          False if keyID was not found.
     */
    bool remove(typename obj_type::key_type keyID) {
        write_lock_guard guard(_mutex);
        iterator iter = _object_map.find(keyID);
        if (iter == _object_map.end()) return false;

        // notify listeners before removal
        for (auto listener : _listeners) {
            listener->notify_remove(keyID);
        }

        // remove object from manager
        // delete object when shared_ptr goes out of scope
        return _object_map.erase(keyID);
    }

    /**
     * Find a specific object in the map.
     *
     * @param keyID  Identification used to find this object.
     * @return    nullptr if not found.
     */
    typename obj_type::sptr find(typename obj_type::key_type keyID) const {
        read_lock_guard guard(_mutex);
        typename obj_type::sptr object = nullptr;
        iterator iter = _object_map.find(keyID);
        if (iter != _object_map.end()) {
            object = iter->second;
        }
        return object;
    }

    /**
     * Creates a temporary list of all objects in the map.
     */
    std::list<typename obj_type::sptr> list() const {
        read_lock_guard guard(_mutex);
        std::list<typename obj_type::sptr> list;
        for (auto pair : _object_map) {
            list.push_back(pair.second);
        }
        return list;
    }

   private:
    /// Mutex for updating the structure of the map.
    mutable read_write_lock _mutex;

    /// List of active listeners.
    std::set<manager_listener<obj_type>*> _listeners;

    /// Type used to store list of shared objects.
    typedef std::map<typename obj_type::key_type, typename obj_type::sptr>
        map_type;

    /// Iterator used to search for specific objects.
    typedef typename map_type::const_iterator iterator;

    /// Stores list of mapped_types keyed by keyID.
    map_type _object_map;
};

/// @}
}  // namespace managed
}  // namespace usml
