/**
 * @file thread_task.cc
 * Task that executes in the thread_pool.
 */

#include <bits/exception.h>
#include <usml/threads/thread_task.h>

#include <iostream>
#include <limits>

using namespace usml::threads;
using namespace std;

/** Next identification number to be assigned to a task. */
std::atomic<std::size_t> thread_task::_id_next = 0;

/** Number of active tasks in the thread pool */
std::atomic<std::size_t> thread_task::_num_active = 0;

/**
 * Initiates a task in the thread pool.
 */
thread_task::thread_task() {
    _abort = false;
    _id = _id_next;
    if (_id_next == std::numeric_limits<std::size_t>::max()) {
        _id_next = 0;
    } else {
        ++_id_next;
    }
    ++_num_active;
}

/**
 * Initiates a task in the thread pool.
 */
void thread_task::start() {
    try {
        run();  // invoke the user's version of this task
    } catch (std::exception& ex) {
        cerr << "Uncaught exception in thread_task: " << ex.what() << endl;
    } catch (...) {
        cerr << "Uncaught exception in thread_task" << endl;
    }
    // After run is completed decrement number of active tasks counter.
    --_num_active;
}
