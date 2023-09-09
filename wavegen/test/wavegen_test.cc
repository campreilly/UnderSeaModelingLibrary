/**
 * @example wavegen/test/wavegen_test.cc
 */

#include <usml/eigenrays/eigenray_collection.h>
#include <usml/eigenverbs/eigenverb_collection.h>
#include <usml/managed/managed_obj.h>
#include <usml/managed/manager_template.h>
#include <usml/ocean/ocean_utils.h>
#include <usml/platforms/platform_manager.h>
#include <usml/platforms/platform_model.h>
#include <usml/sensors/sensor_manager.h>
#include <usml/sensors/sensor_model.h>
#include <usml/threads/thread_controller.h>
#include <usml/threads/thread_task.h>
#include <usml/types/seq_linear.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition1.h>
#include <usml/wavegen/wavefront_listener.h>

#include <boost/test/unit_test.hpp>
#include <chrono>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

BOOST_AUTO_TEST_SUITE(wavegen_test)

using namespace boost::unit_test;
using namespace usml::wavegen;

const int month = 8;        // August
const double south = 34.0;  // Malta Escarpment
const double north = 38.0;
const double west = 15.0;
const double east = 19.0;

/**
 * Listen for eigenray updates on sensor.
 */
class sensor_listener : public wavefront_listener {
   public:
    /**
     * Notify listener of new wavefront data (eigenrays and eigenverbs) have
     * been computed for a sensor.
     *
     * @param source 		Sensor model that generated this wavefront data.
     * @param eigenrays 	Shared pointer to an eigenrays computed.
     * @param eigenverbs 	Shared pointer to an eigenverbs computed.
     */
    void update_wavefront_data(
        const sensor_model* sensor, eigenray_collection::csptr eigenrays,
        eigenverb_collection::csptr /*eigenverbs*/) override {
        // write eigenrays to netCDF file

        std::string fullname = USML_TEST_DIR "/platforms/test/" +
                               sensor->description() + "_eigenrays.nc";
        const char* filename = fullname.c_str();
        cout << "writing eigenrays to " << filename << endl;
        eigenrays->write_netcdf(filename);

        // check to see if eigenrays created correctly

        BOOST_CHECK_EQUAL(eigenrays->size1(), 5);
        BOOST_CHECK_EQUAL(eigenrays->size2(), 1);
    }
};

/**
 * @ingroup wavegen_test
 * @{
 */

/**
 * This test computes eigenrays from a receiver to all sources.
 * It is used as a fundamental test to see if entries can be created
 * and if notifications work properly.
 */
BOOST_AUTO_TEST_CASE(propagate_wavefront) {
    cout << "=== wavegen_test: propagate_wavefront ===" << endl;
    ocean_utils::make_basic(south, north, west, east, month);
    sensor_manager* smgr = sensor_manager::instance();
    sensor_listener listener;

    // define frequencies for calculation

    seq_vector::csptr freq(new seq_linear(900.0, 10.0, 1000.0));
    smgr->frequencies(freq);

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

    // create platform with single omni sensor

    for (platform_model::key_type site = 1; site <= 5; ++site) {
        std::ostringstream name;
        name << "site" << site;
        const int index = site - 1;
        cout << "add sensor " << name.str() << " (" << pos[index][0] << ","
             << pos[index][1] << "," << pos[index][2] << ")" << endl;
        wposition1 position(pos[index][0], pos[index][1], pos[index][2]);
        auto* sensor = new sensor_model(site, name.str(), 0.0, position);
        sensor->time_maximum(8.0);
        sensor->compute_reverb(false);
        sensor->add_wavefront_listener(&listener);
        smgr->add_sensor(sensor_model::sptr(sensor));
    }

    // update acoustics for sensor #2

    cout << "update acoustics for sensor #2" << endl;
    platform_model::sptr platform = platform_manager::instance()->find(2);
    platform->update(0.0, platform_model::FORCE_UPDATE);
    while (thread_task::num_active() > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    cout << "clean up" << endl;
    platform_manager::reset();
    thread_controller::reset();
}

/// @}
BOOST_AUTO_TEST_SUITE_END()
