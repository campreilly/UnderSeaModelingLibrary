/**
 * @file thread_pool.h
 * A thread pool for executing tasks in a separate thread.
 */
#pragma once

#include <usml/threads/thread_task.h>
#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>

namespace usml {
namespace threads {

/// @ingroup threads
/// @{

/**
 * A thread pool for executing tasks in a separate thread.  This scheme
 * allows the developer to limit the number of tasks running
 * simultaneously on a specific computer. Uses Boost's Asynchronous I/O
 * library (ASIO) to schedule the tasks.
 *
 * @xref ASIO Recipe for a thread pool for executing arbitrary tasks,
 *       http://think-async.com/Asio/Recipes
 */
class USML_DECLSPEC thread_pool {

private:

    /** ASIO service that queues tasks for execution. */
    boost::asio::io_service _scheduler;

    /** ASIO service that runs the scheduler. */
    boost::asio::io_service::work _scheduler_work;

    /** Group of threads used by the scheduler to execute tasks. */
    boost::thread_group _thread_group;

public:

    /**
     * Creates a new thread pool with a specific number of threads.
     *
     * @param num_threads   Number of threads used to execute tasks.
     */
    thread_pool( size_t num_threads ) ;

    /**
     * Stop the scheduler and terminate the threads used to execute tasks.
     */
    ~thread_pool() ;

    /**
     * Adds a task to the scheduler.
     * This allows the calling program to invoke the abort() method,
     * on the shared reference, without fear that the scheduler has already
     * disposed of the task object.  The task object is deleted when both the
     * calling program and the scheduler have de-referenced the shared object.
     *
     * @param task      Shared pointer to the task to be executed
     */
    void run( thread_task::reference task ) ;
};

/// @}
} // end of namespace threads
} // end of namespace usml

