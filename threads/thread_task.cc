/**
 * @file thread_pool.cc
 * World location in geodetic earth coordinates
 */
#include <usml/threads/thread_task.h>
//#include <exception>
#include <iostream>
#include <limits>

using namespace usml::threads ;
using namespace std;

/** Next identification number to be assigned to a task. */
size_t thread_task::_id_next = 0 ;

/** Number of active tasks in the thread pool */
size_t thread_task::_num_active = 0;

/**
 * Initiates a task in the thread pool.
 */
thread_task::thread_task() {
    _abort = false ;
    _id = _id_next ;
    ++_num_active;
    if ( _id_next == std::numeric_limits<std::size_t>::max() ) {
        _id_next = 0 ;
    } else {
        ++_id_next ;
    }
}

/**
 * Initiates a task in the thread pool.
 */
void thread_task::start() {
    try {
        run() ;   // invoke the user's version of this task
    } catch( std::exception& ex ) {
        cerr << "Uncaught exception in thread_task: " << ex.what() << endl ;
    } catch( ... ) {
        // suppress all exceptions
    }
    // After run is completed decrement number of active tasks counter.
    --_num_active;
}
