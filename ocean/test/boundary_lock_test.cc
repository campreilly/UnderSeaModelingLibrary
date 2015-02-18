/**
 * @example ocean/test/boundary_lock_test.cc
 */
#include <boost/thread.hpp>
#include <boost/test/unit_test.hpp>

#include <usml/ublas/randgen.h>
#include <usml/netcdf/netcdf_files.h>
#include <usml/ocean/ocean.h>


BOOST_AUTO_TEST_SUITE(boundary_lock_test)

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
            _lockableBoundary(NULL)
        {
            //cout << "tester_base constructor" << endl;
        }

        // Destructor
        virtual ~tester_base()
        {
            //cout << "tester_base destructor" << endl;
            delete _lockableBoundary;
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

        // Pure virtual method which must be overloaded in child classes
        virtual void test() = 0;

    protected:

        // The boundary_lock instance used by all threads.
        usml::ocean::boundary_lock* _lockableBoundary;
};

/**
 * etopo_boundary_tester class the inherits the tester_base class above and is used
 * to create a single instance of a boundary_lock that is accessed by multiple threads.
 * The test method is a replica of the boundary_test/etopo_boundary_test.
 */
class etopo_boundary_tester : public tester_base
{
    public:

    // The setup method is is used to create a boundary_grid
    // that is passed into the constructor of boundary_lock.
    void setup()
    {
        boundary_grid<double, 2>* model =
            new boundary_grid<double, 2>(new netcdf_bathy(
            USML_DATA_DIR "/bathymetry/ETOPO1_Ice_g_gmt4.grd",
            36.0, 36.2, 15.85, 16.0, wposition::earth_radius));
        _lockableBoundary = new boundary_lock(model);
    }

    // This method overloads the pure virtual "test" method in the tester_base.
    // It tests the _heightMutex inside the boundary_lock via calls to height()
    // from multiple threads.
    // The test method is a replica of the boundary_test/etopo_boundary_test.
    virtual void test()
    {
        // simple values for points and depth

        wposition1 points;
        points.latitude(36.000447);
        points.longitude(15.890411);
        double depth;
        wvector1 normal;

        // compute bathymetry

        _lockableBoundary->height(points, &depth, &normal);

        // check the answer

        #ifdef __FAST_MATH__
            const double depth_accuracy = 0.005 ;
            const double normal_accuracy = 2e-4 ;
        #else
            const double depth_accuracy = 5e-4 ;
            const double normal_accuracy = 2e-4 ;
        #endif

        BOOST_CHECK_CLOSE(wposition::earth_radius - depth, 3671.1557116601616, depth_accuracy );
        BOOST_CHECK( abs(normal.theta()) < normal_accuracy );
        BOOST_CHECK( abs(normal.phi() - 0.012764948465248139) < normal_accuracy );

    }

}; // end etopo_boundary_tester class

/**
 * Using the boundary_lock class and multiple threads test the basic
 * features of the boundary_grid with calls to the height() method to
 * obtain the bathymetry data. The test portion is a replica of the
 * boundary_test/etopo_boundary_test.
 * Generate errors if values differ by more that 1E-5 percent,
 * or SIG Fault on thread error.
 * When executed the output should show intertwining between the threads.
 */
BOOST_AUTO_TEST_CASE( etopo_boundary_lock_test ) {

    cout << "=== boundary_lock_test: etopo_boundary_lock_test ===" << endl;
    try {

        // Instantiate a etopo_boundary_tester class defined above.
        // etopo_boundary_tester class is a tester_base class that
        // overloads the "test" method.
        etopo_boundary_tester etopo_boundary;

        // Setup initial conditions
        etopo_boundary.setup();

        // Create thread group to hold threads
        boost::thread_group tgroup;

        // Start two threads each one running the "test" method five times
        // Using thread group create a thread which executes the "run" method in tester_base
        // Start first thread
        tgroup.create_thread(boost::bind(&etopo_boundary_tester::run, &etopo_boundary));

        // Wait 60 msec before starting next thread
        boost::this_thread::sleep(boost::posix_time::milliseconds(60));

        // Start second thread
        tgroup.create_thread(boost::bind(&etopo_boundary_tester::run, &etopo_boundary));

        // Wait for threads to finish then delete them.
        tgroup.join_all();

    } catch (std::exception* except) {
        BOOST_ERROR(except->what());
    }
}

/**
 * reflect_loss_tester a class the inherits the tester_base class above and is used
 * to create a single instance of a boundary_lock that is accessed by multiple threads.
 * The test method is a replica of the reflect_loss_test/reflect_loss_netcdf_test.
 */
