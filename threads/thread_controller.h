/**
 * @file thread_controller.h
 * Singleton version of a thread_pool.
 */
#pragma once

#include <usml/threads/thread_pool.h>
#include <usml/threads/read_write_lock.h>
#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>

namespace usml {
namespace threads {

/// @ingroup threads
/// @{

/**
 * Singleton version of a thread_pool.
 */
class USML_DECLSPEC thread_controller {

public:

    /**
     * Provides a reference to the thread_pool owned by this singleton.
     * If this is the first time that this has been invoked, the singleton
     * is automatically constructed.  The double check locking pattern
     * is used to prevent multiple threads from simultaneously trying to
     * construct the singleton.
     *
     * @xref     Meyers, S., Alexandrescu, A.: C++ and the perils of
     *              double-checked locking. Dr. Dobbs Journal (July-August 2004)
     * @return  Reference to the thread_controller singleton.
     */
    static thread_pool* instance();

    /**
     * Reset the thread_controller singleton unique pointer to empty.
     */
    static void reset();

private:

    /**
     * Number of threads used to create thread_pool.
     * Defaults to the number of cores on this machine.
     */
    static unsigned _num_threads ;

    /**
     * Reference to the thread_pool owned by this singleton.
     */
    static unique_ptr<thread_pool> _instance ;

    /**
     * Mutex to lock creation of instance.
     */
    static read_write_lock _instance_mutex ;

    /**
     * Hide constructors to prevent incorrect use of singleton.
     */
    thread_controller() {}

    /**
     * Hide Destructor to prevent incorrect use of singleton.
     */
    ~thread_controller() {}

};

/// @}
} // end of namespace threads
} // end of namespace usml

