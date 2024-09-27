/**
 * @example eigenrays/test/eigenrays_test.cc
 */
#include <usml/eigenrays/eigenrays.h>
#include <usml/types/types.h>

#include <boost/test/unit_test.hpp>
#include <fstream>
#include <iostream>

BOOST_AUTO_TEST_SUITE(eigenrays_test)

using namespace boost::unit_test;
using namespace usml::eigenrays;

/**
 * Factory that builds eigenrays and notify listeners.
 */
class USML_DECLSPEC eigenray_factory : public eigenray_notifier {
   public:
    /**
     * Build one eigenray and notifies listeners.
     */
    void create_eigenray(const seq_vector::csptr& frequencies, size_t index) {
        auto* ray = new eigenray_model();
        // NOLINTBEGIN(bugprone-narrowing-conversions)
        ray->travel_time = 10.0 + index;
        ray->source_de = 20.0 + index;
        ray->source_az = 30.0 + index;
        ray->target_de = 40.0 + index;
        ray->target_az = 50.0 + index;
        ray->surface = 60 + index;
        ray->bottom = 70 + index;
        ray->caustic = 80 + index;
        ray->upper = 90 + index;
        ray->lower = 10 + index;
        ray->frequencies = frequencies;
        ray->intensity =
            scalar_vector<double>(frequencies->size(), 200.0 + index);
        ray->phase =
            scalar_vector<double>(frequencies->size(), (index + 1.0) / 10.0);
        // NOLINTEND(bugprone-narrowing-conversions)
        notify_eigenray_listeners(0, 0, eigenray_model::csptr(ray), 0);
    }
};

/**
 * @ingroup eigenrays_test
 * @{
 */

/**
 * This test adds entries to an eigenray collection. It is used as a
 * fundamental test to see if entries can be created and if notifications
 * work properly.
 */
BOOST_AUTO_TEST_CASE(create_eigenray) {
    cout << "=== eigenrays_test: create_eigenray ===" << endl;
    const char* ncname = USML_TEST_DIR "/eigenrays/test/create_eigenray.nc";

    // create collection with references to wave front information

    seq_vector::csptr frequencies(new seq_linear(3000.0, 1.0, 1));
    wposition1 source_pos(15.0, 35.0);
    seq_vector::csptr source_de(new seq_linear(-90.0, 5.0, 90.0));
    seq_vector::csptr source_az(new seq_linear(-180.0, 5.0, 180.0));
    wposition targets(1, 1, 12.0, 37.0);
    eigenray_collection collection(frequencies, source_pos, targets);

    // link collection to factory and create entries

    eigenray_factory factory;
    factory.add_eigenray_listener(&collection);
    for (size_t n = 0; n < 3; ++n) {
        factory.create_eigenray(frequencies, n);
    }

    // count entries in collection

    const eigenray_list& ray_list = collection.eigenrays(0, 0);
    BOOST_CHECK_EQUAL(ray_list.size(), 3);

    // sum eigenrays

    collection.sum_eigenrays();
    eigenray_model total =
        collection.total(0, 0);  // copy total instead of using reference
    BOOST_CHECK_CLOSE(total.intensity(0), 191.4, 0.1);

    collection.write_netcdf(ncname);
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
