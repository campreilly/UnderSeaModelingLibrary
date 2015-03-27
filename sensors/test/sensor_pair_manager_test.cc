/**
 * @example sensors/test/sensor_pair_manager_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <boost/progress.hpp>

#include <iostream>

#include <usml/sensors/sensor_pair_manager.h>

BOOST_AUTO_TEST_SUITE(sensor_pair_manager_test)

using namespace boost::unit_test;

using namespace usml::sensors;

/**
* @ingroup sensors_test
* @{
*/

class thread_test_base
{
    public:

        // Constructor
        thread_test_base() :
            _sensor_pair_manager(NULL)
        {
            //cout << "thread_test_base constructor" << endl;
        }

        // Destructor
        virtual ~thread_test_base()
        {
            //cout << "thread_test_base destructor" << endl;
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
 * thread_tester a class the inherits the thread_test_base class above and is used
 * to create a single instance of a sensor_pair_manager that is accessed by multiple threads.
 * The test method is a replica of the sensor_pair_manager_test/add_remove_test, only the
 * BOOST_FAIL items are no longer failures just return status.
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
        sensor::id_type sensors[] = {1, 3, 4, 6, 7, 9};

        xmitRcvModeType sensor_type[] = {usml::sensors::BOTH,     // 1
                                     usml::sensors::SOURCE,   // 3
                                     usml::sensors::RECEIVER, // 4
                                     usml::sensors::SOURCE,   // 6
                                     usml::sensors::SOURCE,   // 7
                                     usml::sensors::BOTH };   // 9

        wposition1 pos(0.0, 0.0, 0.0);

        for ( unsigned i = 0; i < sizeof(sensors)/sizeof(sensor::id_type); ++i )
        {
            cout << "=== sensor_pair_manager_test: add_sensor sensorID "<< sensors[i] << " Type "<< sensor_type[i]  << endl;
            // Randomly wait from 0.1 seconds to 1.0 seconds
            random_wait();
            sensor* sensor_ = new sensor(sensors[i], 0, sensor_type[i], pos, 0.0, 0.0);
            _sensor_pair_manager->add_sensor(sensor_);
        }

        cout << "=== sensor_pair_manager_test: remove_sensor 1 BOTH ===" << endl;
        random_wait();
        sensor* sensor_one = new sensor(sensors[0], 0, sensor_type[0], pos, 0.0, 0.0);
        if (_sensor_pair_manager->remove_sensor(sensor_one) == false) {
             cout << "=== sensor_pair_manager_test: 1 BOTH Previously Removed ===" << endl;
        }

        cout << "=== sensor_pair_manager_test: remove_sensor 3 SOURCE ===" << endl;
        random_wait();
        sensor* sensor_three = new sensor(sensors[1], 0, sensor_type[1], pos, 0.0, 0.0);
        if (_sensor_pair_manager->remove_sensor(sensor_three) == false) {
            cout << "=== sensor_pair_manager_test: 3 SOURCE Previously Removed ===" << endl;
        }

        cout << "=== sensor_pair_manager_test: remove_sensor 4 RECEIVER ===" << endl;
        random_wait();
        sensor* sensor_four = new sensor(sensors[2], 0, sensor_type[2], pos, 0.0, 0.0);
        if (_sensor_pair_manager->remove_sensor(sensor_four) == false) {
             cout << "=== sensor_pair_manager_test: 4 RECEIVER Previously Removed ===" << endl;
        }
    }

}; // end thread_tester class

/**
 * Test the functionality of adding and removing sensor_pair's
 * to the sensor_pair_map from multiple threads.
 */
