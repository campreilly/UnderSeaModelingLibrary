/**
 * @file thread_pool.h
 * A thread pool for executing tasks in a separate thread.
 */
#pragma once

#include <usml/threads/read_write_lock.h>
#include <usml/threads/thread_task.h>
#include <usml/usml_config.h>

#include <atomic>
#include <queue>
#include <thread>
#include <vector>

namespace usml {
namespace threads {

/// @ingroup threads
/// @{

/**
 * A thread pool for executing tasks in a separate thread.  This scheme
 * allows the developer to limit the number of tasks running
 * simultaneously on a specific computer. It also avoids the overhead
 * associated with starting each task on its own thread.
 *
 * @xref Vorbrodt's C++ Blog: Advanced thread pool
 *       Posted on February 27, 2019 by Martin Vorbrodt
 *       https://vorbrodt.blog/2019/02/27/advanced-thread-pool/
 */
class USML_DECLSPEC thread_pool {
   public:
    /**
     * Creates a new thread pool with a specific number of threads.
     *
     * @param num_threads   Number of threads used to execute tasks.
     */
    thread_pool(unsigned num_threads = std::thread::hardware_concurrency());

    /**
     * Stop the scheduler and terminate the threads used to execute tasks.
     */
    ~thread_pool();

    /**
     * Adds a task to the scheduler.
     * This allows the calling program to invoke the abort() method,
     * on the shared reference, without fear that the scheduler has already
     * disposed of the task object. The task object is deleted when both the
     * calling program and the scheduler have de-referenced the shared object.
     *
     * @param task      Shared pointer to the task to be executed
     */
    void run(const thread_task::ref& task);

   private:
    /// List of threads that execute the tasks.
    std::vector<std::thread> _thread_list;

    /// Queue of the tasks to execute.
    std::queue<thread_task::ref> _task_queue;

    /// Mutex used to lock updates to the the task queue.
    read_write_lock _task_mutex;

    /// Flag that controls execution of thread loop.
    std::atomic<bool> _running = true;
};

/// @}
}  // end of namespace threads
}  // end of namespace usml
