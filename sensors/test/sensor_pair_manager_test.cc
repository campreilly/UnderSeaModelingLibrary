/**
 * @example sensors/test/sensor_pair_manager_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <boost/progress.hpp>

#include <iostream>

#include <usml/sensors/sensorIDType.h>
#include <usml/sensors/sensor.h>
#include <usml/sensors/sensor_pair_manager.h>
#include <usml/eigenverb/source_data.h>
#include <usml/eigenverb/receiver_data.h>

BOOST_AUTO_TEST_SUITE(sensor_pair_manager_test)

using namespace boost::unit_test;

using namespace usml::sensors;
using namespace usml::eigenverb;

/**
* @ingroup sensors_test
* 
*/

class thread_test_base
{
    public:

        // Constructor
        thread_test_base() :
            _sensor_pair_manager(NULL)
        {
            //cout << "tester_base constructor" << endl;
        }

        // Destructor
        virtual ~thread_test_base()
        {
            //cout << "tester_base destructor" << endl;
            sensor_pair_manager::destroy();
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

        // The sensor_pair_manager instance used by all threads.
        sensor_pair_manager* _sensor_pair_manager;
};

/**
 * reflect_loss_tester a class the inherits the tester_base class above and is used
 * to create a single instance of a boundary_lock that is accessed by multiple threads.
 * The test method is a replica of the reflect_loss_test/reflect_loss_netcdf_test.
 */
class thread_tester : public thread_test_base
{
    public:

    // The setup method is is used to create/get the sensor_pair_manager singleton instance.
    void setup()
    {
        _sensor_pair_manager = sensor_pair_manager::instance();
    }