BOOST_AUTO_TEST_CASE(thread_test) {

    cout << "=== sensor_pair_manager_test: thread_test ===" << endl;

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

    cout << "=== sensor_pair_manager_test: add_remove_test ===" << endl;

    sensor_pair_manager* sp_manager = sensor_pair_manager::instance();

    sensor::id_type sensors[] = {1, 3, 4, 6, 7, 9};

    xmitRcvModeType sensor_type[] = {usml::sensors::BOTH,     // 1
                                     usml::sensors::SOURCE,   // 3
                                     usml::sensors::RECEIVER, // 4
                                     usml::sensors::SOURCE,   // 6
                                     usml::sensors::SOURCE,   // 7
                                     usml::sensors::BOTH };   // 9

     wposition1 pos(0.0, 0.0, 0.0);

    for ( unsigned i = 0; i < sizeof(sensors)/sizeof(sensor::id_type); ++i )
    {
        //cout << "=== sensor_pair_manager_test: add_sensor sensorID "<< sensors[i] << " Type "<< sensor_type[i]  << endl;
        sensor* sensor_ = new sensor(sensors[i], 0, sensor_type[i], pos, 0.0, 0.0);
        sp_manager->add_sensor(sensor_);
    }

    //cout << "=== sensor_pair_manager_test: remove_sensor non-existent 2 ===" << endl;
    sensor* sensor_two = new sensor(2, 0, usml::sensors::BOTH, pos, 0.0, 0.0);
    if (sp_manager->remove_sensor(sensor_two) != false) {
         BOOST_FAIL("sensor_pair_manager_test:: Removed non-existent sensor");
    }

    //cout << "=== sensor_pair_manager_test: remove_sensor 1 BOTH ===" << endl;
    sensor* sensor_one = new sensor(sensors[0], 0, sensor_type[0], pos, 0.0, 0.0);
    if (sp_manager->remove_sensor(sensor_one) == false) {
         BOOST_FAIL("sensor_pair_manager_test:: Failed to remove sensor");
    }

    //cout << "=== sensor_pair_manager_test: remove_sensor 3 SOURCE ===" << endl;
    sensor* sensor_three = new sensor(sensors[1], 0, sensor_type[1], pos, 0.0, 0.0);
    if (sp_manager->remove_sensor(sensor_three) == false) {
         BOOST_FAIL("sensor_pair_manager_test:: Failed to remove sensor");
    }

    //cout << "=== sensor_pair_manager_test: remove_sensor 4 RECEIVER ===" << endl;
     sensor* sensor_four = new sensor(sensors[2], 0, sensor_type[2], pos, 0.0, 0.0);
    if (sp_manager->remove_sensor(sensor_four) == false) {
         BOOST_FAIL("sensor_pair_manager_test:: Failed to remove sensor");
    }

     // Expected map contents
     sensor::id_type srcID_remaining[] = {6, 7, 9};
     sensor::id_type rcvID_remaining = 9;

     sensor_pair_manager::sensor_pair_map_type::iterator  sp_map_iter;
     const sensor_pair_manager::sensor_pair_map_type* sp_map = sp_manager->sensor_pair_map();
     if (sp_map->size() != sizeof(srcID_remaining)/sizeof(sensor::id_type)) {

         BOOST_FAIL("sensor_pair_manager_test:: sensor_pair_map size not correct");
     }
     else
     {
         cout << "Remaining sensor_pairs in map as expected : " << endl;
         // loop through each sensor_pair to verify expected map contents.
         for (unsigned i  = 0; i < sizeof(srcID_remaining)/sizeof(sensor::id_type); ++i)
         {
             sensor::id_type sourceID = srcID_remaining[i];
             std::stringstream hash_key;
             hash_key<< sourceID << "_" <<  rcvID_remaining;
             sensor_pair* the_sensor_pair = sp_map->find(hash_key.str())->second;
             cout << "_sensor_pair_map[" << hash_key.str() << "] = ";
             cout << "  sourceID: " <<  the_sensor_pair->source()->sensorID();
             cout << "  receiverID: " <<  the_sensor_pair->receiver()->sensorID() << endl;
             BOOST_CHECK(the_sensor_pair->source()->sensorID() == srcID_remaining[i]);
             BOOST_CHECK(the_sensor_pair->receiver()->sensorID() == rcvID_remaining);
         }
     }

    // Run with valgrind memcheck to verify.

} // end add_remove_test

BOOST_AUTO_TEST_SUITE_END()
