/**
 * @example threads/test/threads_test.cc
 */

#include <bits/stdint-intn.h>
#include <cstddef>
#include <usml/threads/read_write_lock.h>
#include <usml/threads/thread_controller.h>
#include <usml/threads/thread_pool.h>
#include <usml/threads/thread_task.h>
#include <usml/ublas/randgen.h>

#include <boost/test/unit_test.hpp>
#include <boost/timer/timer.hpp>
#include <cmath>
#include <iostream>
#include <memory>

BOOST_AUTO_TEST_SUITE(threads_test)

using namespace boost::unit_test;
using namespace usml::threads;
using namespace usml::ublas;

using std::cout;
using std::endl;

//#define DEBUG_THREAD_TASK

/**
 * @ingroup threads_test
 * @{
 */

/**
 * Task to compute sqrt() a random number of times in a separate thread.
 * Using a random number of sqrt() calculations allows each task to take
 * a random amount of time to complete.  Also demonstrates use of a
 * read_write_lock to lock multiple properties at once.
 */
class sqrt_task : public thread_task {
   private:
    /** Number of times to compute sqrt(). */
    size_t _num_calcs;

    /** Result of the final sqrt() operation. */
    double _result;

    /** Set to true when task complete. */
    bool _done;

    /** Mutex to lock multiple properties at once. */
    read_write_lock _lock;

   public:
    /**
     * Constructor defines number of times to compute sqrt()
     * and prints a debugging message.  Each calculation computes
     * sqrt() one million times.  This takes about 1 millisec of
     * execution time on Sean's laptop.
     *
     * @param max_calcs  Maximum number of sqrt() calculations for this task,
     *                   The actual number calcs will be a random number
     *                   in the range [0,max_calcs] with a uniform
     *                   distribution.
     */
    sqrt_task(size_t max_calcs) {
        randgen rand;
        _num_calcs = (size_t) (rand.uniform() * (double) max_calcs);
        _result = -1.0;
        _done = false;
        #ifdef DEBUG_THREAD_TASK
            cout << id() << " task:   created with num_calcs=" << _num_calcs << endl;
        #endif
    }

    /**
     * Destructor prints a debugging message.
     */
    ~sqrt_task() override { // NOLINT modernize-use-equals-default
        #ifdef DEBUG_THREAD_TASK
            cout << id() << " task:   destroyed" << endl;
        #endif
    }

    /**
     * Computes sqrt() in a separate thread.
     * Exits early if abort() invoked.
     */
    void run() override {
        // check to see if task has already been aborted

        if (_abort) {
            #ifdef DEBUG_THREAD_TASK
                cout << id() << " task:   *** aborted before execution ***" << endl;
            #endif
            return;
        }

        // start timer and lock access

        boost::timer::auto_cpu_timer timer(6, "%w secs\n");
        write_lock_guard guard(_lock);

        // computes sqrt() one million times per iteration

        #ifdef DEBUG_THREAD_TASK
            cout << id() << " task:   run " << _num_calcs << " loops" << endl;
        #endif
        for (size_t n = 0; n < _num_calcs; ++n) {
            if (_abort) {
                #ifdef DEBUG_THREAD_TASK
                    cout << id() << " task:   ### aborted during execution ###" << endl;
                #endif
                _done = true;
                return;
            }
            for (size_t m = 0; m < 1000000; ++m) {
                _result = sqrt((double)n);
            }
        }
        cout << id() << " task:   computed result=" << _result << " in ";

        // mark result as complete
        _done = true;
    }

    /**
     * Result of the final sqrt() operation.
     * Tests the ability of the creator to extract results via shared pointer.
     */
    auto result() {
        read_lock_guard guard(_lock);
        return _result;
    }

    /**
     * Gets the status of the task
     * Returns true when task completed.
     */
    auto done() {
        read_lock_guard guard(_lock);
        return _done;
    }

    /**
     * Sets the task completed flag
     * @param bool value to set the done attribute.
     */
    void done(bool done) {
        write_lock_guard guard(_lock);
        _done = done;
    }
};

/**
 * Executes a series of tasks using thread_controller.
 * Cancels a specified percentage of tasks.
 * Waits random amount of time between tasks.
 * Waits for last task to complete, then prints its result.
 */
