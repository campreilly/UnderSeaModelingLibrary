/**
 * @file ocean_shared.cc
 * Shares an ocean singleton across multiple threads.
 */

#include <usml/ocean/ocean_shared.h>

using namespace usml::ocean ;

/** Shared reference to the current ocean. */
shared_ptr<ocean_model> ocean_shared::_current = shared_ptr<ocean_model>();

/**
 * Pass a shared reference of current ocean back to client.
 */
shared_ptr<ocean_model> ocean_shared::current() {
    read_lock_guard(_lock) ;
    return _current ;
}

/**
 * Update shared ocean with new data.
 */
void ocean_shared::update( ocean_model* ocean ) {
    write_lock_guard(_lock) ;
    _current = shared_ptr<ocean_model>(ocean) ;
}
