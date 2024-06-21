/**
 * @file read_write_lock.h
 * Reader–writer lock implementation using shared_mutex
 */
#pragma once

#include <mutex>
#include <shared_mutex>

namespace usml {
namespace threads {

/// @ingroup threads
/// @{

/**
 * The shared mutex type used by read_lock_guard and write_lock_guard.
 * Together, these classes provide a simple mechanism for implementing
 * a "multiple read/single write" lock internal to class methods. The goal is
 * to create a scheme that blocks during writing, but allows concurrent access
 * by many threads when just reading.
 *
 * The example below shows these types being used in concert to control
 * multi-threaded access to set/get accessors on a simple class. During
 * writing, the class uses the #write_lock_guard to gain exclusive access
 * to the mutex that locks this whole object. During reading, #read_lock_guard
 * establishes a shared lock. Multiple read_lock_guard locks can be active
 * without blocking each other. But, the write_lock_guard block with
 * all other read_lock_guard and write_lock_guard locks. Both types of
 * guard unlock when the guard object goes out of scope. Additional
 * read_write_lock objects can be added to make the locking more granular.
 *
 * <pre>
 * class Thing {
 * private:
 *    read_write_lock _mutex ;
 *    int _value ;
 * public:
 *    // constructor
 *    Thing( int v ) {
 *        _value = v ;
 *    }
 *    // Retrieve current value with locking.
 *    int value() const {
 *        read_lock_guard guard(_mutex) ;
 *        return _value ;
 *    }
 *    // Define new value with locking.
 *    void value( int v ) {
 *        write_lock_guard guard(_mutex) ;
 *        _value = v ;
 *    }
 * };
 * </pre>
 *
 * @xref "Readers–writer lock" article on Wikipedia, the free encyclopedia,
 * http://en.wikipedia.org/wiki/Readers-writer_lock
 */
typedef std::shared_mutex read_write_lock;

/**
 * Acquires a shared lock on a #read_write_lock.  Used as a preamble to
 * routines that retrieve data from a shared object. Retrieval routines
 * that use this lock should be defined as const operations.
 */
typedef std::shared_lock<read_write_lock> read_lock_guard;

/**
 * Acquires a unique lock on a #read_write_lock.  Used as a preamble to
 * routines that update data from a shared object.  Can be used on any
 * operation not defined as const.
 */
typedef std::unique_lock<read_write_lock> write_lock_guard;

/// @}
}  // end of namespace threads
}  // end of namespace usml
