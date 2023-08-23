/**
 * @example eigenverbs/test/eigenverbs_test.cc
 */
#include <usml/eigenverbs/eigenverbs.h>
#include <usml/types/types.h>

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
 * Factory that builds eigenverbs and notifies listeners.
 */
class USML_DECLSPEC eigenverb_factory : public eigenverb_notifier {
   public:
    /**
     * Build one eigenverb and notify listeners.
     */
    void create_eigenverb(const wposition1 source_pos, double depth, double de,
                          double az, const seq_vector::csptr& frequencies) {
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
        verb->length =
            0.5 * slant_range * to_radians(de_spacing) / sin(grazing);
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

        // compute bounding box

        static const double SIGMA = 1.5;
        static const double SCALE = 1.0 / (60.0 * 1852.0) * SIGMA;
        const double latitude = verb->position.latitude();
        const double longitude = verb->position.longitude();
        const double cosA = abs(cos(verb->direction));
        const double sinA = abs(sin(verb->direction));
        const double cosLat = abs(sin(verb->position.theta()));
        const double delta_lat = std::max(verb->length * cosA, verb->width * sinA) * SCALE;
        const double delta_long =
            std::max(verb->length * sinA, verb->width * cosA) * SCALE / cosLat;
        verb->bounding_box.north = latitude + delta_lat;
        verb->bounding_box.south = latitude - delta_lat;
        verb->bounding_box.east = longitude + delta_long;
        verb->bounding_box.west = longitude - delta_long;

        notify_eigenverb_listeners(eigenverb_model::csptr(verb),
                                   eigenverb_model::BOTTOM);
    }
};

/**
 * This test builds eigenverbs on bottom for varying DE and AZ. First it tests
 * eigenverb collection to see if entries can be created and if notifications
 * work properly. Then it uses the find_eigenverbs to find all eigenverbs in the
 * neighborhood of the first entry in the list.
 *
 * The basic_eigenverbs.m Matlab script overlays plots of the full list of
 * eigenrays overlaid with the results of the rtree query. The number of
 * eigenverbs decreases from 80 to 11 when the size of the overlap
 * (SIGMA in eigenverb::derived()) is 1.5.
 */
BOOST_AUTO_TEST_CASE(create_eigenverbs) {
    cout << "=== eigenverb_eigenverbs_test: create_eigenverbs ===" << endl;

    const char* ncname1 =
        USML_TEST_DIR "/eigenrays/test/create_eigenverbs.nc";
    const char* ncname2 = USML_TEST_DIR "/eigenrays/test/find_eigenverbs.nc";

    seq_vector::csptr frequencies(new seq_linear(3000.0, 1.0, 1));
    wposition1 source_pos(15.0, 35.0, 0.0);
    double depth = 1000;

    // build eigenverbs on bottom for varying DE and AZ

    eigenverb_collection collection(0);
    eigenverb_factory factory;
    factory.add_eigenverb_listener(&collection);
    for (double az = 0.0; az <= 90.0; az += az_spacing) {
        for (double de = -90.0 + de_spacing; de < 0.0; de += de_spacing) {
            factory.create_eigenverb(source_pos, depth, de, az, frequencies);
        }
    }
    cout << "writing full set of eigenverbs to " << ncname1 << endl;
    collection.write_netcdf(ncname1, eigenverb_model::BOTTOM);

    // extract eigenverbs and count entries in collection

    const eigenverb_list& verb_list =
        collection.eigenverbs(eigenverb_model::BOTTOM);
    BOOST_CHECK_EQUAL(verb_list.size(), 80);
    BOOST_CHECK_EQUAL(collection.size(eigenverb_model::BOTTOM), 80);

    // query collection for all eigenverb near a specific area

    eigenverb_model::csptr verb = verb_list.front();
    const eigenverb_collection::box box(
        eigenverb_collection::point(verb->bounding_box.south,
                                    verb->bounding_box.west),
        eigenverb_collection::point(verb->bounding_box.north,
                                    verb->bounding_box.east));
    eigenverb_list found_list =
        collection.find_eigenverbs(box, eigenverb_model::BOTTOM);
    eigenverb_collection found(0);
    for (const auto& verb_found : found_list) {
        found.add_eigenverb(verb_found, eigenverb_model::BOTTOM);
    }
    cout << "writing found eigenverbs to " << ncname1 << endl;
    found.write_netcdf(ncname2, eigenverb_model::BOTTOM);
    BOOST_CHECK_LT(found.size(eigenverb_model::BOTTOM),
                   collection.size(eigenverb_model::BOTTOM));
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
