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

        tester_base() :
            _lockableProfile(NULL),
            _temp(NULL)
        {}

        virtual ~tester_base()
        {}

        void random_wait()
        {
            double seed = randgen::uniform();
            if (seed == 0.0) seed = 0.0001;
            int msec = seed * 1000;
            if (msec > 1000 ) msec = 1000;
            boost::this_thread::sleep(boost::posix_time::milliseconds(msec));
        }

        // This is the method which gets called from inside the thread.
        void operator () ()
        {
           for (int i = 0; i < 5; ++i)
           {
               random_wait();
               test();
               std::cout << "Thread " << boost::this_thread::get_id() << ": " << i << std::endl;
           }
        }

        virtual void test() = 0;

    protected:

        usml::ocean::profile_lock* _lockableProfile;
        usml::netcdf::netcdf_woa* _temp;

};

class linear_tester : public tester_base
{
    public:

    void setup()
    {
        // liner profile
        attenuation_model* attn = new attenuation_constant(5.0);
        profile_linear* model = new profile_linear();
        model->attenuation(attn);
        _lockableProfile = new profile_lock(model);
    }

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

class mackenzie_tester : public tester_base
{
    public:

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

    void test()
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
 * and attenuation with a random wait between start of the test.
 * Generate errors if values differ by more that 1E-6 percent, or SIG Fault on thread error.
 */
BOOST_AUTO_TEST_CASE( linear_profile_lock_test ) {

    cout << "=== profile_lock_test: linear_profile_lock_test ===" << endl;
    try {

        linear_tester linear;
        linear.setup();
        boost::thread t1(linear);
        boost::thread t2(linear);

        t1.join();
        t2.join();

    } catch (std::exception* except) {
        BOOST_ERROR(except->what());
    }
}

/**
 * This test reproduces the mackenzie_profile_test found in the profile_test
 * only accessing the profile_model with a data_grid via profile_lock class.
 * A random wait is used between the start of each thread that performs the test.
 * Generate errors if values differ by more that 1E-3 percent or SIG Fault on thread error.
 */
BOOST_AUTO_TEST_CASE( mackenzie_profile_lock_test ) {

    cout << "=== profile_lock_test: mackenzie_profile_lock_test ===" << endl;
    try {

        mackenzie_tester mackenzie;
        mackenzie.setup();

        boost::thread t1(mackenzie);
        boost::thread t2(mackenzie);

        t1.join();
        t2.join();

    } catch (std::exception* except) {
        BOOST_ERROR(except->what());
    }
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
