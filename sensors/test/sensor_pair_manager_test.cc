/**
 * @example sensors/test/sensor_pair_manager_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <boost/progress.hpp>

#include <iostream>

#include <usml/sensors/sensorIDType.h>
#include <usml/sensors/sensor.h>
#include <usml/sensors/sensor_pair_manager.h>
#include <usml/eigenverb/source_data.h>
#include <usml/eigenverb/receiver_data.h>

BOOST_AUTO_TEST_SUITE(sensor_pair_manager_test)

using namespace boost::unit_test;

using namespace usml::sensors;
using namespace usml::eigenverb;

/**
* @ingroup sensors_test
* 
*/
BOOST_AUTO_TEST_CASE(init_test) {

    cout << "=== sensor_pair_map_test: init_test ===" << endl;

    sensor_pair_manager* spManager = sensor_pair_manager::instance();

    sensorIDType sources[] = {1, 3, 4, 7};

    sensorIDType receivers[] = { 1, 5, 6, 8 };

    xmitRcvModeType sensor_type = usml::sensors::SOURCE;

    spManager->add_sensor(sources[0], sensor_type );

    for ( unsigned i = 0; i < sizeof(sources)/sizeof(sensorIDType); ++i )
    {
        for ( unsigned j = 0; j < sizeof(receivers)/sizeof(sensorIDType) ; ++j )
        {
            spManager->add_sensor_pair(sources[i], receivers[j]);
        }
    }

    //spMap->find(1);

//    for (outer_map_type::iterator oi = spMap->begin(); oi != spMap->end(); ++oi)
//    {
//        inner_map_type &im = oi->second;
//        for (inner_map_type::iterator ii = im.begin(); ii != im.end(); ++ii)
//        {
//            cout << "_map[" << oi->first << "][" << ii->first << "] =" << endl;
//        }
//    }

    // Run with valgrind memcheck to verify.
    delete spManager;
}

BOOST_AUTO_TEST_SUITE_END()
