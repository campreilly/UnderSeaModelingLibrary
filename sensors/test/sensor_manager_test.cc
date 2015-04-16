/**
 * @example sensors/test/sensor_manager_test.cc
 */
#include <usml/sensors/sensor_manager.h>
#include <usml/sensors/sensor_pair_manager.h>
#include <usml/sensors/beam_pattern_map.h>
#include <usml/sensors/source_params_map.h>
#include <usml/sensors/receiver_params_map.h>
#include <boost/test/unit_test.hpp>
#include <iostream>


using namespace boost::unit_test;
using namespace usml::sensors;

BOOST_AUTO_TEST_SUITE(sensor_manager_test)

/**
* @ingroup sensors_test
* @{
*/

BOOST_AUTO_TEST_CASE(pairs_test)
{
    cout << "=== sensor_manager_test: pairs_test ===" << endl;
    
    // insert beam patterns into map

    beam_pattern_map* beam_map = beam_pattern_map::instance();

    beam_pattern_model* beam_patt1 = new beam_pattern_omni();
    beam_pattern_model::id_type id0 = 0;
    beam_pattern_model::id_type id1 = 1;
    beam_patt1->beamID(id1);
    beam_pattern_model::reference beam_heap1(beam_patt1);
    beam_map->insert(id1, beam_heap1);

    std::list<beam_pattern_model::id_type> beamList;
    beamList.push_back(id0);
    beamList.push_back(id1);

    // Source frequencies 6.5K, 7.5K, 8.5K, 9.5K
    seq_linear source_frequencies(6500.0, 1000.0, 4);
    // Receiver frequencies 3.0K, 10.0K
    seq_linear receiver_frequencies(3000.0, 7000.0, 2);

    // setup SOURCE sensor type  #12 with omni beam pattern
    sensor_params::id_type params1 = 12;
    source_params::reference source1(new source_params(
        params1,	// paramsID
        123.0,		// source_level
        source_frequencies,
        0));		// beamID
    source_params_map::instance()->insert(source1->paramsID(), source1);

    // setup RECEIVER sensor type #21 
    sensor_params::id_type params2 = 21;
    receiver_params::reference receiver1(new receiver_params(
        params2,	// paramsID
        receiver_frequencies,
        beamList));
    receiver_params_map::instance()->insert(receiver1->paramsID(), receiver1);

    // Sensor type 33 is for a BOTH mode with src and receiver NOT multistatic
    // setup source side of sensor type #33  BOTH with omni beam pattern
    sensor_params::id_type params3 = 33;
    source_params::reference source3(new source_params(
        params3,	// paramsID
        130.0,		// source_level
        source_frequencies,
        0,   		// beamID
        false));	// not multistatic
    source_params_map::instance()->insert(source3->paramsID(), source3);

    // setup receiver side of sensor type #33 BOTH with beam pattern's 0 and 1
    receiver_params::reference receiver3(new receiver_params(
        params3,	// paramsID
        receiver_frequencies,
        beamList,
        false));	// not multistatic
    receiver_params_map::instance()->insert(receiver3->paramsID(), receiver3);

    // Vary source and or reciever mutlistatic flag's to test 
    // setup source side of sensor type #44 BOTH with omni beam pattern
    sensor_params::id_type params4 = 44;
    source_params::reference source4(new source_params(
        params4,	// paramsID	
        130.0,		// source_level
        source_frequencies,
        0,  		// beamID
        true));     // multistatic flag - vary to test different combo's
    source_params_map::instance()->insert(source4->paramsID(), source4);
    // setup receiver side of sensor type #44 with beam pattern's 0 and 1
    receiver_params::reference receiver4(new receiver_params(
        params4,	// paramsID
        receiver_frequencies,
        beamList,
        true));     // multistatic flag - vary to test different combo's
    receiver_params_map::instance()->insert(receiver4->paramsID(), receiver4);


    // Create sensor IDs and param IDs 
    sensor_model::id_type sensors[] = {1, 3, 4, 6, 7, 9};

    sensor_params::id_type sensor_type[] = { 33,   // usml::sensors::BOTH,     // 1 source and receiver NOT multistatic
                                             12,   // usml::sensors::SOURCE,   // 3
                                             21,   // usml::sensors::RECEIVER, // 4
                                             12,   // usml::sensors::SOURCE,   // 6
                                             21,   // usml::sensors::RECEIVER, // 7
                                             44 }; // usml::sensors::BOTH      // 9 Vary source and or reciever mutlistatic flag's to test 

    sensor_manager* manager = sensor_manager::instance();
    for ( unsigned i = 0; i < sizeof(sensors)/sizeof(sensor_model::id_type); ++i ) {
        manager->add_sensor(sensors[i], sensor_type[i]);
    }

    // Attempt to remove a non-existant sensor
    if ( manager->remove_sensor(2) != false ) {
         BOOST_FAIL("pairs_test:: Removed non-existent sensor_model");
    }

    //cout << "=== pairs_test: remove_sensor 1 BOTH ===" << endl;
    if ( manager->remove_sensor(sensors[0]) == false )
    {
         BOOST_FAIL("pairs_test:: Failed to remove sensor_model");
    }

    //cout << "=== pairs_test: remove_sensor 3 SOURCE ===" << endl;
    if ( manager->remove_sensor(sensors[1]) == false )
    {
         BOOST_FAIL("pairs_test:: Failed to remove sensor_model");
    }

    //cout << "=== pairs_test: remove_sensor 4 RECEIVER ===" << endl;
    if ( manager->remove_sensor(sensors[2]) == false )
    {
         BOOST_FAIL("pairs_test:: Failed to remove sensor_model");
    }

    // Expected map contents
     sensor_model::id_type sensors_remaining[] = {6, 7, 9};

    // From
    // xmitRcvModeType sensor_mode[] = { usml::sensors::BOTH,     // 1
    //                                   usml::sensors::SOURCE,   // 3
    //                                   usml::sensors::RECEIVER, // 4
    //                                   usml::sensors::SOURCE,   // 6
    //                                   usml::sensors::RECEIVER, // 7
    //                                   usml::sensors::BOTH }    // 9

     xmitRcvModeType sensor_mode[] = { usml::sensors::SOURCE,   // 6
                                       usml::sensors::RECEIVER, // 7
                                       usml::sensors::BOTH };   // 9
    // Build a query 
    sensor_pair_manager::sensor_query_map query;
    for ( int i = 0; i < sizeof(sensors_remaining) / sizeof(sensor_model::id_type); ++i ) {
        query.insert(std::pair<sensor_model::id_type, xmitRcvModeType>(sensors_remaining[i], sensor_mode[i]));
    }

    // Test find_pair through get_fathometers
    fathometer_model::fathometer_package fathometers = 
        sensor_pair_manager::instance()->get_fathometers(query);

    cout << "=== pairs_test: fathometers return size " << fathometers.size() << endl;
    
    int index = 0;
    std::string ncname = USML_TEST_DIR "/sensors/test/fathometers_";
    std::stringstream fatho_filename;
    fathometer_model::fathometer_package::iterator iter;
    for ( iter = fathometers.begin(); iter != fathometers.end(); ++iter ) 
    {
        fathometer_model* model = (*iter);
        fatho_filename.clear();
        fatho_filename << ncname << index++ << ".nc";
        model->write_netcdf(fatho_filename.str().c_str(), "");
    }

    // Clean up all singleton to prevent use by other tests!
    source_params_map::reset();
    receiver_params_map::reset();
    beam_pattern_map::reset();
    sensor_pair_manager::reset();
    sensor_manager::reset();
    
} // end pairs_test

BOOST_AUTO_TEST_SUITE_END()
