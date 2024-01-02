/**
 * @example rvbts/test/rvbts_test.cc
 */

#include <usml/beampatterns/bp_model.h>
#include <usml/beampatterns/bp_omni.h>
#include <usml/biverbs/biverb_collection.h>
#include <usml/eigenrays/eigenray_collection.h>
#include <usml/eigenverbs/eigenverb_collection.h>
#include <usml/managed/managed_obj.h>
#include <usml/managed/manager_template.h>
#include <usml/managed/update_listener.h>
#include <usml/ocean/ocean_utils.h>
#include <usml/platforms/platform_manager.h>
#include <usml/platforms/platform_model.h>
#include <usml/rvbts/rvbts.h>
#include <usml/rvbts/rvbts_collection.h>
#include <usml/sensors/sensor_manager.h>
#include <usml/sensors/sensor_model.h>
#include <usml/sensors/sensor_pair.h>
#include <usml/threads/thread_task.h>
#include <usml/transmit/transmit_cw.h>
#include <usml/transmit/transmit_model.h>
#include <usml/types/seq_linear.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition1.h>

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <list>
#include <memory>
#include <sstream>
#include <string>

BOOST_AUTO_TEST_SUITE(rvbts_test)

using namespace usml::sensors;
using namespace usml::sensors;
using namespace usml::rvbts;
using namespace usml::transmit;

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
        cout << "rvbts_test::notify_update " << pair->source()->description()
             << " -> " << pair->receiver()->description() << endl;
    }
};
pair_listener test_listener;

/**
 * @ingroup rvbts_test
 * @{
 */

/**
 * Models reverberation envelope for a single bistatic pair where the receiver
 * is below the source. Uses a simple isovelocity ocean with a 2000m depth.
 */
BOOST_AUTO_TEST_CASE(update_envelope) {
    cout << "=== bistatic_test: update_envelope ===" << endl;
    const char* ncname = USML_TEST_DIR "/rvbts/test/";

    ocean_utils::make_iso(2000.0);
    auto* platform_mgr = platform_manager::instance();
    auto* sensor_mgr = sensor_manager::instance();
    seq_vector::csptr freq(new seq_linear(900.0, 100.0, 1100.0));
    sensor_mgr->frequencies(freq);
    auto max_time = 8.0;

    // static database of sensor locations (latitude,longitude,altitude)

    // clang-format off
    static double pos[][3] = {
		{36.0, 16.0, -100},
		{36.0, 16.0, -500},
    };
    // clang-format on
    auto num_sites = 2;

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
        sensor->compute_reverb(true);
        sensor->multistatic(1);
        sensor->time_maximum(max_time);
        if (site == 1) {
            sensor->src_beam(0, beam);
            std::string type1("CW");
            double duration = 0.1;
            double fcenter = 1005.0;
            double delay = 0.0;
            double source_level = 200.0;
            transmit_list transmits;
            transmits.push_back(transmit_model::csptr(new transmit_cw(
                type1, duration, fcenter, delay, source_level)));
            sensor->transmit_schedule(transmits);
        }
        if (site == 2) {
            sensor->rcv_beam(0, beam);
            // sensor->fsample(10.0);
        }
        sensor_mgr->add_sensor(sensor_model::sptr(sensor), &test_listener);
    }

    // compute acoustics in background for all sensors

    for (auto& platform : platform_mgr->list()) {
        platform->update(0.0, platform_model::FORCE_UPDATE);
    }
    thread_task::wait();

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
        if (pair->src_eigenverbs() != nullptr) {
            std::ostringstream filename;
            filename << ncname << "src_eigenverbs_" << pair->hash_key()
                     << ".nc";
            pair->src_eigenverbs()->write_netcdf(filename.str().c_str(), 0);
        }
        if (pair->rcv_eigenverbs() != nullptr) {
            std::ostringstream filename;
            filename << ncname << "rcv_eigenverbs_" << pair->hash_key()
                     << ".nc";
            pair->rcv_eigenverbs()->write_netcdf(filename.str().c_str(), 0);
        }
        if (pair->biverbs() != nullptr) {
            std::ostringstream filename;
            filename << ncname << "biverbs_" << pair->hash_key() << ".nc";
            pair->biverbs()->write_netcdf(filename.str().c_str(), 0);
        }
        if (pair->rvbts() != nullptr) {
            std::ostringstream filename;
            filename << ncname << "rvbts_" << pair->hash_key() << ".nc";
            pair->rvbts()->write_netcdf(filename.str().c_str());
        }
    }

    // clean up and exit

    cout << "clean up" << endl;
    sensor_manager::reset();
}

/// @}
BOOST_AUTO_TEST_SUITE_END()
