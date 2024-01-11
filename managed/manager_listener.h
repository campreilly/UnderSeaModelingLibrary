/**
 * @file manager_listener.h
 * Abstract listener for object manager changes.
 */
#pragma once

#include <usml/usml_config.h>

namespace usml {
namespace managed {

/// @ingroup managed
/// @{

/**
 * Abstract listener for object manager adds and removes.
 *
 * @param obj_type  Type of object stored in the manager.
 */
template <typename obj_type>
class USML_DECLSPEC manager_listener {
   public:
    /// Virtual destructor.
    virtual ~manager_listener() {}

    /**
     * Notification that a new object has been added to the manager.
     * Invoked after the object has been added to the manager.
     *
     * @param object            Object stored in the manager.
     * @throw duplicate_key     If keyID already exists.
     */
    virtual void notify_add(const obj_type* object) const = 0;

    /**
     * Notification that an existing object is being removed from the manager.
     * Invoked before the object has been removed from the manager.
     *
     * @param pair     ID for object stored in the manager.
     */
    virtual void notify_remove(typename obj_type::key_type pair) const {}
};

/// @}
}  // namespace managed
}  // namespace usml
