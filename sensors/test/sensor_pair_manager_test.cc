/**
 * @example sensors/test/sensor_pair_manager_test.cc
 */
#include <usml/sensors/sensor_pair_manager.h>
#include <usml/sensors/source_params_map.h>
#include <usml/sensors/receiver_params_map.h>
#include <boost/test/unit_test.hpp>
#include <boost/progress.hpp>
#include <iostream>

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
//        sensor_model::id_type sensors[] = {1, 3, 4, 6, 7, 9};
//
//        xmitRcvModeType sensor_type[] = {usml::sensors::BOTH,     // 1
//                                     usml::sensors::SOURCE,   // 3
//                                     usml::sensors::RECEIVER, // 4
//                                     usml::sensors::SOURCE,   // 6
//                                     usml::sensors::SOURCE,   // 7
//                                     usml::sensors::BOTH };   // 9
//
//        wposition1 pos(0.0, 0.0, 0.0);
//
//        for ( unsigned i = 0; i < sizeof(sensors)/sizeof(sensor_model::id_type); ++i )
//        {
//            cout << "=== sensor_pair_manager_test: add_sensor sensorID "<< sensors[i] << " Type "<< sensor_type[i]  << endl;
//            // Randomly wait from 0.1 seconds to 1.0 seconds
//            random_wait();
//            sensor_model* sensor_ = new sensor_model(sensors[i], 0, sensor_type[i], pos, 0.0, 0.0);
//            _sensor_pair_manager->add_sensor(sensor_);
//        }
//
//        cout << "=== sensor_pair_manager_test: remove_sensor 1 BOTH ===" << endl;
//        random_wait();
//        sensor_model* sensor_one = new sensor_model(sensors[0], 0, sensor_type[0], pos, 0.0, 0.0);
//        if (_sensor_pair_manager->remove_sensor(sensor_one) == false) {
//             cout << "=== sensor_pair_manager_test: 1 BOTH Previously Removed ===" << endl;
//        }
//
//        cout << "=== sensor_pair_manager_test: remove_sensor 3 SOURCE ===" << endl;
//        random_wait();
//        sensor_model* sensor_three = new sensor_model(sensors[1], 0, sensor_type[1], pos, 0.0, 0.0);
//        if (_sensor_pair_manager->remove_sensor(sensor_three) == false) {
//            cout << "=== sensor_pair_manager_test: 3 SOURCE Previously Removed ===" << endl;
//        }
//
//        cout << "=== sensor_pair_manager_test: remove_sensor 4 RECEIVER ===" << endl;
//        random_wait();
//        sensor_model* sensor_four = new sensor_model(sensors[2], 0, sensor_type[2], pos, 0.0, 0.0);
//        if (_sensor_pair_manager->remove_sensor(sensor_four) == false) {
//             cout << "=== sensor_pair_manager_test: 4 RECEIVER Previously Removed ===" << endl;
//        }
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

