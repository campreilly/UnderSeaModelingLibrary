/**
 * @file managed_obj.h
 * Base class for objects held inside manager_template.
 */
#pragma once

#include <usml/usml_config.h>

#include <memory>
#include <string>

namespace usml {
namespace managed {

/// @ingroup managed
/// @{

/**
 * Base class for objects controlled by manager_template.
 * Assumes that ref_type is some type of pointer or shared pointer.
 *
 * @param K  Key type used to lookup this entry type.
 * @param T  Type of object stored in the manager.
 */
template <typename K, typename T>
class USML_DLLEXPORT managed_obj {
   public:
    /// Key type used to lookup this type of entry.
    using key_type = K;

    /// Reference to object stored in the manager.
    using sptr = std::shared_ptr<T>;

    /**
     * Initialize class members.
     *
     * @param keyID         Identification used to find this object.
     * @param description   Human readable name for this object.
     */
    managed_obj(key_type keyID, const std::string& description)
        : _keyID(keyID), _description(description) {}

    /// Identification used to find this object.
    key_type keyID() const { return _keyID; }

    /// Identification used to find this object.
    void keyID(key_type keyID) { _keyID = keyID; }

    /// Human readable name for this object.
    const std::string& description() const { return _description; }

   private:
    /// Identification used to find this object.
    key_type _keyID;

    /// Human readable name for this sensor instance.
    const std::string _description;
};

/// @}
}  // namespace managed
}  // namespace usml
