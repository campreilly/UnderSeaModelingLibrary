/**
 * @example eigenverbs/test/eigenverbs_test.cc
 */
#include <usml/eigenverbs/eigenverbs.h>
#include <usml/types/types.h>

#include <boost/geometry/geometry.hpp>
#include <boost/test/unit_test.hpp>
#include <fstream>
#include <iostream>

BOOST_AUTO_TEST_SUITE(eigenverbs_test)

using namespace boost::unit_test;
using namespace usml::eigenverbs;

static const double de_spacing = 10.0;
static const double az_spacing = 10.0;

/**
 * @ingroup eigenverbs_test
 * @{
 */

/**
 * Build simplified eigenverb for one combination of DE and AZ.
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
    verb->time = slant_range / verb->sound_speed;
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
 * This test builds eigenverbs on bottom for varying DE and AZ. First it tests
 * eigenverb collection to see if entries can be created and if notifications
 * work properly. Then it uses the find_eigenverbs to find all eigenverbs in the
 * neighborhood of the first entry in the list.
 */
BOOST_AUTO_TEST_CASE(create_eigenverbs) {
    cout << "=== eigenverbs_test: create_eigenverbs ===" << endl;

    const char* ncname1 = USML_TEST_DIR "/eigenverbs/test/create_eigenverbs.nc";
    const char* ncname2 = USML_TEST_DIR "/eigenverbs/test/find_eigenverbs.nc";

    seq_vector::csptr frequencies(new seq_linear(3000.0, 1.0, 1));
    wposition1 source_pos(15.0, 35.0, 0.0);
    double depth = 1000;

    // build eigenverbs on bottom for varying DE and AZ

    eigenverb_collection collection(0);
    for (double az = 0.0; az <= 90.0; az += az_spacing) {
        for (double de = -90.0 + de_spacing; de < 0.0; de += de_spacing) {
            eigenverb_model::csptr verb =
                create_eigenverb(source_pos, depth, de, az, frequencies);
            collection.add_eigenverb(verb, eigenverb_model::BOTTOM);
        }
    }
    cout << "writing full set of eigenverbs to " << ncname1 << endl;
    collection.write_netcdf(ncname1, eigenverb_model::BOTTOM);

    // extract eigenverbs and count entries in collection

    eigenverb_list full_list = collection.eigenverbs(eigenverb_model::BOTTOM);
    BOOST_CHECK_EQUAL(full_list.size(), 80);
    BOOST_CHECK_EQUAL(collection.size(eigenverb_model::BOTTOM), 80);

    // query collection for all eigenverb near a specific area

    eigenverb_model::csptr bounding_verb =
        create_eigenverb(source_pos, depth, -40.0, 30.0, frequencies);
    eigenverb_list found_list =
        collection.find_eigenverbs(bounding_verb, eigenverb_model::BOTTOM);

    // put found eigenverbs in collection so we can write to disk

    eigenverb_collection found(0);
    for (const auto& verb_found : found_list) {
        found.add_eigenverb(verb_found, eigenverb_model::BOTTOM);
    }
    cout << "writing found eigenverbs to " << ncname2 << endl;
    found.write_netcdf(ncname2, eigenverb_model::BOTTOM);

    // ensure that found list is smaller than full list

    BOOST_CHECK_EQUAL(found_list.size(), 7);
    BOOST_CHECK_EQUAL(found.size(eigenverb_model::BOTTOM), 7);
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
