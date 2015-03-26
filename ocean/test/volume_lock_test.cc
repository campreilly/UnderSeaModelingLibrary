/**
 * @example ocean/test/volume_lock_test.cc
 */
#include <boost/thread.hpp>
#include <boost/test/unit_test.hpp>

#include <usml/ublas/randgen.h>
#include <usml/netcdf/netcdf_files.h>
#include <usml/ocean/ocean.h>


BOOST_AUTO_TEST_SUITE(volume_lock_test)

using namespace boost::unit_test;
using namespace usml::netcdf;
using namespace usml::ocean;

/**
 * @ingroup ocean_test
 * @{
 */

class tester_base
{
    public:

        /** Constructor */
        tester_base() : _volume(NULL) {
        }

        /** Destructor */
        virtual ~tester_base() {
            delete _volume;
        }

        /**
         * Generate random value between 0.1 seconds to 1.0 seconds
         * and wait that amount of time.
         */
        void random_wait()
        {
            double seed = randgen::uniform();
            if (seed < 0.1) seed = 0.1;
            int msec = seed * 1000;
            if (msec > 1000 ) msec = 1000;
            boost::this_thread::sleep(boost::posix_time::milliseconds(msec));
        }

        /**
         * This is the method which gets called when the thread is instantiated.
         */
        void run()
        {
           for (int i = 0; i < 5; ++i)
           {
               // Randomly wait from 0.1 seconds to 1.0 seconds
               random_wait();
               // Run the overloaded virtual test method.
               test();
               // Print out the thread id and for loop id for output comparison.
               std::cout << "Thread " << boost::this_thread::get_id() << ": " << i << std::endl;
           }
        }

        /** Pure virtual method which must be overloaded in child classes. */
        virtual void test() = 0;

    protected:

        /** The volume_lock instance used by all threads. */
        usml::ocean::volume_lock* _volume;
};

/**
 * Creates a single instance of a volume_lock object that is accessed by
 * multiple threads.
 */
class tester : public tester_base
{
    public:

    /**
     * Creates a locked version of a flat volume scattering layer at a
     * depth of 10 meters with a constant scattering strength of -27 dB.
     */
    void setup()
    {
        volume_model* volume = new volume_flat(10.0) ;
        volume->scattering( new scattering_constant(-27.0) ) ;
        _volume = new volume_lock(volume);
    }

    /**
     * Test the ability to extract depth, thickness, and scattering information
     * from the share volume.
     */
    void test() {

        // test extraction of depth and thickness

        wposition points(1, 1);
        matrix<double> rho(1, 1);
        matrix<double> thickness(1, 1);
        _volume->depth(points, &rho, &thickness);
        BOOST_CHECK_CLOSE(rho(0, 0), wposition::earth_radius - 10.0, 1e-6);

        // test extraction of scattering strength

        wposition1 points1(1, 1);
        seq_linear freq(10.0, 1.0, 1);
        vector<double> result(1);
        _volume->scattering(points1, freq, 0.0, 0.0, 0.0, 0.0, &result);
        BOOST_CHECK_CLOSE(result(0), pow(10.0,-2.7), 1e-6);
    }

}; // end tester class

/**
 * Test the basic features of the volume_lock class using a constant volume model
 * and attenuation with a random wait between multiple "test" method calls.
 * The test portion is a replica of the volume_test/constant_volume_test.
 * Generate errors if values differ by more that 1E-6 percent, or SIG Fault on thread error.
 * When executed the output should show intertwining between the threads.
 */
BOOST_AUTO_TEST_CASE( volume_lock_test ) {

    cout << "=== volume_lock_test: volume_lock_test ===" << endl;
    try {

        // Instantiate a tester class defined above.
        // tester class is a tester_base class that
        // overloads the "test" method.
        tester test;

        // Setup initial conditions
        test.setup();

        // Create thread group to hold threads
        boost::thread_group tgroup;

        // Start two threads each one running the "test" method five times
        // Using thread group create a thread which executes the "run" method in tester_base
        // Start first thread
        tgroup.create_thread(boost::bind(&tester::run, &test));

        // Wait 60 msec before starting next thread
        boost::this_thread::sleep(boost::posix_time::milliseconds(60));

        // Start second thread
        tgroup.create_thread(boost::bind(&tester::run, &test));

        // Wait for threads to finish then delete them.
        tgroup.join_all();

    } catch (std::exception* except) {
        BOOST_ERROR(except->what());
    }
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
