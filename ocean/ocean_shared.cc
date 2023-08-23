/**
 * @file ocean_shared.cc
 * Shares an ocean singleton across multiple threads.
 */
#include <usml/ocean/ocean_shared.h>

#include <utility>

using namespace usml::ocean;

/** Shared reference to the current ocean. */
ocean_model::csptr ocean_shared::_current = nullptr;

/** Locks singleton while ocean is being changed. */
read_write_lock ocean_shared::_mutex;

/**
 * Pass a shared reference of current ocean back to client.
 */
ocean_model::csptr ocean_shared::current() {
    read_lock_guard guard(ocean_shared::_mutex);
    return _current;
}

/**
 * Update shared ocean with new data.
 */
void ocean_shared::update(ocean_model::csptr ocean) {
    write_lock_guard guard(ocean_shared::_mutex);
    _current = std::move(ocean);
}

/**
 * Reset the shared ocean to empty.
 */
void ocean_shared::reset() {
    write_lock_guard guard(ocean_shared::_mutex);
    _current.reset();
}
