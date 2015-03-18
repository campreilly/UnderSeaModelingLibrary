/**
 * @example threads/test/threads_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <boost/progress.hpp>
#include <usml/ublas/ublas.h>
#include <usml/threads/threads.h>

BOOST_AUTO_TEST_SUITE(threads_test)

using namespace boost::unit_test;
using namespace usml::threads;
using namespace usml::ublas;

/**
 * @ingroup ocean_test
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
    size_t _num_calcs ;

    /** Result of the final sqrt() operation. */
    double _result ;

    /** Set to true when task complete. */
    bool _done ;

    /** Mutex to lock multiple properties at once. */
    read_write_lock _lock ;

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
    sqrt_task( size_t max_calcs ) {
        _num_calcs = (size_t) ( randgen::uniform() * max_calcs ) ;
        _result = -1.0 ;
        _done = false ;
        cout << id() << " task:   created with num_calcs=" << _num_calcs << endl ;
    }

    /**
     * Destructor prints a debugging message.
     */
    virtual ~sqrt_task() {
        cout<< id() << " task:   destroyed" << endl ;
    }

    /**
     * Computes sqrt() in a separate thread.
     * Exits early if abort() invoked.
     */
    virtual void run() {

        // check to see if task has already been aborted

        if ( _abort ) {
            cout << id() << " task:   *** aborted before execution ***" << endl ;
            return ;
        }

        // start timer and lock access

        boost::timer timer ;
        write_lock_guard guard( _lock ) ;

        // computes sqrt() one million times per iteration

        cout << id() << " task:   run" << endl ;
        for ( size_t n=0 ; n < _num_calcs ; ++n ) {
            if ( _abort ) {
                cout << id() << " task:   ### aborted during execution ###" << endl ;
                return ;
            }
            for ( size_t m=0; m < 1000000 ; ++m ) {
                _result = sqrt( (double) n ) ;
            }
        }
        cout << id() << " task:   computed result=" << _result 
             << " in " << timer.elapsed() << " sec" << endl ;

        // mark result as complete

        _done = true ;
    }

    /** 
     * Result of the final sqrt() operation. 
     * Tests the ability of the creator to extract results via shared pointer.
     */
    double result() {
        read_lock_guard guard(_lock);
        return _result ; 
    }

    /** 
     * Set to true when task complete. 
     * Tests the ability of the creator to monitor a locked operation.
     */
    bool done() {
        read_lock_guard guard(_lock);
        return _done ; 
    }
} ;

/**
 * Executes a series of tasks using thread_controller.
 * Cancels a specified percentage of tasks.
 * Waits random amount of time between tasks.
 * Waits for last task to complete, then prints its result.
 */
class sqrt_task_tester {

private:

    /** Number of tasks to create. */
    size_t _num_tasks ;

    /** Maximum number of calculations per task. */
    size_t _max_calcs;

    /** Maximum amount of time to wait before starting new task. */
    double _max_wait;

    /** Percentage of tasks to cancel. */
    double _percent_cancel ;

public:

    /**
     * Defines test scenario.
     *
     * @param num_tasks         Number of tasks to create, defines scope of test.
     * @param max_calcs         Maximum number of calculations per task, defines scope of test.
     * @param max_wait          Maximum amount of time to wait before starting new task (sec).
     * @param percent_cancel    Percentage of tasks to cancel, range=[0,1]
     */
    sqrt_task_tester( size_t num_tasks, size_t max_calcs, double max_wait, double percent_cancel ) :
        _num_tasks(num_tasks),
        _max_calcs(max_calcs),
        _max_wait(max_wait),
        _percent_cancel(percent_cancel)
    {
    }

    /**
     * Executes each task using thread_controller.
     * Waits a random amount of time before starting next task.
     * Cancels a specified percentage of tasks.
     * Waits for last task to complete, then prints its result.
     */
    void run() {

        shared_ptr<sqrt_task> task ;
        for ( size_t n=0 ; n < _num_tasks ; ++n ) {

            // test ability to add tasks to thread pool

            task = shared_ptr<sqrt_task>( new sqrt_task(_max_calcs) ) ;
            thread_controller::instance()->run( task ) ;

            // test ability to cancel tasks
            // before, during, and after execution by thread pool

            if ( n < _num_tasks-1 ) {
                random_wait() ;
                if ( randgen::uniform() < _percent_cancel ) {
                    cout << task->id() << " tester: %%% abort task %%%" << endl ;
                    task->abort() ;
                }
            }
        }

        // test ability to monitor a locked operation
        // this should block while task's run() method is being executed

        cout << task->id() << " tester: wait until done" << endl ;
        {
            boost::timer timer ;
            while ( ! task->done() ) {
                boost::this_thread::sleep(boost::posix_time::milliseconds(1));
            }
            cout << task->id() << " tester: waited for " << timer.elapsed() << " secs" << endl ;
        }

        // test ability to extract results via shared pointer
        // a bit redundant with prior uses of pointer, but nice example

        cout << task->id() << " tester: completed with result=" << task->result() << endl ;
    } ;

private:

    /** 
     * Generate random wait between tasks.  The wait time is a 
     * uniformly distributed random value in the range 
     * [0,_max_wait] seconds.
     */
    void random_wait()
    {
        long msec = (long) ( 1000.0 * _max_wait * randgen::uniform() ) ;
        boost::this_thread::sleep(boost::posix_time::milliseconds(msec));
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
 *  - 30   Number of tasks to create, add tasks to increase scope of test
 *  - 1000 Maximum number of calcs per task, avg execution time of 0.5 sec.
 *  - 0.25 Maximum amount of time to wait before starting new task (sec).
 *  - 0.25 Percentage of tasks to cancel
 *
 * This test passes if:
 *   - it runs successfully to conclusion
 *   - all tasks are created and destroyed
 *   - processing is evenly distributed across all cores
 */
BOOST_AUTO_TEST_CASE( thread_controller_test ) {
    cout << "=== threads_test: thread_controller_test ===" << endl;
    randgen::seed(0); // create same results each time
    sqrt_task_tester(30,1000,0.25,0.25).run() ;
    thread_controller::destroy();
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
