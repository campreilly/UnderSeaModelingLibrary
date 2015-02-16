/**
 * @example sensors/test/beam_pattern_map_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <boost/progress.hpp>

#include <iostream>
//#include <sstream>
//#include <stdio.h>
#include <usml/sensors/beam_pattern_map.h>


BOOST_AUTO_TEST_SUITE(beam_pattern_map_test)

using namespace boost::unit_test;
using namespace std;

using namespace usml::sensors;

//namespace usml {
 //   namespace sensors {
        // Create mock beam_pattern_model
        struct beam_pattern_model {
            beam_pattern_model()  { _i = new int; *_i = 0; }
            ~beam_pattern_model() { delete _i; }
            void set(int i) { *_i = i; }
            int* _i;
        };
 //   }
//}

/**
* @ingroup sensors_test
* Test the ability to instanicate a beam_pattern_map and insert 
* several beam_pattern_models into it.
* Generate errors if 
*/
BOOST_AUTO_TEST_CASE(insert_test) {

    cout << "=== beam_patterm_map_test: insert_test ===" << endl;

    beam_pattern_map* bpMap = beam_pattern_map::instance();
    beam_pattern_model* beamModelHeap1 = new beam_pattern_model();
    beam_pattern_model* beamModelHeap2 = new beam_pattern_model();

    beamModelHeap1->set(10);

    usml::sensors::beam_pattern_model*  bpHeap1 = reinterpret_cast<usml::sensors::beam_pattern_model*> (beamModelHeap1);

    bpMap->insert(1, bpHeap1);

    beamModelHeap2->set(20);

    usml::sensors::beam_pattern_model*  bpHeap2 = reinterpret_cast<usml::sensors::beam_pattern_model*> (beamModelHeap2);

    bpMap->insert(2, bpHeap2);


    usml::sensors::beam_pattern_model* bpm = bpMap->find(1);
}

/**
 * @ingroup sensors_test
 * 
 * Generate errors if 
 */
BOOST_AUTO_TEST_CASE( find_test ) {
   
    cout << "=== beam_patterm_map_test: insert_test ===" << endl;
}

/**
 * @ingroup types_test
 * Generate errors if values
 */
//BOOST_AUTO_TEST_CASE( cubic_1d_test ) {
//  
//}

BOOST_AUTO_TEST_SUITE_END()
