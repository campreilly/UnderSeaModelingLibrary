/**
 * @file ocean_shared.cc
 * Shares an ocean singleton across multiple threads.
 */
#include <usml/ocean/ocean_shared.h>

using namespace usml::ocean ;

/** Shared reference to the current ocean. */
ocean_shared::reference ocean_shared::_current = ocean_shared::reference();

/**
 * Pass a shared reference of current ocean back to client.
 */
ocean_shared::reference ocean_shared::current() {
    read_lock_guard(_lock) ;
    return _current ;
}

/**
 * Update shared ocean with new data.
 */
void ocean_shared::update( ocean_shared::reference& ocean ) {
    write_lock_guard(_lock) ;
    _current = ocean ;
}
