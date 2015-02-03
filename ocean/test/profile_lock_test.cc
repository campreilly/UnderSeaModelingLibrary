/**
 * @example ocean/test/profile_lock_test.cc
 */
#include <boost/thread.hpp>
#include <boost/test/unit_test.hpp>

#include <usml/ublas/randgen.h>
#include <usml/netcdf/netcdf_files.h>
#include <usml/ocean/ocean.h>


BOOST_AUTO_TEST_SUITE(profile_lock_test)

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

        // Constructor
        tester_base() :
            _lockableProfile(NULL),
            _temp(NULL)
        {
            //cout << "tester_base constructor" << endl;
        }

        // Destructor
        virtual ~tester_base()
        {
            //cout << "tester_base destructor" << endl;
            delete _lockableProfile;
            delete _temp;
        }

        // Generate random value between 0.1 seconds to 1.0 seconds
        // and wait that amount of time.
        void random_wait()
        {
            double seed = randgen::uniform();
            if (seed < 0.1) seed = 0.1;
            int msec = seed * 1000;
            if (msec > 1000 ) msec = 1000;
            boost::this_thread::sleep(boost::posix_time::milliseconds(msec));
        }

        // This is the method which gets called when the thread is instantiated.
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

        // Pure virtual method which must be overloaded in child classes.
        virtual void test() = 0;

    protected:

        // The profile_lock instance used by all threads.
        usml::ocean::profile_lock* _lockableProfile;

        // The temperature values from the netcdf file
        // Used for comparison in "test" method.
        usml::netcdf::netcdf_woa* _temp;
};

/**
 * linear_tester class the inherits the tester_base class above and is used
 * to create a single instance of a profile_lock that is accessed by multiple threads.
 * The test method is a replica of the profile_test/constant_profile_test.
 */
class linear_tester : public tester_base
{
    public:

    // The setup method is is used to create a profile_linear
    // instance that is passed into the constructor of profile_lock.
    // An attenuation_model is created set via the attenuation method.
    void setup()
    {
        // liner profile
        attenuation_model* attn = new attenuation_constant(5.0);
        profile_linear* model = new profile_linear();
        model->attenuation(attn);
        _lockableProfile = new profile_lock(model);
    }

    // This method overloads the pure virtual "test" method in the tester_base.
    // It tests the _attenuationMutex inside the profile_lock via calls to attenuation()
    // from multiple threads.
    // The test method is a replica of the profile_test/constant_profile_test.
    void test()
    {
        // simple values for points and speed
        wposition points(1, 1);
        matrix<double> speed(1, 1);
        wvector gradient(1, 1);

        _lockableProfile->sound_speed(points, &speed, &gradient);

        // check the answer
        BOOST_CHECK_CLOSE(speed(0, 0), 1500.0, 1e-6);

        // setup for attenuation check
        seq_linear freq(10.0,1.0,1);
        scalar_matrix<double> distance(1,1,1);
        matrix< vector<double> > result(1,1);
        result(0,0) = scalar_vector<double> (1,1);

        _lockableProfile->attenuation(points, freq, distance, &result);

        // check the answer
        BOOST_CHECK_CLOSE((result(0, 0))[0], 50.0, 1e-6);
    }

}; // end linear_tester class

/**
 * mackenzie_tester class the inherits the tester_base class above and is used
 * to create a single instance of a profile_lock that is accessed by multiple threads.
 * The test method is a replica of the profile_test/compute_mackenzie_test.
 */
class mackenzie_tester : public tester_base
{
    public:

    // The setup method is is used to create a profile_grid
    // instance that is passed into the constructor of profile_lock.
    // An attenuation_model is created and passed into the static
    // data_grid_mackenzie::construct method.
    void setup()
    {
        int month = 6;
        wposition::compute_earth_radius((18.5 + 22.5) / 2.0);

        // load temperature & salinity data from World Ocean Atlas
        _temp = new netcdf_woa(
        USML_DATA_DIR "/woa09/temperature_seasonal_1deg.nc",
        USML_DATA_DIR "/woa09/temperature_monthly_1deg.nc",
            month, 18.5, 22.5, 200.5, 205.5 ) ;
        netcdf_woa* temperature = new netcdf_woa(
        USML_DATA_DIR "/woa09/temperature_seasonal_1deg.nc",
        USML_DATA_DIR "/woa09/temperature_monthly_1deg.nc",
            month, 18.5, 22.5, 200.5, 205.5 ) ;
        netcdf_woa* salinity = new netcdf_woa(
        USML_DATA_DIR "/woa09/salinity_seasonal_1deg.nc",
        USML_DATA_DIR "/woa09/salinity_monthly_1deg.nc",
            month, 18.5, 22.5, 200.5, 205.5 ) ;

        // setup attenuation
        attenuation_model* attn = new attenuation_constant(5.0);

        // compute sound speed
        profile_grid<double,3>* profile = new profile_grid<double,3>(
            data_grid_mackenzie::construct(temperature, salinity), attn) ;

        // Instance of lockable
        _lockableProfile = new profile_lock(profile);
    }

