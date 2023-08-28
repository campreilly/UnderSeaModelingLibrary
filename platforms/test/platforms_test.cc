/**
 * @example platforms/test/platforms_test.cc
 */
#include <usml/platforms/platforms.h>
#include <usml/platforms/test/simple_sonobuoy.h>

#include <boost/test/unit_test.hpp>

using namespace boost::unit_test;
using namespace usml::platforms;
using namespace usml::platforms::test;

/** Tolerance for value comparisons */
const double tol = 1e-10;

BOOST_AUTO_TEST_SUITE(platforms_test)

/**
 * @ingroup platforms_test
 * @{
 */

/**
 * Test the ability to create a simple sensor.
 */
BOOST_AUTO_TEST_CASE(create_sensor) {
    cout << "=== platforms_test: create_sensor ===" << endl;
    simple_sonobuoy sensor(0,"simple_sonobuoy");
    platform_manager::reset();
}

/**
 * Test the ability to create a single platform with an attached sensor.
 * Mounts a sensor on the port side, aft of center, tilted down.
 * Then it checks that principle axes of the sensor are all facing in the
 * expected directions.  Finally, it moves the platform using the
 * platform_model::update() method and tests to see if the sensor moved to the
 * correct location,
 */
BOOST_AUTO_TEST_CASE(create_platform) {
    cout << "=== platforms_test: create_platform ===" << endl;

    platform_manager* platform_mgr = platform_manager::instance();

    // create a platform traveling north-east. */

    const int platformID = 1;
    const time_t time = 0.0;
    const double lat = 45.0;  // platform starts at 45N, 50E
    const double lng = 50.0;
    const double alt = -100.0;
    const double yaw = 45.0;  // platform is level, heading NE
    const double pitch = 0.0;
    const double roll = 0.0;
    const double speed = 10.0;  // platform moves when update(time) called

    platform_model::sptr platform(new platform_model(
        platformID, "platform", time, wposition1(lat, lng, alt),
        orientation(yaw, pitch, roll), speed));

    // mount a sensor on the port side, aft of center, tilted down

    sensor_model::sptr sensor(new sensor_model(0, "sensor"));
    platform->add_child(sensor, bvector(-1.0, -0.5, 0.0),
                     orientation(-90.0, -45.0, 0.0));
    platform_mgr->add(platform);

    // extract sensor principle axes

    vector<double> front = sensor->orient().front();
    vector<double> right = sensor->orient().right();
    vector<double> up = sensor->orient().up();
    cout << "sensor front=" << front << " right=" << right << " up=" << up
         << endl;

    // check that forward points north-west and down

    const double value = cos(to_radians(45.0));
    const double value2 = value * value;
    BOOST_CHECK_CLOSE(front(0), value2, tol);
    BOOST_CHECK_CLOSE(front(1), -value2, tol);
    BOOST_CHECK_CLOSE(front(2), -value, tol);

    // check that right points north-east, parallel to surface

    BOOST_CHECK_CLOSE(right(0), value, tol);
    BOOST_CHECK_CLOSE(right(1), value, tol);
    BOOST_CHECK_SMALL(right(2), tol);

    // check that up points north-west and up

    BOOST_CHECK_CLOSE(up(0), value2, tol);
    BOOST_CHECK_CLOSE(up(1), -value2, tol);
    BOOST_CHECK_CLOSE(up(2), value, tol);

    // move the platform and check location of sensor

    platform->update(60.0);
    wposition1 pos = sensor->position();
    BOOST_CHECK(pos.latitude() > lat);
    BOOST_CHECK(pos.longitude() > lng);
    BOOST_CHECK(pos.altitude() == alt);

    platform_manager::reset();
}
/// @}

BOOST_AUTO_TEST_SUITE_END()
