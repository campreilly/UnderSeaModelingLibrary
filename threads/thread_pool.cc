/**
 * @file thread_pool.cc
 * A thread pool for executing tasks in a separate thread.
 */

#include <usml/threads/thread_pool.h>

#include <atomic>
#include <cassert>
#include <chrono>
#include <iostream>
#include <memory>
#include <vector>

using namespace usml::threads;
using namespace std::chrono_literals;

/**
 * Creates a new thread pool with a specific number of threads.
 *
 */
thread_pool::thread_pool(unsigned num_threads) {
    assert(num_threads != 0);

    // Use a lambda expression to create an infinite loop that
    // checks for (and then executes) new entries in the _task_queue
    // until _running is false.

    auto worker = [this] {
        while (this->_running) {
            // find the next task in the queue

            thread_task::ref task;
            {
                write_lock_guard guard(_task_mutex);
                if (!_task_queue.empty()) {
                    task = _task_queue.front();
                    _task_queue.pop();
                }
            }

            // run this task or wait for a short time

            if (task != nullptr) {
                task->start();
            } else {
                // std::this_thread::sleep_for(std::chrono::milliseconds(1));
                std::this_thread::sleep_for(1ms);
            }
        }
    };

    // create a list of threads using this worker

    for (unsigned n = 0; n < num_threads; ++n) {
        _thread_list.emplace_back(worker);
    }
}

/**
 * Stop the scheduler and terminate the threads used to execute tasks.
 */
thread_pool::~thread_pool() {
    write_lock_guard guard(_task_mutex);
    this->_running = false;
    for (auto& thread : _thread_list) {
        thread.join();
    }
}

/**
 * Adds a task to the scheduler.
 */
void thread_pool::run(const thread_task::ref& task) {
    write_lock_guard guard(_task_mutex);
    _task_queue.push(task);
}
