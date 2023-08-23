/**
 * @file thread_pool.cc
 * A thread pool for executing tasks in a separate thread.
 */

#include <usml/threads/thread_controller.h>
#include <usml/threads/thread_pool.h>

using namespace usml::threads;

/**
 * Number of threads used to create thread_pool.
 * Defaults to the number of cores on this machine.
 */
unsigned thread_controller::_num_threads = std::thread::hardware_concurrency();

/// Reference to the thread_pool owned by this singleton.
std::unique_ptr<thread_pool> thread_controller::_instance;

/// Mutex to lock creation of instance.
read_write_lock thread_controller::_instance_mutex;

/**
 * Provides a reference to the thread_pool owned by this singleton.
 * Constructs the object on the first time that this method is called.
 */
thread_pool* thread_controller::instance() {
    thread_pool* pool = _instance.get();
    if (pool == nullptr) {
        write_lock_guard guard(_instance_mutex);
        pool = _instance.get();
        if (pool == nullptr) {
            pool = new thread_pool(_num_threads);
            _instance.reset(pool);
        }
    }
    return pool;
}

/**
 * Reset the thread_controller to empty.
 */
void thread_controller::reset(unsigned num_threads) {
    write_lock_guard guard(_instance_mutex);
    _num_threads = num_threads;
    _instance.reset();
}
