/**
 * @example threads/test/ocean_shared_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <boost/progress.hpp>
#include <usml/ublas/ublas.h>
#include <usml/ocean/ocean.h>
#include <usml/ocean/ocean_shared.h>

BOOST_AUTO_TEST_SUITE(ocean_shared_test)

using namespace boost::unit_test;
using namespace usml::threads;
using namespace usml::ocean;

/**
 * @ingroup ocean_test
 * @{
 */

/**
 * Testing task that creates ocean data.  The run() method builds a simole ocean
 * and uses that ocean to update the ocean_shared singleton.  In a real
 * sonar training system, the producer would have properties that controlled
 * the data extraction (example: area of interest), and these properties would be
 * used by the run() method to control the creation of the ocean.
 */
class ocean_producer : public thread_task {

public:

    /**
     * Create a new set of ocean data.
     */
    virtual void run() {
        boundary_model* surface = new boundary_lock( new boundary_flat() );
        boundary_model* bottom = new boundary_lock( new boundary_flat(1000.0) );
        profile_model* profile = new profile_lock( new profile_linear() ) ;
        ocean_model* ocean = new ocean_model( surface, bottom, profile ) ;
        cout << id() << " producer: creating ocean_model=" << ocean << endl ;
        ocean_shared::update( ocean ) ;
    }
} ;

/**
 * Testing task that uses ocean data.  Plays a role similar to the
 * one that propagates the wave_queue in the sonar training system.
 * First, the run() method gets a shared reference to the current ocean.
 * Instead of propagating a wave_queue, this test task just holds access 
 * to that ocean for a specific amount of time.  Finally, it releases the 
 * shared ocean reference.
 */
class ocean_consumer : public thread_task {

private:

    /** Amount of time to hold onto ocean data. */
    double _delay ;

public:

    /**
     * Constructor amount of time that consumer will maintain
     * reference to ocean data.
     *
     * @param delay  Amount of time to hold onto ocean data.
     */
    ocean_consumer( double delay ) : _delay( delay ) {}

    /**
     * Get ocean reference and hold access for a specific amount of time,
     * in a separate thread.
     */
    virtual void run() {
        shared_ptr<ocean_model> ocean = ocean_shared::current() ;
        cout << id() << " consumer: accessing ocean_model=" << ocean.get() << endl ;
        long msec = (long) ( 1000.0 * _delay ) ;
        boost::this_thread::sleep(boost::posix_time::milliseconds(msec));
        cout << id() << " consumer: releasing ocean_model=" << ocean.get() << endl ;
    }
} ;

/**
 * Executes a series of tasks that produce or consume ocean data.
 * Waits random amount of time between tasks.
 */
class ocean_shared_tester {

private:

    /** Number of tasks to create. */
    size_t _num_tasks ;

    /** Maximum amount of time to hold onto ocean data (sec). */
    double _max_delay;

    /** Percentage of tasks that are producers. */
    double _percent_producer ;

public:

    /**
     * Defines test scenario.
     *
     * @param num_tasks         Number of tasks to create, defines scope of test.
     * @param max_delay         Maximum amount of time to hold onto ocean data (sec).
     * @param percent_producer  Percentage of tasks that are producers, range=[0,1]
     */
    ocean_shared_tester( size_t num_tasks, double max_delay, double percent_producer ) :
        _num_tasks(num_tasks),
        _max_delay(max_delay),
        _percent_producer(percent_producer)
    {
    }

    /**
     * Executes each task using thread_controller.
     * Waits a random amount of time before starting next task.
     */
    void run() {
        for ( size_t n=0 ; n < _num_tasks ; ++n ) {

            // create a prodcuder
            if ( randgen::uniform() < _percent_producer ) {
                thread_controller::instance()->run( 
                    shared_ptr<ocean_producer>( new ocean_producer() ) ) ;

            // create a consumer
            } else {
                thread_controller::instance()->run( 
                    shared_ptr<ocean_consumer>( new ocean_consumer(
                    _max_delay * randgen::uniform() ) ) ) ;
            }

            // random delay 
            random_wait() ;
        }
    }

private:

    /** 
     * Generate random wait between tasks.  The wait time is a 
     * uniformly distributed random value in the range 
     * [0,_max_delay] seconds.
     */
    void random_wait()
    {
        long msec = (long) ( 1000.0 * _max_delay * randgen::uniform() ) ;
        boost::this_thread::sleep(boost::posix_time::milliseconds(msec));
    }
};

/**
 * Test the ability to share an ocean between threads.
 * Each iteration creates either a producer or consumer task.
 * Producer tasks create a new ocean and use it to update 
 * the ocean_share singleton.  Consumer tasks get a reference to
 * currently shared ocean, wait a random time, and then release it.
 * Tasks print the pointer value of the ocean they are working with
 * to uniquely identify each ocean.   Does not include any automated
 * Boost checks.
 *
 * Setup:
 *  - 10   Number of tasks to create, add tasks to increase scope of test
 *  - 0.50 Maximum amount of time to wait before starting new task (sec).
 *  - 0.25 Percentage of producer tasks
 *
 * Example Results:
 * <pre>
 *      Running 1 test case...
 *      === ocean_shared_test: ocean_shared_test ===
 *      0 consumer: accessing ocean_model=00000000
 *      1 consumer: accessing ocean_model=00000000
 *      0 consumer: releasing ocean_model=00000000
 *      2 producer: creating ocean_model=00561DC0
 *      1 consumer: releasing ocean_model=00000000
 *      3 consumer: accessing ocean_model=00561DC0
 *      3 consumer: releasing ocean_model=00561DC0
 *      4 consumer: accessing ocean_model=00561DC0
 *      5 producer: creating ocean_model=00561E88
 *      6 producer: creating ocean_model=00561F50
 *      4 consumer: releasing ocean_model=00561DC0
 *      7 consumer: accessing ocean_model=00561F50
 *      8 consumer: accessing ocean_model=00561F50
 *      7 consumer: releasing ocean_model=00561F50
 *      9 consumer: accessing ocean_model=00561F50
 *      9 consumer: releasing ocean_model=00561F50
 *      8 consumer: releasing ocean_model=00561F50
 * </pre>
 *
 * What happened here?
 *      - Tasks 0 and 1 try to access the share ocean, but find that 
 *        it has not yet been defined.
 *      - Task 2 creates the first instance of the ocean 00561DC0.
 *      - Tasks 3 and 4 use ocean 00561DC0.
 *      - Task 5 creates ocean 00561E88, but task 6 immediately 
 *        replaces it with ocean 00561F50.
 *      - Task 4 continues to use ocean 00561DC0 even after two other 
 *        oceans have been created.
 *      - Ocean 00561F50 is used by tasks 7-9.
 *
 * This test passes if it runs successfully to conclusion.
 */
BOOST_AUTO_TEST_CASE( random_producer ) {
    cout << "=== ocean_shared_test: random_producer ===" << endl;
    randgen::seed(0); // create same results each time
    ocean_shared_tester(10,0.5,0.25).run() ;
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