class reflect_loss_tester : public tester_base
{
    public:

    // The setup method is is used to create a reflect_loss_netcdf
    // that is passed into the constructor of boundary_flat which is then
    // passed into the boundary_lock constructor.
    void setup()
    {
        reflect_loss_netcdf* reflectLoss = new reflect_loss_netcdf( USML_DATA_DIR "/bottom_province/sediment_test.nc" ) ;

        boundary_flat* model = new boundary_flat(1000.0, reflectLoss);

        _lockableBoundary = new boundary_lock(model);
    }

    // This method overloads the pure virtual "test" method in the tester_base.
    // It tests the _reflect_lossMutex inside the boundary_lock via calls to the
    // reflect_loss() method from multiple threads.
    // The test method is a replica of the reflect_loss_test/reflect_loss_netcdf_test.
    virtual void test()
    {
        seq_linear frequency(1000.0, 1000.0, 0.01) ;
        double angle = M_PI_2 ;
        vector<double> amplitude( frequency.size() ) ;

        double limestone = 3.672875 ;
        double sand = 10.166660 ;
        double tolerance = 4e-4 ;

        /** bottom type numbers in the center of the data field top left, right, bottom left, right */
        _lockableBoundary->reflect_loss(wposition1(29.5, -83.4), frequency, angle, &amplitude) ;
        BOOST_CHECK_CLOSE(amplitude(0), limestone, tolerance) ;
        _lockableBoundary->reflect_loss(wposition1(30.5, -83.4), frequency, angle, &amplitude) ;
        BOOST_CHECK_CLOSE(amplitude(0), sand, tolerance) ;
        _lockableBoundary->reflect_loss(wposition1(29.5, -84.2), frequency, angle, &amplitude) ;
        BOOST_CHECK_CLOSE(amplitude(0), sand, tolerance) ;
        _lockableBoundary->reflect_loss(wposition1(30.5, -84.2), frequency, angle, &amplitude) ;
        BOOST_CHECK_CLOSE(amplitude(0), limestone, tolerance) ;
        /** bottom type numbers at the corners of the data field top left, bottom left, top right, bottom right */
        _lockableBoundary->reflect_loss(wposition1(26, -80), frequency, angle, &amplitude) ;
        BOOST_CHECK_CLOSE(amplitude(0), sand, tolerance) ;
        _lockableBoundary->reflect_loss(wposition1(26, -89), frequency, angle, &amplitude) ;
        BOOST_CHECK_CLOSE(amplitude(0), limestone, tolerance) ;
        _lockableBoundary->reflect_loss(wposition1(35, -80), frequency, angle, &amplitude) ;
        BOOST_CHECK_CLOSE(amplitude(0), sand, tolerance) ;
        _lockableBoundary->reflect_loss(wposition1(35, -89), frequency, angle, &amplitude) ;
        BOOST_CHECK_CLOSE(amplitude(0), limestone, tolerance) ;
    }

}; // end reflect_loss_tester class

/**
 * Using the boundary_lock class and multiple threads test the basic
 * features of the reflection loss model using the netCDF bottom type file.
 * The test portion is a replica of the reflect_loss_test/reflect_loss_netcdf_test.
 * Generate errors if values differ by more that 1E-5 percent,
 * or SIG Fault on thread error.
 * When executed the output should show intertwining between the threads.
 */
BOOST_AUTO_TEST_CASE( reflect_loss_boundary_lock_test ) {

    cout << "=== boundary_lock_test: reflect_loss_boundary_lock_test ===" << endl;
    try {

        // Instantiate a reflect_loss_tester class defined above.
        // reflect_loss_tester class is a tester_base class that
        // overloads the "test" method.
        reflect_loss_tester reflect_loss_boundary;

        // Setup initial conditions
        reflect_loss_boundary.setup();

        // Create thread group to hold threads
        boost::thread_group tgroup;

        // Start two threads each one running the "test" method five times.
        // Using thread group create a thread which executes the "run" method in tester_base
        // Start first thread
        tgroup.create_thread(boost::bind(&reflect_loss_tester::run, &reflect_loss_boundary));

        // Wait 60 msec before starting next thread
        boost::this_thread::sleep(boost::posix_time::milliseconds(60));

        // Start second thread
        tgroup.create_thread(boost::bind(&reflect_loss_tester::run, &reflect_loss_boundary));

        // Wait for threads to finish then delete them.
        tgroup.join_all();

    } catch (std::exception* except) {
        BOOST_ERROR(except->what());
    }
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
