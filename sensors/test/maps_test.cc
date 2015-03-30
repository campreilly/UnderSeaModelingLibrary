/**
 * @example sensors/test/maps_test.cc
 */
#include <boost/test/unit_test.hpp>
// #include <boost/progress.hpp>

// #include <iostream>

#include <usml/sensors/beam_pattern_map.h>
#include <usml/sensors/source_params_map.h>
#include <usml/sensors/receiver_params_map.h>
#include <usml/sensors/sensor_manager.h>

BOOST_AUTO_TEST_SUITE(maps_test)

using namespace boost::unit_test;
using namespace usml::sensors;
using namespace usml::threads;

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
	beam_pattern_model::id_type id0 = 0 ; // default beam

	// insert beam patterns into map

	beam_pattern_model* beam_patt1 = new beam_pattern_omni();
	beam_pattern_model::id_type id1 = 1 ;
	beam_patt1->beamID(id1) ;
	beam_pattern_model::reference beam_heap1(beam_patt1);
	beam_map->insert(id1, beam_heap1);

	beam_pattern_model* beam_patt2 = new beam_pattern_omni();
	beam_pattern_model::id_type id2 = 2 ;
	beam_patt1->beamID(id2) ;
	beam_pattern_model::reference beam_heap2(beam_patt2);
	beam_map->insert(2, beam_heap2);

	// test retrieval

	beam_pattern_model::reference bpm0 = beam_map->find(id0);
	beam_pattern_model::reference bpm1 = beam_map->find(id1);
	beam_pattern_model::reference bpm2 = beam_map->find(id2);
	BOOST_CHECK_EQUAL(bpm0->beamID(), 0);
	BOOST_CHECK_EQUAL(bpm1.get(), beam_patt1);
	BOOST_CHECK_EQUAL(bpm2.get(), beam_patt2);

	// check key not found returns null

	beam_pattern_model::id_type id3 = 3 ;
	beam_pattern_model::reference bpm = beam_map->find(id3);
	BOOST_CHECK_EQUAL(bpm.get(), (beam_pattern_model* ) 0);

	// cleanup inserted beam patterns so that other tests start fresh

	beam_map->erase(id1);
	beam_map->erase(id2);
}

/**
 * @ingroup sensors_test
 * Test the ability to instantiate a source_params_map
 * and insert several source_params into it.
 * Also test the find method.
 * Generate errors if pointer values are not equal.
 */
BOOST_AUTO_TEST_CASE(source_params_test) {

	cout << "=== maps_test: source_params_test ===" << endl;

	source_params_map* source_map = source_params_map::instance();
	seq_linear frequencies(1000.0, 10000.0, 1000.0);	// 1-10 KHz

	// setup sensor #1 with omni beam pattern

	sensor_params::id_type id1 = 1 ;
	source_params::reference source1( new source_params(
		id1, 		// paramsID
		false,		// multistatic
		123.0,		// source_level
		frequencies,
		0 ));		// beamID
	source_map->insert(source1->paramsID(), source1);

	// setup sensor #2 with bad beam pattern

	sensor_params::id_type id2 = 2 ;
	source_params::reference source2( new source_params(
		id2, 		// paramsID
		true,		// multistatic
		321.0,		// source_level
		frequencies,
		999 ));		// beamID
	source_map->insert(source2->paramsID(), source2);

	// test retrieval

	source_params::reference spm1 = source_map->find(1);
	source_params::reference spm2 = source_map->find(2);
	BOOST_CHECK_EQUAL(spm1, source1);
	BOOST_CHECK_EQUAL(spm2, source2);

	// check beam patterns

	beam_pattern_model::reference bpm1 = spm1->beam_pattern() ;
	beam_pattern_model::reference bpm2 = spm2->beam_pattern() ;
	BOOST_CHECK_EQUAL(bpm1->beamID(), 0);
	BOOST_CHECK_EQUAL(bpm2.get(), (beam_pattern_model*) 0);

	// cleanup inserted source_params so that other tests start fresh

	source_map->erase(id1);
	source_map->erase(id2);
}

