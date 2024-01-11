/**
 * @file update_notifier.h
 * Stores list of managed_object listeners and distributes updates.
 */
#pragma once

#include <usml/managed/update_listener.h>
#include <usml/usml_config.h>

#include <set>

namespace usml {
namespace managed {

/// @ingroup managed
/// @{

/**
 * Stores list of pointers to managed_object listeners and distributes updates.
 * Sub-classes of managed_obj can also inherit from this class.
 *
 * @param obj_type  Type of object stored in the manager.
 */
template <typename obj_type>
class USML_DECLSPEC update_notifier {
   public:
    using listener_type = update_listener<obj_type>;

    /// Virtual destructor.
    virtual ~update_notifier() {}

    /**
     * Add an update listener to this object.
     */
    void add_listener(listener_type* listener) { _listeners.insert(listener); }

    /**
     * Remove an update listener to this object.
     */
    void remove_listener(listener_type* listener) {
        _listeners.erase(listener);
    }

    /**
     * Notify listeners that an object has been updated.
     *
     * @param object    Reference to the object that has been updated.
     */
    virtual void notify_update(const obj_type* object) const {
        for (auto listener : _listeners) {
            listener->notify_update(object);
        }
    }

   private:
    /// List of active managed listeners.
    std::set<listener_type*> _listeners;
};

/// @}
}  // namespace managed
}  // namespace usml
