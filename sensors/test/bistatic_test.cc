/**
 * @example sensors/test/bistatic_test.cc
 */

#include <usml/beampatterns/bp_model.h>
#include <usml/beampatterns/bp_omni.h>
#include <usml/biverbs/biverb_collection.h>
#include <usml/eigenrays/eigenray_collection.h>
#include <usml/managed/managed_obj.h>
#include <usml/managed/manager_template.h>
#include <usml/managed/update_listener.h>
#include <usml/ocean/ocean_shared.h>
#include <usml/ocean/ocean_utils.h>
#include <usml/platforms/platform_manager.h>
#include <usml/platforms/platform_model.h>
#include <usml/sensors/sensor_manager.h>
#include <usml/sensors/sensor_model.h>
#include <usml/sensors/sensor_pair.h>
#include <usml/threads/thread_controller.h>
#include <usml/threads/thread_task.h>
#include <usml/types/seq_linear.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition1.h>

#include <boost/test/unit_test.hpp>
#include <chrono>
#include <iostream>
#include <list>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

BOOST_AUTO_TEST_SUITE(bistatic_test)

using namespace boost::unit_test;
using namespace usml::beampatterns;
using namespace usml::biverbs;
using namespace usml::eigenrays;
using namespace usml::managed;
using namespace usml::ocean;
using namespace usml::platforms;
using namespace usml::sensors;
using namespace usml::threads;
using namespace usml::types;

/**
 * Listen for eigenray updates on sensor.
 */
class pair_listener : public update_listener<sensor_pair> {
   public:
    /**
     * Notify listeners of updates to sensor_pair.
     *
     * @param pair  Reference to updated sensor_pair.
     */
    void notify_update(const sensor_pair* pair) override {
        cout << "bistatic_test::notify_update " << pair->source()->description()
             << " -> " << pair->receiver()->description() << endl;
    }
};
pair_listener test_listener;

/**
 * @ingroup sensors_test
 * @{
 */

/**
 * Tests the ability to control the production of sensor_pair objects with the
 * multistatic(), is_source(), is_receiver(), min_range(), and compute_reverb()
 * methods of the sensor_model. Uses a simple isovelocity ocean with a 2000m
 * depth and the following sensors:
 *
 * - sensor #1 = south side, monostatic, pairs are 1_1
 * - sensor #2 = center, pairs are 2_2, 2_4, 2_5
 * - sensor #3 = north side, source only, pairs are 3_2, 3_4, 3_5,
 * - sensor #4 = below #3, receiver only
 * - sensor #5 = east side, min range 1m, pairs are 5_2, 5_4
 *
 * Tests the ability use a wavefront_generator, running in the background, to
 * automatically to compute the bistatic direct path eigenrays (fathometers)
 * between these sensors. Tests the ability to write dirpath data to netCDF
 * files.
 *
 * Tests the ability use a biverb_generator, running in the background, to
 * automatically to compute the bistatic eigenverbs for pairs 2_2 and 2_4. Tests
 * the ability to exclude reverberation calculations from bistatic sensor pair
 * processing for all other pairs. Tests the ability to write biverb_model data
 * to netCDF files.
 *
 * Test automatically fails if the list of expected bistatic pairs does not
 * match the list in the documentation above or if any of the bistatic pairs
 * have less than 5 direct path eigenrays. Previous experiments showed that
 * monostatic pairs should have about 5 in this environment and that the
 * bistatic pairs have more. This difference is the result of accuracy limits in
 * the wavefront generator ray fan for paths near vertical.
 *
 * TODO: Compare acoustic paths to analytic solutions to validate number of
 * paths for each pair.
 */
BOOST_AUTO_TEST_CASE(update_wavefront_data) {
    cout << "=== bistatic_test: update_wavefront_data ===" << endl;
    const char* ncname = USML_TEST_DIR "/sensors/test/";

    //    ocean_utils::make_basic(south,north,west,east,month);
    ocean_utils::make_iso(2000.0);
    auto* platform_mgr = platform_manager::instance();
    auto* sensor_mgr = sensor_manager::instance();
    seq_vector::csptr freq(new seq_linear(900.0, 10.0, 1000.0));
    sensor_mgr->frequencies(freq);
    auto max_time = 10.0;

    // static database of sensor locations (latitude,longitude,altitude)

    // clang-format off
    static double pos[][3] = {
		{35.9, 17.0, -100},
		{36.0, 17.0, -100},
		{36.1, 17.0, -100},
		{36.1, 17.0, -500},
		{36.0, 17.1, -100},
    };
    // clang-format on
    auto num_sites = 5;
    std::string expected_pairs[] = {"1_1", "2_2", "2_4", "2_5", "3_2",
                                    "3_4", "3_5", "5_2", "5_4"};

    // create platform and sensor_pair objects.

    for (platform_model::key_type site = 1; site <= num_sites; ++site) {
        std::ostringstream name;
        name << "site" << site;
        const int index = site - 1;

        cout << "add sensor " << name.str() << " (" << pos[index][0] << ","
             << pos[index][1] << "," << pos[index][2] << ")" << endl;
        wposition1 position(pos[index][0], pos[index][1], pos[index][2]);
        auto* sensor = new sensor_model(site, name.str(), 0.0, position);
        auto beam = bp_model::csptr(new bp_omni());
        sensor->time_maximum(max_time);
        if (site != 1) {
            sensor->multistatic(1);
        }
        if (site != 3) {
            sensor->rcv_beam(0, beam);
        }
        if (site != 4) {
            sensor->src_beam(0, beam);
        }
        if (site == 2) {
            sensor->compute_reverb(true);
        }
        if (site == 4) {
            sensor->compute_reverb(true);
        }
        if (site == 5) {
            sensor->min_range(1.0);
        }
        sensor_mgr->add_sensor(sensor_model::sptr(sensor), &test_listener);
    }

    // compute acoustics in background for all sensors

    for (auto& platform : platform_mgr->list()) {
        platform->update(0.0, platform_model::FORCE_UPDATE);
    }
    while (thread_task::num_active() > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // write direct path collections to disk

    cout << endl << "*** pairs ***" << endl;
    for (const auto& pair : sensor_mgr->list()) {
        cout << pair->description()
             << " dirpaths=" << pair->dirpaths()->eigenrays().size() << endl;
        if (pair->dirpaths() != nullptr) {
            std::ostringstream filename;
            filename << ncname << "dirpaths_" << pair->hash_key() << ".nc";
            pair->dirpaths()->write_netcdf(filename.str().c_str());
        }
        if (pair->biverbs() != nullptr) {
            std::ostringstream filename;
            filename << ncname << "biverbs_" << pair->hash_key() << ".nc";
            pair->biverbs()->write_netcdf(filename.str().c_str(), 0);
        }
    }

    // check that the rights bistatic pairs created

    int n = 0;
    for (const auto& pair : sensor_mgr->list()) {
        BOOST_CHECK_EQUAL(pair->hash_key(), expected_pairs[n++]);
        BOOST_CHECK_GE(pair->dirpaths()->eigenrays().size(), 4);
    }

    // clean up and exit

    cout << "clean up" << endl;
    sensor_manager::reset();
}

/// @}
BOOST_AUTO_TEST_SUITE_END()
