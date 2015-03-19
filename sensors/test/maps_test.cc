/**
 * @example sensors/test/maps_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <boost/progress.hpp>

#include <iostream>

#include <usml/sensors/sensor_map.h>
#include <usml/sensors/beam_pattern_map.h>
#include <usml/sensors/source_params_map.h>
#include <usml/sensors/receiver_params_map.h>


BOOST_AUTO_TEST_SUITE(maps_test)

using namespace boost::unit_test;

using namespace usml::sensors;

/**
* @ingroup sensors_test
* Test the ability to instantiate a beam_pattern_map
* and insert several beam_pattern_models into it.
* Also test the find method and the destructor.
* Generate errors if pointer values are not equal.
* For Destructor testing run with Valgrind memcheck.
*
* @author Ted Burns, AEgis Technologies Group, Inc.
* @version 1.0
* @updated 6-Mar-2015 3:15:03 PM
*/
BOOST_AUTO_TEST_CASE(beam_pattern_test) {

    cout << "=== maps_test: beam_pattern_test ===" << endl;

    beam_pattern_map* beam_map = beam_pattern_map::instance();

    beam_pattern_model* beam_heap1 = new beam_pattern_omni();
   
    beam_map->insert(1, beam_heap1);

    beam_pattern_model* beam_heap2 = new beam_pattern_omni();

    beam_map->insert(2, beam_heap2);

    const usml::sensors::beam_pattern_model* bpm1 = beam_map->find(1);

    BOOST_CHECK_EQUAL(bpm1, beam_heap1);

    const usml::sensors::beam_pattern_model* bpm2 = beam_map->find(2);

    BOOST_CHECK_EQUAL(bpm2, beam_heap2);

    // Check key not found returns null
    const usml::sensors::beam_pattern_model* bpm = beam_map->find(3);

    BOOST_CHECK_EQUAL(bpm, (const usml::sensors::beam_pattern_model*)0);

    // Run with valgrind memcheck to verify.
    beam_pattern_map::destroy();
}

/**
* @ingroup sensors_test
* Test the ability to instantiate a source_params_map
* and insert several source_params into it.
* Also test the find method and the destructor.
* Generate errors if pointer values are not equal.
* For Destructor testing run with Valgrind memcheck.
*/

BOOST_AUTO_TEST_CASE(source_params_test) {

    cout << "=== maps_test: source_params_test ===" << endl;

    std::list<beamIDType> beamList1;
    beamList1.push_back(1);
    beamList1.push_back(2);

    // Set up ID 1
    paramsIDType sourceID = 1;
    // Insert array in source
    source_params* source_heap1 = new source_params(sourceID, 20, 900, 0.5, 100.0, beamList1);

    // Grab source_map
    source_params_map* source_map = source_params_map::instance();

    // Insert into map
    source_map->insert(sourceID, source_heap1);

    // Set up ID 2
    std::list<beamIDType> beamList2;
    beamList2.push_back(3);
    beamList2.push_back(4);

    sourceID = 2;
    source_params* source_heap2 = new source_params(sourceID, 10, 900, 0.5, 100.0, beamList2);

    // Insert into map
    source_map->insert(sourceID, source_heap2);

    const usml::sensors::source_params* spm1 = source_map->find(1);

    BOOST_CHECK_EQUAL(spm1, source_heap1);

    const usml::sensors::source_params* spm2 = source_map->find(2);

    BOOST_CHECK_EQUAL(spm2, source_heap2);

    // Check key not found returns null
    const usml::sensors::source_params* spm3 = source_map->find(3);

    BOOST_CHECK_EQUAL(spm3, (const usml::sensors::source_params*)0);

    // Run with valgrind memcheck to verify.
    source_params_map::destroy();
}

/**
* @ingroup sensors_test
* Test the ability to instantiate a receiver_params_map
* and insert several receiver_params into it.
* Also test the find method and the destructor.
* Generate errors if pointer values are not equal.
* For Destructor testing run with Valgrind memcheck.
*/