/**
 * @ingroup sensors_test
 * Test the ability to instantiate a receiver_params_map
 * and insert several receiver_params into it.
 * Also test the find method.
 * Generate errors if pointer values are not equal.
 */
BOOST_AUTO_TEST_CASE(receiver_params_test) {

	cout << "=== maps_test: receiver_params_test ===" << endl;

	receiver_params_map* receiver_map = receiver_params_map::instance();
	seq_linear frequencies(1000.0, 10000.0, 1000.0);	// 1-10 KHz

	std::list<beam_pattern_model::id_type> beamList;
	beamList.push_back(0);
	beamList.push_back(1);

	// setup sensor #1 with omni beam pattern

	sensor_params::id_type id1 = 1 ;
	receiver_params::reference receiver1( new receiver_params(
		id1, 		// paramsID
		false,		// multistatic
		beamList ));
	receiver_map->insert(receiver1->paramsID(), receiver1);

	// setup sensor #2 with bad beam pattern

	sensor_params::id_type id2 = 2 ;
	receiver_params::reference receiver2( new receiver_params(
		id2, 		// paramsID
		true,		// multistatic
		beamList ));
	receiver_map->insert(receiver2->paramsID(), receiver2);

	// test retrieval

	receiver_params::reference spm1 = receiver_map->find(1);
	receiver_params::reference spm2 = receiver_map->find(2);
	BOOST_CHECK_EQUAL(spm1, receiver1);
	BOOST_CHECK_EQUAL(spm2, receiver2);

	// check beam patterns

	beam_pattern_model::reference bpm1 = spm1->beam_pattern(0) ;
	beam_pattern_model::reference bpm2 = spm2->beam_pattern(1) ;
	BOOST_CHECK_EQUAL(bpm1->beamID(), 0);
	BOOST_CHECK_EQUAL(bpm2.get(), (beam_pattern_model*) 0);

	// cleanup inserted receiver_params so that other tests start fresh

	receiver_map->erase(id1);
	receiver_map->erase(id2);
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

    sensor_manager* sensor_mgr = sensor_manager::instance();

	seq_linear frequencies(1000.0, 10000.0, 1000.0);	// 1-10 KHz

	std::list<beam_pattern_model::id_type> beamList;
	beamList.push_back(0);
	beamList.push_back(1);

	// setup sensor #101 with omni beam pattern

	sensor_params::id_type params1 = 1 ;
	source_params::reference source1( new source_params(
		params1,	// paramsID
		false,		// multistatic
		123.0,		// source_level
		frequencies,
		0 ));		// beamID
	source_params_map::instance()->insert(source1->paramsID(), source1);

	sensor::id_type id1 = 101 ;
	sensor::reference sensor1( new sensor( id1, params1, "source_101" ) ) ;
	sensor_mgr->insert(id1,sensor1) ;

	// setup sensor #212 with bad beam pattern

	sensor_params::id_type params2 = 2 ;
	receiver_params::reference receiver2( new receiver_params(
		params2,	// paramsID
		true,		// multistatic
		beamList ));
	receiver_params_map::instance()->insert(receiver2->paramsID(), receiver2);

	sensor::id_type id2 = 212 ;
	sensor::reference sensor2( new sensor( id2, params2, "receiver_212" ) );
	sensor_mgr->insert(id2,sensor2) ;

	// update sensor #101 with new data

    sensor::reference m1 = sensor_mgr->find(id1);
	wposition1 pos( 1.0, 2.0, 3.0 ) ;			// arbitrary location
	sensor_orientation orient( 4.0, 5.0 ) ;		// tilt and direction
	if ( ! sensor_mgr->update_sensor(id1,pos,orient,true) ) {
		BOOST_FAIL("sensor_test::Failed to update sensor!");
	}

	// cleanup inserted objects so that other tests start fresh

	source_params_map::instance()->erase(params1);
	receiver_params_map::instance()->erase(params2);
	sensor_mgr->erase(id1) ;
	sensor_mgr->erase(id2) ;
}

BOOST_AUTO_TEST_SUITE_END()
