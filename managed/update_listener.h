/**
 * @file update_listener.h
 * Abstract listener for managed object changes,
 */
#pragma once

#include <usml/usml_config.h>

namespace usml {
namespace managed {

/// @ingroup managed
/// @{

/**
 * Abstract listener for managed object changes,
 *
 * @param obj_type  Type of object stored in the manager.
 */
template <typename obj_type>
class USML_DECLSPEC update_listener {
   public:
    /// Virtual destructor.
    virtual ~update_listener() {}

    /**
     * Notify listeners of changes to object.
     *
     * @param object  Pointer to updated object.
     */
    virtual void notify_update(const obj_type* object) = 0;
};

/// @}
}  // namespace managed
}  // namespace usml
