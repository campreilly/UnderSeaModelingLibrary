/**
 * @example sensors/test/sensor_pair_map_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <boost/progress.hpp>

#include <iostream>

#include <usml/sensors/sensorIDType.h>
#include <usml/sensors/sensor.h>
#include <usml/sensors/sensor_pair_map.h>
#include <usml/eigenverb/data_collections.h>

BOOST_AUTO_TEST_SUITE(sensor_pair_map_test)

using namespace boost::unit_test;

using namespace usml::sensors;
using namespace usml::eigenverb;

/**
* @ingroup sensors_test
* 
*/
BOOST_AUTO_TEST_CASE(init_test) {

    cout << "=== sensor_pair_map_test: init_test ===" << endl;

    sensor_pair_map* spMap = sensor_pair_map::instance();
    
    data_collections data;

    sensorIDType sources[] = {1, 3, 4, 7};

    sensorIDType receivers[] = { 1, 5, 6, 8 };

    spMap->add_sensor_source(sources[0]);

    for ( unsigned i = 0; i < sizeof(sources); ++i )
    {
        for ( unsigned j = 0; j < sizeof(receivers); ++j )
        {
            spMap->add_sensor_pair(sources[i], receivers[j]);
        }
    }

    //spMap->find(1);

    for (outer_map_type::iterator oi = spMap->begin(); oi != spMap->end(); ++oi)
    {
        inner_map_type &im = oi->second;
        for (inner_map_type::iterator ii = im.begin(); ii != im.end(); ++ii)
        {
            cout << "_map[" << oi->first << "][" << ii->first << "] =" << endl;
        }
    }

    // Run with valgrind memcheck to verify.
    delete spMap;
}

BOOST_AUTO_TEST_SUITE_END()
