/**
 * @file read_write_lock.h
 * Readers–writer lock implementation using Boost shared_mutex
 */
#pragma once

#include <usml/usml_config.h>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

namespace usml {
namespace threads {

using boost::shared_mutex ;
using boost::shared_lock ;
using boost::unique_lock ;

/// @ingroup threads
/// @{

/**
 * The shared mutex type used by read_lock_guard and write_lock_guard.
 * Together, these classes provide a simple mechanism for implementing
 * a "multiple read/single write" lock internal to class methods.  The goal is
 * to create a scheme that blocks during writing, but allows concurrent access
 * by many threads when just reading.
 *
 * The example below shows these types being used in concert to control
 * multi-threaded access to set/get accessors on a simple class. During
 * writing, the class uses the #write_lock_guard to gain exclusive access
 * to the mutex that locks this whole object.  During reading, #read_lock_guard
 * establishes a shared lock.  Multiple read_lock_guard locks can be active
 * without blocking each other. But, the write_lock_guard block with
 * all other read_lock_guard and write_lock_guard locks.  Both types of
 * guard unlock when the guard object goes out of scope.  Additional
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
 * Defining these as typedefs allow us to easily migrate from
 * using std:: threads and locks when shared locks become available
 * in the C++14 standard.
 *  Notice to Mariners:
 *       The following will compile even if the _mutex is not defined:
 *          write_lock_guard(_mutex);
 *     The C++ parser sees:
 *              type  write_lock_guard
 *              name  _mutex()  default constructor
 *     Parser allows "name" inside () and type "write_lock_guard" has an
 *     empty default constructor that does not require ().
 *
 * @xref "Readers–writer lock" article on Wikipedia, the free encyclopedia,
 * http://en.wikipedia.org/wiki/Readers-writer_lock
 */
typedef shared_mutex read_write_lock;

/**
 * Acquires a shared lock on a #read_write_lock.  Used as a preamble to
 * routines that retrieve data from a shared object. Retrieval routines
 * that use this lock should be defined as const operations.
 */
typedef shared_lock< read_write_lock > read_lock_guard;

/**
 * Acquires a unique lock on a #read_write_lock.  Used as a preamble to
 * routines that update data from a shared object.  Can be used on any
 * operation not defined as const.
 */
typedef unique_lock< read_write_lock > write_lock_guard;

/// @}
} // end of namespace threads
} // end of namespace usml
