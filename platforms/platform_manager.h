/**
 * @file platform_manager.h
 * Singleton container for all platforms in the simulation.
 */
#pragma once

#include <usml/managed/managed_obj.h>
#include <usml/managed/manager_template.h>
#include <usml/platforms/platform_model.h>
#include <usml/threads/read_write_lock.h>
#include <usml/types/seq_vector.h>
#include <usml/usml_config.h>

#include <memory>

namespace usml {
namespace platforms {

using namespace usml::threads;
using namespace usml::types;

/// @ingroup platforms
/// @{

/**
 * Singleton container for all platforms in the simulation.
 */
class USML_DECLSPEC platform_manager : public manager_template<platform_model> {
   public:
    /**
     * Singleton constructor, implemented using double-checked locking pattern.
     *
     * @return Pointer to the singleton,
     */
    static platform_manager* instance();

    /**
     * Removes all platforms from the manager and destroys it.
     */
    static void reset();

    /// Hide copy constructor to prevent incorrect use of singleton.
    platform_manager(const platform_manager&) = delete;

    /// Hide assignment operator to prevent incorrect use of singleton.
    platform_manager& operator=(const platform_manager&) = delete;

    /**
     * Adds a new platform to the manager. Creates unique keyID if one not
     * provided. Notifies listeners after the object has been added to this
     * manager. Overrides equivalent function in manager_template<>.
     *
     * @param platform	        Reference to this object.
     * @return                  Key used to store this object.
     * @throw duplicate_key     If keyID already exists.
     */
    typename platform_model::key_type add(
        const typename platform_model::sptr& platform);

    /**
     * Frequencies over which propagation is computed (Hz). Making this common
     * to all the platforms controlled by this manager avoids the problem of
     * having to compute the frequency overlap between sources and receivers.
     */
    seq_vector::csptr frequencies() const {
        read_lock_guard guard(_singleton_mutex);
        return _frequencies;
    }

    /**
     * Frequencies over which propagation is computed (Hz).
     */
    void frequencies(seq_vector::csptr freq) {
        write_lock_guard guard(_singleton_mutex);
        _frequencies = freq;
    }

   private:
    /// Reference to singleton.
    static std::unique_ptr<platform_manager> _instance;

    /// Mutex for singleton construction.
    static read_write_lock _singleton_mutex;

    /// Maximum key value that has been inserted into this manager
    platform_model::key_type _max_key;

    /// Frequencies over which propagation is computed (Hz).
    seq_vector::csptr _frequencies;

    /// Hide default constructor to prevent incorrect use of singleton.
    platform_manager() : _max_key(0) {}
};

/// @}
}  // namespace platforms
}  // namespace usml
