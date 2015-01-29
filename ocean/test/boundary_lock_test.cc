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

class USML_DECLSPEC tester_base
{
    public:

        tester_base() :
            _lockableBoundary(NULL)
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

        usml::ocean::boundary_lock* _lockableBoundary;
};

class USML_DECLSPEC etopo_boundary_tester : public tester_base
{
    public:

    void setup()
    {
        boundary_grid<double, 2>* model =
            new boundary_grid<double, 2>(new netcdf_bathy(
            USML_DATA_DIR "/bathymetry/ETOPO1_Ice_g_gmt4.grd",
            36.0, 36.2, 15.85, 16.0, wposition::earth_radius));
        _lockableBoundary = new boundary_lock(model);
    }

    void test()
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

BOOST_AUTO_TEST_CASE( etopo_boundary_lock_test ) {

    cout << "=== boundary_lock_test: etopo_boundary_lock_test ===" << endl;
    try {

        etopo_boundary_tester etopo_boundary;
        etopo_boundary.setup();

        boost::thread t1(etopo_boundary);
        boost::thread t2(etopo_boundary);

        t1.join();
        t2.join();

    } catch (std::exception* except) {
        BOOST_ERROR(except->what());
    }
}

class USML_DECLSPEC reflect_loss_tester : public tester_base
{
    public:

    void setup()
    {
        reflect_loss_netcdf* reflectLoss = new reflect_loss_netcdf( USML_DATA_DIR "/bottom_province/sediment_test.nc" ) ;

        boundary_flat* model = new boundary_flat(1000.0, reflectLoss);

        _lockableBoundary = new boundary_lock(model);
    }

    void test()
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
 * Using the boundary_lock class and multiple threads test the basic features
 * of the reflection loss model using the netCDF bottom type file.
 * Generate errors if values differ by more that 1E-5 percent.
 */
BOOST_AUTO_TEST_CASE( reflect_loss_boundary_lock_test ) {

    cout << "=== boundary_lock_test: reflect_loss_boundary_lock_test ===" << endl;
    try {

        reflect_loss_tester reflect_loss_boundary;
        reflect_loss_boundary.setup();

        boost::thread t1(reflect_loss_boundary);
        boost::thread t2(reflect_loss_boundary);

        t1.join();
        t2.join();

    } catch (std::exception* except) {
        BOOST_ERROR(except->what());
    }
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
