/**
 * @example sensors/test/maps_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <boost/progress.hpp>

#include <iostream>

#include <usml/sensors/singleton_map.h>
#include <usml/sensors/beams.h>
#include <usml/sensors/beam_pattern_map.h>
#include <usml/sensors/source_params.h>
#include <usml/sensors/source_params_map.h>


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
*/
BOOST_AUTO_TEST_CASE(beam_pattern_test) {

    cout << "=== maps_test: beam_pattern_test ===" << endl;

    beam_pattern_map* bpMap = beam_pattern_map::instance();
    beam_pattern_model* beamModelHeap1 = new beam_pattern_omni();
   
    bpMap->insert(1, beamModelHeap1);

    beam_pattern_model* beamModelHeap2 = new beam_pattern_omni();

    bpMap->insert(2, beamModelHeap2);

    const usml::sensors::beam_pattern_model* bpm1 = bpMap->find(1);

    BOOST_CHECK_EQUAL(bpm1, beamModelHeap1);

    const usml::sensors::beam_pattern_model* bpm2 = bpMap->find(2);

    BOOST_CHECK_EQUAL(bpm2, beamModelHeap2);

    // Check key not found returns null
    const usml::sensors::beam_pattern_model* bpm = bpMap->find(3);

    BOOST_CHECK_EQUAL(bpm, (const usml::sensors::beam_pattern_model*)0);

    // Run with valgrind memcheck to verify.
    delete bpMap;
}

///**
//* @ingroup sensors_test
//* Test the ability to instantiate a source_params_map
//* and insert several source_params into it.
//* Also test the find method and the destructor.
//* Generate errors if pointer values are not equal.
//* For Destructor testing run with Valgrind memcheck.
//*/
//BOOST_AUTO_TEST_CASE(source_params_test) {
//
//    cout << "=== maps_test: source_params_test ===" << endl;
//
//    source_params_map* spMap = source_params_map::instance();
//    source_params* sourceParamsHeap1 = new source_params();
//
//    // Insert
//    (*spMap)[1] = sourceParamsHeap1;
//    //spMap->insert(1, sourceParamsHeap1);
//
//    source_params* sourceParamsHeap2 = new source_params();
//
//    // Insert
//    (*spMap)[2] = sourceParamsHeap2;
//    //spMap->insert(2, sourceParamsHeap2);
//
//    //spMap->find(1);
//    const usml::sensors::source_params* spm1 = spMap->find(1)->second;
//
//    BOOST_CHECK_EQUAL(spm1, sourceParamsHeap1);
//
//    const usml::sensors::source_params* spm2 = spMap->find(2)->second;
//
//    BOOST_CHECK_EQUAL(spm2, sourceParamsHeap2);
//
//    // Check key not found returns end()
//    std::map <const paramsIDType, const source_params*>::iterator iter;
//    iter = spMap->find(3);
//    if (iter != spMap->end()) {
//        BOOST_ERROR("Found value other than end");
//    }
//
//    // Accessing a non-existing element creates it
//    if ((*spMap)[4] == spm2) {
//        std::cout << "Oha!\n";
//    }
//
//    iter = spMap->find(4);
//    if (iter != spMap->end()) {
//        cout << "Created non-existing element by accessing it - source_params* = " << iter->second << endl;
//    }
//    // BOOST_CHECK_EQUAL(spm, (const usml::sensors::source_params*)0);
//
//    // Run with valgrind memcheck to verify.
//    delete spMap;
//}

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

    singleton_map< paramsIDType, source_params >* spMap = source_params_map::instance();
    source_params* sourceParamsHeap1 = new source_params();

    // Insert
    spMap->insert(1, sourceParamsHeap1);

    source_params* sourceParamsHeap2 = new source_params();

    // Insert
    spMap->insert(2, sourceParamsHeap2);

    const usml::sensors::source_params* spm1 = spMap->find(1);

    BOOST_CHECK_EQUAL(spm1, sourceParamsHeap1);

    const usml::sensors::source_params* spm2 = spMap->find(2);

    BOOST_CHECK_EQUAL(spm2, sourceParamsHeap2);

    // Check key not found returns null
    const usml::sensors::source_params* spm = spMap->find(3);

    BOOST_CHECK_EQUAL(spm, (const usml::sensors::source_params*)0);

    // Run with valgrind memcheck to verify.
    delete spMap;
}

BOOST_AUTO_TEST_SUITE_END()
