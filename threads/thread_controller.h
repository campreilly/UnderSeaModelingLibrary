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

private:

    /**
     * Number of threads used to create thread_pool.
     * Defaults to the number of cores on this machine.
     */
    static unsigned _num_threads ;

    /**
     * Reference to the thread_pool owned by this singleton.
     */
    static thread_pool* _instance ;

    /** Mutex to lock multiple properties at once. */
    static read_write_lock _lock ;

    /**
     * Hide constructors to prevent incorrect use of singleton.
     */
    thread_controller() {}

    /**
     * Hide Destructor to prevent incorrect use of singleton.
     */
    ~thread_controller() {}

public:

    /**
     * Provides a reference to the thread_pool owned by this singleton.
     * Constructs the object on the first time that this method is called.
     */
    static thread_pool* instance();

    /**
     * Singleton Destructor to delete static _instance.
     */
    static void destroy();

};

/// @}
} // end of namespace threads
} // end of namespace usml

