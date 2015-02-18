/**
 * @example sensors/test/beam_pattern_map_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <boost/progress.hpp>

#include <iostream>

#include <usml/sensors/beams.h>
#include <usml/sensors/beam_pattern_map.h>


BOOST_AUTO_TEST_SUITE(beam_pattern_map_test)

using namespace boost::unit_test;
using namespace std;

using namespace usml::sensors;

/**
* @ingroup sensors_test
* Test the ability to instantiate a beam_pattern_map
* and insert several beam_pattern_models into it.
* Also test the find method and the destructor.
* Generate errors if pointer values are not equal.
* For Destructor testing run with Valgrind memcheck.
*/
BOOST_AUTO_TEST_CASE(base_test) {

    cout << "=== beam_patterm_map_test: base_test ===" << endl;

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

BOOST_AUTO_TEST_SUITE_END()
