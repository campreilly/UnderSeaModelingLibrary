/**
 * @file platform_manager.cc
 * Singleton container for all platforms in the simulation.
 */

#include <usml/platforms/platform_manager.h>

#include <memory>

using namespace usml::platforms;

/** Initializes empty reference to singleton. */
std::unique_ptr<platform_manager> platform_manager::_instance;

/** Initializes mutex for singleton construction. */
read_write_lock platform_manager::_mutex;

/**
 * Singleton constructor, implemented using double-checked locking pattern.
 */
platform_manager* platform_manager::instance() {
    platform_manager* manager = _instance.get();
    if (manager == nullptr) {
        write_lock_guard guard(_mutex);
        manager = _instance.get();
        if (manager == nullptr) {
            manager = new platform_manager();
            manager->_max_key = 0;
            _instance.reset(manager);
        }
    }
    return manager;
}

/**
 * Removes all platforms from the manager and destroys it.
 */
void platform_manager::reset() {
    write_lock_guard guard(_mutex);
    _instance.reset();
}

/**
 * Adds a new platform to the manager.
 */
typename platform_model::key_type platform_manager::add(
    const typename platform_model::sptr& platform) {
    write_lock_guard guard(_mutex);
    if (platform->keyID() == 0) {  // auto create new key id
        platform->keyID(++_max_key);
    } else {
        _max_key = max(_max_key, platform->keyID());
    }
    return manager_template<platform_model>::add(platform);
}
