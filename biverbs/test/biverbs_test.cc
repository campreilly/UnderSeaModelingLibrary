/**
 * @example biverbs/test/biverbs_test.cc
 */

#include <usml/biverbs/biverbs.h>
#include <usml/eigenrays/eigenray_collection.h>
#include <usml/eigenverbs/eigenverb_collection.h>
#include <usml/eigenverbs/eigenverb_model.h>
#include <usml/managed/managed_obj.h>
#include <usml/ocean/ocean.h>
#include <usml/platforms/platform_manager.h>
#include <usml/platforms/platform_model.h>
#include <usml/platforms/test/simple_sonobuoy.h>
#include <usml/threads/thread_controller.h>
#include <usml/threads/thread_task.h>
#include <usml/types/seq_linear.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition1.h>
#include <usml/types/wvector1.h>
#include <usml/ublas/math_traits.h>

#include <algorithm>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/test/unit_test.hpp>
#include <usml/sensors/sensor_manager.h>
#include <usml/sensors/sensor_pair.h>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <list>
#include <memory>
#include <thread>

BOOST_AUTO_TEST_SUITE(biverbs_test)

using namespace boost::unit_test;
using namespace usml::sensors;
using namespace usml::biverbs;
using namespace usml::eigenrays;
using namespace usml::eigenverbs;
using namespace usml::platforms;

static const double de_spacing = 10.0;
static const double az_spacing = 10.0;
static const double depth = 1000.0;

/**
 * Build hard-coded eigenverb and notify listeners.
 */
static eigenverb_model::csptr create_eigenverb(
    const wposition1 source_pos, double depth, double de, double az,
    const seq_vector::csptr& frequencies) {
    auto* verb = new eigenverb_model();

    double de_rad = to_radians(de);
    double az_rad = to_radians(az);
    double grazing = abs(de_rad);
    double slant_range = depth / sin(grazing);
    double horz_range = depth / tan(grazing);

    verb->sound_speed = 1500.0;
    verb->travel_time = slant_range / verb->sound_speed;
    verb->frequencies = frequencies;
    verb->power = vector<double>(frequencies->size(), 1.0);
    verb->length = 0.5 * slant_range * to_radians(de_spacing) / sin(grazing);
    verb->width = 0.5 * slant_range * to_radians(az_spacing) * cos(grazing);
    verb->position = wposition1(source_pos, horz_range, az_rad);
    verb->position.altitude(-depth);
    verb->direction = az_rad;
    verb->grazing = grazing;
    verb->de_index = (size_t)((90.0 + de) / de_spacing - 1.0);
    verb->az_index = (size_t)(az / az_spacing);
    verb->source_de = de_rad;
    verb->source_az = az_rad;
    verb->surface = 0;
    verb->bottom = 0;
    verb->caustic = 0;
    verb->upper = 0;
    verb->lower = 0;
    return eigenverb_model::csptr(verb);
}

/**
 * @ingroup biverbs_test
 * @{
 */

/**
 * Tests ability to construct bistatic eigenverbs in a background task. Builds
 * hard-coded eigenverbs on bottom for varying DE and AZ. Launches
 * update_wavefront_data() background task to compute biverbs. Extract biverbs,
 * write to disk, and count entries in collection
 */
BOOST_AUTO_TEST_CASE(update_wavefront_data) {
    cout << "=== biverbs_test: update_wavefront_data ===" << endl;

    const char* ncname = USML_TEST_DIR "/biverbs/test/biverbs_test.nc";

    ocean_utils::make_iso(depth);
    seq_vector::csptr frequencies(new seq_linear(3000.0, 1.0, 1));
    sensor_manager::instance()->frequencies(frequencies);
    wposition1 source_pos(15.0, 35.0, 0.0);
    vector<double> scatter(frequencies->size(), 1.0);

    // construct monostatic sensor pair in the bistatic manager

    sensor_model* sensor = new test::simple_sonobuoy(0, "simple_sonobuoy");
    sensor_manager* bmgr = sensor_manager::instance();
    bmgr->add_sensor(sensor_model::sptr(sensor));
    bistatic_list blist = bmgr->find_source(0);
    sensor_pair::sptr pair = *(blist.begin());

    // build hard-coded eigenverbs on bottom for varying DE and AZ

    wposition1 pos1 = sensor->position();
    wposition pos(1, 1, pos1.latitude(), pos1.longitude(), pos1.altitude());
    auto* verb_collection = new eigenverb_collection(eigenverb_model::BOTTOM);
    for (double az = 0.0; az <= 90.0; az += az_spacing) {
        for (double de = -90.0 + de_spacing; de < 0.0; de += de_spacing) {
            eigenverb_model::csptr verb = create_eigenverb(
                sensor->position(), depth, de, az, frequencies);
            verb_collection->add_eigenverb(verb, eigenverb_model::BOTTOM);
        }
    }

    // launch update_wavefront_data() background task to compute biverbs

    auto* ray_collection = new eigenray_collection(frequencies, pos1, pos);
    pair->update_wavefront_data(sensor,
                                eigenray_collection::csptr(ray_collection),
                                eigenverb_collection::csptr(verb_collection));
    while (thread_task::num_active() > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // extract biverbs, write to disk, and count entries in collection

    auto collection = pair->biverbs();
    biverb_list verb_list = collection->biverbs(eigenverb_model::BOTTOM);
    BOOST_CHECK_EQUAL(verb_list.size(), 36);
    BOOST_CHECK_EQUAL(collection->size(eigenverb_model::BOTTOM), 36);
    collection->write_netcdf(ncname, eigenverb_model::BOTTOM);

    sensor_manager::reset();
    thread_controller::reset();
    ocean_shared::reset();
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