BOOST_AUTO_TEST_CASE(find_pairs_test) {

    cout << "=== sensor_pair_manager_test: find_pairs_test ===" << endl;

    sensor_pair_manager* sp_manager = sensor_pair_manager::instance();

    seq_linear frequencies(1000.0, 10000.0, 1000.0);	// 1-10 KHz

    std::list<beam_pattern_model::id_type> beamList;
    beamList.push_back(0);
    beamList.push_back(1);

    // setup sensor #101 with omni beam pattern
    sensor_params::id_type params1 = 12;
    source_params::reference source1(new source_params(
        params1,	// paramsID
        true,		// multistatic
        123.0,		// source_level
        frequencies,
        0));		// beamID
    source_params_map::instance()->insert(source1->paramsID(), source1);

    // setup sensor #212 with bad beam pattern
    sensor_params::id_type params2 = 21;
    receiver_params::reference receiver1(new receiver_params(
        params2,	// paramsID
        true,		// multistatic
        beamList));
    receiver_params_map::instance()->insert(receiver1->paramsID(), receiver1);

    // setup source side of sensor #33 with omni beam pattern
    sensor_params::id_type params3 = 33;
    source_params::reference source2(new source_params(
        params3,	// paramsID
        false,		// monostatic
        130.0,		// source_level
        frequencies,
        0));		// beamID
    source_params_map::instance()->insert(source2->paramsID(), source2);


    // setup receiver side of sensor #33 with bad beam pattern
    receiver_params::reference receiver2(new receiver_params(
        params3,	// paramsID
        false,		// monostatic
        beamList));
    receiver_params_map::instance()->insert(receiver2->paramsID(), receiver2);

    // Create some sensor IDs and param IDs 
    sensor_model::id_type sensors[] = {1, 3, 4, 6, 7, 9};

    sensor_params::id_type sensor_type[] = { 33,   // usml::sensors::BOTH,     // 1
                                             12,   // usml::sensors::SOURCE,   // 3
                                             21,   // usml::sensors::RECEIVER, // 4
                                             12,   // usml::sensors::SOURCE,   // 6
                                             21,   // usml::sensors::RECEIVER,   // 7
                                             33 }; // usml::sensors::BOTH      // 9

    for ( unsigned i = 0; i < sizeof(sensors)/sizeof(sensor_model::id_type); ++i )
    {
        //cout << "=== sensor_pair_manager_test: add_sensor sensorID "<< sensors[i] << " Type "<< sensor_type[i]  << endl;
        sensor_model::reference sensor = sensor_model::reference(new sensor_model(sensors[i], sensor_type[i]));
        sp_manager->add_sensor(sensor);
    }

    //cout << "=== sensor_pair_manager_test: remove_sensor non-existent 2 ===" << endl;
    sensor_model::reference sensor_two = sensor_model::reference(new sensor_model(2, usml::sensors::BOTH));
    if (sp_manager->remove_sensor(sensor_two) != false) {
         BOOST_FAIL("sensor_pair_manager_test:: Removed non-existent sensor_model");
    }

    //cout << "=== sensor_pair_manager_test: remove_sensor 1 BOTH ===" << endl;
    sensor_model::reference sensor_one = sensor_model::reference(new sensor_model(sensors[0], sensor_type[0]));
    if (sp_manager->remove_sensor(sensor_one) == false) {
         BOOST_FAIL("sensor_pair_manager_test:: Failed to remove sensor_model");
    }

    //cout << "=== sensor_pair_manager_test: remove_sensor 3 SOURCE ===" << endl;
    sensor_model::reference sensor_three = sensor_model::reference(new sensor_model(sensors[1], sensor_type[1]));
    if (sp_manager->remove_sensor(sensor_three) == false) {
         BOOST_FAIL("sensor_pair_manager_test:: Failed to remove sensor_model");
    }

    //cout << "=== sensor_pair_manager_test: remove_sensor 4 RECEIVER ===" << endl;
    sensor_model::reference sensor_four = sensor_model::reference(new sensor_model(sensors[2], sensor_type[2]));
    if (sp_manager->remove_sensor(sensor_four) == false) {
         BOOST_FAIL("sensor_pair_manager_test:: Failed to remove sensor_model");
    }

     // Expected map contents
     sensor_model::id_type sensors_remaining[] = {6, 7, 9};
     // From
     //xmitRcvModeType sensor_mode[] = { usml::sensors::BOTH,     // 1
     //                                  usml::sensors::SOURCE,   // 3
     //                                  usml::sensors::RECEIVER, // 4
     //                                  usml::sensors::SOURCE,   // 6
     //                                  usml::sensors::RECEIVER, // 7
     //                                  usml::sensors::BOTH }    // 9

     xmitRcvModeType sensor_mode[] = { usml::sensors::SOURCE,   // 6
                                       usml::sensors::RECEIVER, // 7
                                       usml::sensors::BOTH };    // 9
    // Build a query 
    sensor_pair_manager::sensor_query_map query;
    for ( int i = 0; i < sizeof(sensors_remaining) / sizeof(sensor_model::id_type); ++i ) {
        query.insert(std::pair<sensor_model::id_type, xmitRcvModeType>(sensors_remaining[i], sensor_mode[i]));
    }

    // test find_pair through get_fathometers
    sp_manager->get_fathometers(query);
    
} // end find_pairs_test

BOOST_AUTO_TEST_SUITE_END()
