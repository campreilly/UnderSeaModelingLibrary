/**
 * @example biverbs/test/biverbs_test.cc
 */

#include <usml/biverbs/biverb_model.h>
#include <usml/biverbs/biverbs.h>
#include <usml/eigenverbs/eigenverb_collection.h>
#include <usml/eigenverbs/eigenverb_model.h>
#include <usml/ocean/ocean_utils.h>
#include <usml/sensors/sensor_manager.h>
#include <usml/sensors/test/simple_sonobuoy.h>
#include <usml/threads/thread_task.h>
#include <usml/types/seq_linear.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition.h>
#include <usml/types/wposition1.h>
#include <usml/ublas/math_traits.h>

#include <boost/numeric/ublas/vector.hpp>
#include <boost/test/unit_test.hpp>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <list>
#include <memory>

BOOST_AUTO_TEST_SUITE(biverbs_test)

using namespace boost::unit_test;
using namespace usml::biverbs;

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

    double power = verb->length * verb->width / (slant_range * slant_range);
    verb->power = vector<double>(frequencies->size(), power);

    return eigenverb_model::csptr(verb);
}

/**
 * @ingroup biverbs_test
 * @{
 */

/**
 * Tests ability to construct bistatic eigenverbs in a background task. Builds
 * hard-coded eigenverbs on bottom for 8 different DE and 10 different AZ.
 * Test automatically fails if eigenverb_collection does not have an eigenverb
 * for each DE and AZ combination.
 *
 * Launches update_wavefront_data() background task to compute biverbs. Extracts
 * biverbs, writes them to disk, and counts entries in biverbs collection.
 *
 * TODO Compute analytic solution for biverb field values.
 */
BOOST_AUTO_TEST_CASE(update_wavefront_data) {
    cout << "=== biverbs_test: update_wavefront_data ===" << endl;
    const char* ncname = USML_TEST_DIR "/biverbs/test/";
    sensor_manager* smgr = sensor_manager::instance();

    ocean_utils::make_iso(depth);
    seq_vector::csptr frequencies(new seq_linear(3000.0, 1.0, 1));
    smgr->frequencies(frequencies);
    wposition1 source_pos(15.0, 35.0, 0.0);
    vector<double> scatter(frequencies->size(), 1.0);

    // construct monostatic sensor pair in the bistatic manager

    sensor_model* sensor_ptr = new test::simple_sonobuoy(1, "simple_sonobuoy");
    sensor_ptr->compute_reverb(true);
    sensor_model::sptr sensor(sensor_ptr);
    smgr->add_sensor(sensor);
    pair_list blist = smgr->find_source(1);
    sensor_pair::sptr pair = *(blist.begin());

    // build hard-coded eigenverbs on bottom for varying DE and AZ

    auto* verb_collection = new eigenverb_collection(eigenverb_model::BOTTOM);
    for (double az = 0.0; az <= 90.0; az += az_spacing) {
        for (double de = -90.0 + de_spacing; de < 0.0; de += de_spacing) {
            eigenverb_model::csptr verb = create_eigenverb(
                sensor->position(), depth, de, az, frequencies);
            verb_collection->add_eigenverb(verb, eigenverb_model::BOTTOM);
        }
    }
    {
        std::ostringstream filename;
        filename << ncname << "eigenverbs.nc";
        verb_collection->write_netcdf(filename.str().c_str(),
                                      eigenverb_model::BOTTOM);
    }
    eigenverb_list eigenverb_list =
        verb_collection->eigenverbs(eigenverb_model::BOTTOM);
    BOOST_CHECK_EQUAL(eigenverb_list.size(), 80);

    // launch update_wavefront_data() background task to compute biverbs
    // using empty eigenray_collection

    wposition1 pos1 = sensor->position();
    wposition pos(pos1);
    auto* ray_collection = new eigenray_collection(frequencies, pos1, pos);
    pair->update_wavefront_data(sensor.get(),
                                eigenray_collection::csptr(ray_collection),
                                eigenverb_collection::csptr(verb_collection));
    thread_task::wait();

    // extract biverbs, write to disk, and count entries in collection

    auto collection = pair->biverbs();
    biverb_list verb_list = collection->biverbs(eigenverb_model::BOTTOM);
    BOOST_CHECK_EQUAL(verb_list.size(), 182);
    BOOST_CHECK_EQUAL(collection->size(eigenverb_model::BOTTOM), 182);
    {
        std::ostringstream filename;
        filename << ncname << "biverbs_test.nc";
        collection->write_netcdf(filename.str().c_str(),
                                 eigenverb_model::BOTTOM);
    }

    sensor_manager::reset();
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
