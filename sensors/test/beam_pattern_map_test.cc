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
* Test the ability to instanicate a beam_pattern_map and insert 
* several beam_pattern_models into it.
* Generate errors if 
*/
BOOST_AUTO_TEST_CASE(insert_test) {

    cout << "=== beam_patterm_map_test: insert_test ===" << endl;


    beam_pattern_map* bpMap = beam_pattern_map::instance();
    beam_pattern_model* beamModelHeap = new beam_pattern_omni();
   
    bpMap->insert(1, beamModelHeap);

    beamModelHeap = new beam_pattern_omni();

    bpMap->insert(2, beamModelHeap);

    usml::sensors::beam_pattern_model* bpm = bpMap->find(1);
}

/**
 * @ingroup sensors_test
 * 
 * Generate errors if 
 */
BOOST_AUTO_TEST_CASE( find_test ) {
   
    cout << "=== beam_patterm_map_test: find_test ===" << endl;
}

/**
 * @ingroup types_test
 * Generate errors if values
 */
//BOOST_AUTO_TEST_CASE( find_test ) {
//  
//}

BOOST_AUTO_TEST_SUITE_END()