class sqrt_task_tester {
   private:
    /** Number of tasks to create. */
    size_t _num_tasks;

    /** Maximum number of calculations per task. */
    size_t _max_calcs;

    /** Maximum amount of time to wait before starting new task. */
    double _max_wait;

    /** Percentage of tasks to cancel. */
    double _percent_cancel;

   public:
    /**
     * Defines test scenario.
     *
     * @param num_tasks      Number of tasks to create, defines scope of test.
     * @param max_calcs      Maximum number of calculations per task, defines scope of test.
     * @param max_wait       Maximum amount of time to wait before starting new task (sec).
     * @param percent_cancel Percentage of tasks to cancel, range=[0,1]
     */
    sqrt_task_tester(size_t num_tasks, size_t max_calcs, double max_wait,
                     double percent_cancel)
        : _num_tasks(num_tasks),
          _max_calcs(max_calcs),
          _max_wait(max_wait),
          _percent_cancel(percent_cancel) {}

    /**
     * Executes each task using thread_controller.
     * Waits a random amount of time before starting next task.
     * Cancels a specified percentage of tasks.
     * Waits for last task to complete, then prints its result.
     */
    void run() {
        std::shared_ptr<sqrt_task> task;
        for (size_t n = 0; n < _num_tasks; ++n) {
            // test ability to add tasks to thread pool

            task = std::make_shared<sqrt_task>(_max_calcs);
            thread_controller::instance()->run(task);

            // test ability to cancel tasks
            // before, during, and after execution by thread pool

            if (n < _num_tasks - 1) {
                random_wait();
                randgen rand;
                if (rand.uniform() < _percent_cancel) {
                    #ifdef DEBUG_THREAD_TASK
                        cout << task->id() << " tester: %%% abort task %%%" << endl;
                    #endif
                    task->abort();
                    task->done(true);
                }
            }
        }

        // test ability to monitor a locked operation
        // this should block while task's run() method is being executed

        cout << task->id() << " tester: wait until done" << endl;
        {
            #ifdef DEBUG_THREAD_TASK
                boost::timer::auto_cpu_timer timer(3, "%w secs\n");
            #endif
            while (!task->done()) {
            	thread_task::sleep();
            }
            #ifdef DEBUG_THREAD_TASK
                cout << task->id() << " tester: waited for ";
            #endif
        }

        // test ability to extract results via shared pointer
        // a bit redundant with prior uses of pointer, but nice example

        cout << task->id()
             << " tester: completed with result=" << task->result() << endl;

        // Wait here until all tasks complete
        thread_task::wait();
    };

   private:
    /**
     * Generate random wait between tasks.  The wait time is a
     * uniformly distributed random value in the range
     * [0,_max_wait] seconds.
     */
    void random_wait() const {
        randgen rand;
        auto msec = (int64_t)(1000.0 * _max_wait * rand.uniform());
        thread_task::sleep(msec);
    }
};

/**
 * Test the ability of thread_controller to schedule computationally intense
 * tasks across cores on the computer.  Does not include any automated
 * Boost checks.
 *
 * Tests ability to:
 *   - add tasks to thread pool
 *   - cancel tasks in thread pool
 *   - monitor a locked operation
 *   - extract results via shared pointer
 *
 * Setup:
 *  - 10,30   	Number of tasks to create, add tasks to increase scope of test
 *  - 100,1000 	Maximum number of calcs per task, avg execution time of 0.5 sec.
 *  - 0.25 		Maximum amount of time to wait before starting new task (sec).
 *  - 0.25 		Percentage of tasks to cancel
 *
 * This test passes if:
 *   - it runs successfully to conclusion
 *   - all tasks are created and destroyed
 *   - processing is evenly distributed across all cores
 */
BOOST_AUTO_TEST_CASE(thread_controller_test) {
    cout << "=== threads_test: thread_controller_test ===" << endl;
    #ifdef DEBUG_THREAD_TASK
        sqrt_task_tester(30, 1000, 0.25, 0.25).run();
        thread_controller::reset();
    #else
        sqrt_task_tester(10, 100, 0.25, 0.25).run();
        thread_controller::reset();
    #endif
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
