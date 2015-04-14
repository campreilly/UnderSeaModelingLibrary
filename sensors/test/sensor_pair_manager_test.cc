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
                                       usml::sensors::BOTH };   // 9
    // Build a query 
    sensor_pair_manager::sensor_query_map query;
    for ( int i = 0; i < sizeof(sensors_remaining) / sizeof(sensor_model::id_type); ++i ) {
        query.insert(std::pair<sensor_model::id_type, xmitRcvModeType>(sensors_remaining[i], sensor_mode[i]));
    }

    // test find_pair through get_fathometers
    sp_manager->get_fathometers(query);

    source_params_map::reset();
    receiver_params_map::reset();
    sensor_pair_manager::reset();
    
} // end find_pairs_test

BOOST_AUTO_TEST_SUITE_END()
