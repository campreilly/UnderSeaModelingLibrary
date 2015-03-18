/**
 * @file thread_pool.cc
 * A thread pool for executing tasks in a separate thread.
 */
#include <usml/threads/thread_controller.h>

using namespace usml::threads ;

/** Number of threads used to create thread_pool. */
unsigned thread_controller::_num_threads = boost::thread::hardware_concurrency() ;

/** Reference to the thread_pool owned by this singleton. */
thread_pool* thread_controller::_instance = NULL ;

/**
 * The _mutex for the singleton pointer.
 */
read_write_lock thread_controller::_lock;

/**
 * Singleton Constructor - Double Check Locking Pattern DCLP
 * Provides a reference to the thread_pool owned by this singleton.
 */
thread_pool* thread_controller::instance()
{
    thread_pool* tmp = _instance;
    // TODO: insert memory barrier.
    if (tmp == NULL)
    {
        write_lock_guard guard(_lock);
        tmp = _instance;
        if (tmp == NULL)
        {
            tmp = new thread_pool(_num_threads);
            // TODO: insert memory barrier
            _instance = tmp;
        }
    }
    return tmp;
}

/**
 * Singleton Destructor
 */
void thread_controller::destroy()
{
    write_lock_guard guard(_lock);
    if ( _instance != NULL ) {
        delete _instance ;
        _instance = NULL ;
    }
}
