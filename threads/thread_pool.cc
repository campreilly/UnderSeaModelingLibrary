/**
 * @file thread_pool.cc
 * A thread pool for executing tasks in a separate thread.
 */
#include <usml/threads/thread_pool.h>
#include <boost/bind.hpp>

using namespace usml::threads ;

/**
 * Creates a new thread pool with a specific number of threads.
 */
thread_pool::thread_pool( size_t num_threads ) :
    _scheduler(),
    _scheduler_work(_scheduler)
{
    for (size_t n = 0; n < num_threads; ++n) {
        _thread_group.create_thread(
                boost::bind(&boost::asio::io_service::run, &_scheduler));
    }
}

/**
 * Stop the scheduler and terminate the threads used to execute tasks.
 */
thread_pool::~thread_pool() {
    _scheduler.stop();
    try {
        _thread_group.join_all();
    } catch( ... ) {
        // suppress all exceptions
    }
}

/**
 * Adds a task to the scheduler.
 */
void thread_pool::run( shared_ptr<thread_task> task ) {
    _scheduler.post( boost::bind(&thread_task::start, task) ) ;
}