    // This method overloads the pure virtual "test" method in the tester_base.
    // It tests the _sound_speedMutex and attenuationMutex inside the profile_lock via calls to
    // sound_speed() and attenuation() from multiple threads.
    // The test method is a replica of the profile_test/compute_mackenzie_test.
    virtual void test()
    {
        size_t index[3];
        index[1] = 0;
        index[2] = 0;

        matrix<double> speed(1, 1);
        wposition location(1, 1);
        location.latitude(0, 0, 18.5);
        location.longitude(0, 0, 200.5);
        wvector gradient(1, 1);

        for (size_t d = 0; d < _temp->axis(0)->size(); ++d)
        {
            index[0] = d;
            location.rho(0, 0, (*_temp->axis(0))(d));

            _lockableProfile->sound_speed(location, &speed, &gradient);

            // compare to UK National Physical Laboratory software

            switch (d) {
                case 0:     // depth=0 temp=25.8543 sal=34.6954
                    BOOST_CHECK_CLOSE( speed(0, 0), 1535.9781, 1e-3 );
                    break;
                case 18:    // depth=1000 temp=4.3149 sal=34.5221
                    BOOST_CHECK_CLOSE( speed(0, 0), 1483.6464, 1e-3 );
                    break;
                case 32:    // depth=5500 temp=1.50817 sal=34.7001
                    BOOST_CHECK_CLOSE( speed(0, 0), 1549.90469, 1e-3 );
                    break;
                default:
                    break;
            }
        }

        // setup for attenuation check
       seq_linear freq(10.0,1.0,1);
       scalar_matrix<double> distance(1,1,1);
       matrix< vector<double> > result(1,1);
       result(0,0) = scalar_vector<double> (1,1);

       _lockableProfile->attenuation(location, freq, distance, &result);

       // check the answer
       BOOST_CHECK_CLOSE((result(0, 0))[0], 50.0, 1e-6);
    }

}; // end mackinzie_tester class


/**
 * Test the basic features of the profile_lock class using a constant profile model
 * and attenuation with a random wait between multiple "test" method calls.
 * The test portion is a replica of the profile_test/constant_profile_test.
 * Generate errors if values differ by more that 1E-6 percent, or SIG Fault on thread error.
 * When executed the output should show intertwining between the threads.
 */
BOOST_AUTO_TEST_CASE( linear_profile_lock_test ) {

    cout << "=== profile_lock_test: linear_profile_lock_test ===" << endl;
    try {

        // Instantiate a linear_tester class defined above.
        // linear_tester class is a tester_base class that
        // overloads the "test" method.
        linear_tester linear;

        // Setup initial conditions
        linear.setup();

        // Create thread group to hold threads
        boost::thread_group tgroup;

        // Start two threads each one running the "test" method five times
        // Using thread group create a thread which executes the "run" method in tester_base
        // Start first thread
        tgroup.create_thread(boost::bind(&linear_tester::run, &linear));

        // Wait 60 msec before starting next thread
        boost::this_thread::sleep(boost::posix_time::milliseconds(60));

        // Start second thread
        tgroup.create_thread(boost::bind(&linear_tester::run, &linear));

        // Wait for threads to finish then delete them.
        tgroup.join_all();

    } catch (std::exception* except) {
        BOOST_ERROR(except->what());
    }
}

/**
 * This test reproduces the mackenzie_profile_test found in profile_test
 * only accessing the profile_model with a data_grid via profile_lock class.
 * A random wait is used between the start of multiple "test" calls from multiple threads.
 * Generate errors if values differ by more that 1E-3 percent or SIG Fault on thread error.
 * When executed the output should show intertwining between the threads.
 */
BOOST_AUTO_TEST_CASE( mackenzie_profile_lock_test ) {

    cout << "=== profile_lock_test: mackenzie_profile_lock_test ===" << endl;
    try {

        // Instantiate a mackenzie_tester class defined above.
        // mackenzie_tester class is a tester_base class that
        // overloads the "test" method.
        mackenzie_tester mackenzie;

        // Setup initial conditions.
        mackenzie.setup();

        // Create thread group to hold threads
       boost::thread_group tgroup;

       // Start two threads each one running the "test" method five times
       // Using thread group create a thread which executes the "run" method in tester_base
       // Start first thread
       tgroup.create_thread(boost::bind(&mackenzie_tester::run, &mackenzie));

       // Wait 60 msec before starting next thread
       boost::this_thread::sleep(boost::posix_time::milliseconds(60));

       // Start second thread
       tgroup.create_thread(boost::bind(&mackenzie_tester::run, &mackenzie));

       // Wait for threads to finish then delete them.
       tgroup.join_all();

    } catch (std::exception* except) {
        BOOST_ERROR(except->what());
    }
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