BOOST_AUTO_TEST_CASE(receiver_params_test) {

    cout << "=== maps_test: receiver_params_test ===" << endl;

    std::list<beamIDType> beamList1;
    beamList1.push_back(1);
    beamList1.push_back(2);

    paramsIDType receiverID = 1;

    // Grab receiver_map
    receiver_params_map* receiver_map = receiver_params_map::instance();

    // Insert  beam array list in receiver
    receiver_params* paramsHeap1 = new receiver_params(receiverID, beamList1);

    // Insert receiver in receiver_map
    receiver_map->insert(receiverID, paramsHeap1);

    std::list<beamIDType> beamList2;
    beamList2.push_back(3);
    beamList2.push_back(4);

    // Bump ID
    receiverID = 2;

    // Insert  beam array list in receiver
    receiver_params* paramsHeap2 = new receiver_params(receiverID, beamList2 );

    // Insert in receiver map
    receiver_map->insert(2, paramsHeap2);

    // find(1)
    receiverID = 1;
    const usml::sensors::receiver_params* rpm1 = receiver_map->find(receiverID);

    BOOST_CHECK_EQUAL(rpm1, paramsHeap1);

    // find(2)
    receiverID = 2;
    const usml::sensors::receiver_params* rpm2 = receiver_map->find(receiverID);

    BOOST_CHECK_EQUAL(rpm2, paramsHeap2);

    // Check key not found returns null
    const usml::sensors::receiver_params* rpm = receiver_map->find(3);

    BOOST_CHECK_EQUAL(rpm, (const usml::sensors::receiver_params*)0);

    // Run with valgrind memcheck to verify.
    receiver_params_map::destroy();
}

/**
* @ingroup sensors_test
* Test the ability to instantiate a receiver_params_map
* and insert several receiver_params into it.
* Also test the find method and the destructor.
* Generate errors if pointer values are not equal.
* For Destructor testing run with Valgrind memcheck.
*/

BOOST_AUTO_TEST_CASE(sensor_test) {

    cout << "=== maps_test: sensor_test ===" << endl;

    std::list<beamIDType> beamList1;
    beamList1.push_back(1);
    beamList1.push_back(2);

    // Grab the source map
    source_params_map* source_map = source_params_map::instance();

    // Insert array in source
    paramsIDType paramID = 1;
    source_params* src_params = new source_params(paramID, 20, 900, 0.5, 100.0, beamList1);
    // Insert source_param into source map
    source_map->insert(paramID, src_params);

    // Grab the receiver map
    receiver_params_map* receiver_map = receiver_params_map::instance();

    std::list<beamIDType> beamList2;
    beamList2.push_back(2);
    beamList2.push_back(3);

    receiver_params* rcv_params = new receiver_params(paramID, beamList2);
    // Insert in receiver map
    receiver_map->insert(paramID, rcv_params);


    sensor* sensor_data;
    //No sensorID
    sensor_data = new sensor();

    // Test change xmitRcvMode
    sensor_data->mode(usml::sensors::SOURCE);
    sensor_data->source(*src_params);

    // Get sensor_map
    sensor_map* sensorMap = sensor_map::instance();

    // insert
    sensorIDType sensorID = 1;
    sensorMap->insert(sensorID, sensor_data);


    // Test find(1)
    usml::sensors::sensor* m1 = sensorMap->find(sensorID);

    // Check find
    BOOST_CHECK_EQUAL(m1, sensor_data);

    // Modify #1 paramID #1
    sensor_data = new sensor(sensorID, paramID, usml::sensors::BOTH,
                             wposition1(0.0,0.0,0.0), 0.0, 0.0);

    // Test update
    sensor_data->source(*src_params);
    sensor_data->receiver(*rcv_params);
    if (sensorMap->update(sensorID, sensor_data) == false) {
        BOOST_FAIL("sensor_test::Failed to update sensor!");
    }

    // Test erase #1
    sensorMap->erase(1, usml::sensors::BOTH);

    // Run with valgrind memcheck to verify.
    sensor_map::destroy();
    source_params_map::destroy();
    receiver_params_map::destroy();
    delete sensor_data ;
}

BOOST_AUTO_TEST_SUITE_END()