    // This method overloads the pure virtual "test" method in the thread_test_base.
    // It tests the _mutex inside the sensor_pair_manager via calls to the
    // add_sensor, remove_sensor, and update_sensor methods from multiple threads.
    virtual void test()
    {
        sensorIDType sensors[] = {1, 3, 4, 6, 7, 9};

        xmitRcvModeType sensor_type[] = {usml::sensors::BOTH,     // 1
                                     usml::sensors::SOURCE,   // 3
                                     usml::sensors::RECEIVER, // 4
                                     usml::sensors::SOURCE,   // 6
                                     usml::sensors::SOURCE,   // 7
                                     usml::sensors::BOTH };   // 9

        for ( unsigned i = 0; i < sizeof(sensors)/sizeof(sensorIDType); ++i )
        {
            cout << "=== sensor_pair_map_test: add_sensor sensorID "<< sensors[i] << " Type "<< sensor_type[i]  << endl;
            // Randomly wait from 0.1 seconds to 1.0 seconds
            random_wait();
            _sensor_pair_manager->add_sensor(sensors[i], sensor_type[i] );
        }

        cout << "=== sensor_pair_map_test: remove_sensor 1 BOTH ===" << endl;
        random_wait();
        if (_sensor_pair_manager->remove_sensor(1) == false) {
             BOOST_FAIL("sensor_pair_map_test:: Failed to remove sensor");
        }

        cout << "=== sensor_pair_map_test: remove_sensor 3 SOURCE ===" << endl;
        random_wait();
        if (_sensor_pair_manager->remove_sensor(3) == false) {
             BOOST_FAIL("sensor_pair_map_test:: Failed to remove sensor");
        }

        cout << "=== sensor_pair_map_test: remove_sensor 4 RECEIVER ===" << endl;
        random_wait();
        if (_sensor_pair_manager->remove_sensor(4) == false) {
             BOOST_FAIL("sensor_pair_map_test:: Failed to remove sensor");
        }
    }
}; // end thread_tester class

BOOST_AUTO_TEST_CASE(thread_test) {

    cout << "=== sensor_pair_map_test: thread_test ===" << endl;

    try {

        // Instantiate a reflect_loss_tester class defined above.
        // reflect_loss_tester class is a tester_base class that
        // overloads the "test" method.
        thread_tester sensor_pair_manager_test;

        // Setup initial conditions
        sensor_pair_manager_test.setup();

        // Create thread group to hold threads
        boost::thread_group tgroup;

        // Start two threads each one running the "test" method five times.
        // Using thread group create a thread which executes the "run" method in tester_base
        // Start first thread
        tgroup.create_thread(boost::bind(&thread_tester::run, &sensor_pair_manager_test));

        // Wait 60 msec before starting next thread
        boost::this_thread::sleep(boost::posix_time::milliseconds(60));

        // Start second thread
        tgroup.create_thread(boost::bind(&thread_tester::run, &sensor_pair_manager_test));

        // Wait for threads to finish then delete them.
        tgroup.join_all();

    } catch (std::exception* except) {
        BOOST_ERROR(except->what());
    }
} // end thread_test

BOOST_AUTO_TEST_CASE(add_remove_test) {

    cout << "=== sensor_pair_map_test: init_test ===" << endl;

    sensor_pair_manager* sp_manager = sensor_pair_manager::instance();

    sensorIDType sensors[] = {1, 3, 4, 6, 7, 9};

    xmitRcvModeType sensor_type[] = {usml::sensors::BOTH,     // 1
                                     usml::sensors::SOURCE,   // 3
                                     usml::sensors::RECEIVER, // 4
                                     usml::sensors::SOURCE,   // 6
                                     usml::sensors::SOURCE,   // 7
                                     usml::sensors::BOTH };   // 9

    for ( unsigned i = 0; i < sizeof(sensors)/sizeof(sensorIDType); ++i )
    {
        cout << "=== sensor_pair_map_test: add_sensor sensorID "<< sensors[i] << " Type "<< sensor_type[i]  << endl;

        sp_manager->add_sensor(sensors[i], sensor_type[i] );
    }

    cout << "=== sensor_pair_map_test: remove_sensor non-existent 2 ===" << endl;
    if (sp_manager->remove_sensor(2) != false) {
         BOOST_FAIL("sensor_pair_map_test:: Removed non-existent sensor");
    }

    cout << "=== sensor_pair_map_test: remove_sensor 1 BOTH ===" << endl;
    if (sp_manager->remove_sensor(1) == false) {
         BOOST_FAIL("sensor_pair_map_test:: Failed to remove sensor");
    }

    cout << "=== sensor_pair_map_test: remove_sensor 3 SOURCE ===" << endl;
    if (sp_manager->remove_sensor(3) == false) {
         BOOST_FAIL("sensor_pair_map_test:: Failed to remove sensor");
    }

    cout << "=== sensor_pair_map_test: remove_sensor 4 RECEIVER ===" << endl;
    if (sp_manager->remove_sensor(4) == false) {
         BOOST_FAIL("sensor_pair_map_test:: Failed to remove sensor");
    }


     sensorIDType srcID_remaining[] = {6, 7, 9};
     sensorIDType rcvID_remaining = 9;
     sensor_map_iter map_iter;
     sensor_list_iter iter;

     // Verify remaining source receiver map
     //         _src_rcv_pair_map[6] = 9
     //         _src_rcv_pair_map[7] = 9
     //         _src_rcv_pair_map[9] = 9
     std::map<sensorIDType, std::list<sensorIDType> > src_rcv_pair = sp_manager->src_rcv_pair_map();
     if (src_rcv_pair.size() != sizeof(srcID_remaining)/sizeof(sensorIDType)) {

         BOOST_FAIL("sensor_pair_map_test:: src_rcv_pair size not correct");
     }
     else {

         for ( unsigned i = 0; i < sizeof(srcID_remaining)/sizeof(sensorIDType); ++i )
         {
             map_iter = src_rcv_pair.find(srcID_remaining[i]);
             if (map_iter == src_rcv_pair.end())
             {
                  BOOST_FAIL("sensor_pair_map_test:: src_rcv_pair source not found");

             } else {

                 std::list<sensorIDType> rcv_list = map_iter->second;
                 iter = std::find(rcv_list.begin(), rcv_list.end(), rcvID_remaining);
                 if (iter == rcv_list.end()){

                     BOOST_FAIL("sensor_pair_map_test:: src_rcv_pair receiver item not found");
                 }
             }
         }
     }
     // Verify remaining receiver source map
     // _rcv_src_pair_map[9] = 6 7 9
     std::map<sensorIDType, std::list<sensorIDType> > rcv_src_pair = sp_manager->rcv_src_pair_map();
     if (rcv_src_pair.size() != 1) {

         BOOST_FAIL("sensor_pair_map_test:: rcv_src_pair size not correct");
     }
     else {

         map_iter = rcv_src_pair.find(rcvID_remaining);
         if (map_iter == rcv_src_pair.end())
         {
             BOOST_FAIL("sensor_pair_map_test:: rcv_src_pair receiver not found");

         } else {

             for ( unsigned i = 0; i < sizeof(srcID_remaining)/sizeof(sensorIDType); ++i )
             {
                 std::list<sensorIDType> src_list = map_iter->second;
                 iter = std::find(src_list.begin(), src_list.end(), srcID_remaining[i]);
                 if (iter == src_list.end()){

                     BOOST_FAIL("sensor_pair_map_test:: rcv_src_pair source item not found");
                 }
             }
         }
      }

    // Verify remaining source data map
    //        _src_data_map[6] = proploss: 0 eigenverbs: 0
    //        _src_data_map[7] = proploss: 0 eigenverbs: 0
    //        _src_data_map[9] = proploss: 0 eigenverbs: 0

     std::map<sensorIDType,source_data>::iterator  src_data_map_iter;
     std::map<sensorIDType,source_data> src_data_map = sp_manager->src_data_map();
     if (src_data_map.size() != sizeof(srcID_remaining)/sizeof(sensorIDType)) {

         BOOST_FAIL("sensor_pair_map_test:: src_data_map size not correct");
     }
     else {

         for ( unsigned i = 0; i < sizeof(srcID_remaining)/sizeof(sensorIDType); ++i )
         {
             src_data_map_iter = src_data_map.find(srcID_remaining[i]);
             if (map_iter == src_rcv_pair.end())
             {
                  BOOST_FAIL("sensor_pair_map_test:: src_data_map item not found");
             }
         }
     }

    // Verify remaining receiver data map
    //        _rcv_data_map[9] = eigenverbs: 0 envelopes: 0
    std::map<sensorIDType,receiver_data>::iterator rcv_data_map_iter;
    std::map<sensorIDType,receiver_data> rcv_data_map = sp_manager->rcv_data_map();
    if (rcv_data_map.size() != 1)
    {
        BOOST_FAIL("sensor_pair_map_test:: rcv_data_map size not correct");

    } else
    {
        rcv_data_map_iter = rcv_data_map.find(rcvID_remaining);
        if (rcv_data_map_iter == rcv_data_map.end())
        {
            BOOST_FAIL("sensor_pair_map_test:: rcv_data_map item not found");
        }
    }

    // Run with valgrind memcheck to verify.
    sensor_pair_manager::destroy();

} // endadd_remove_test

BOOST_AUTO_TEST_SUITE_END()
