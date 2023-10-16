/**
 * @example ocean/test/ambient_test.cc
 */

#include <usml/ocean/ambient_constant.h>
#include <usml/ocean/ambient_wenz.h>
#include <usml/types/seq_log.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition1.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/vector.hpp>
#include <boost/test/unit_test.hpp>
#include <cmath>
#include <fstream>
#include <iostream>

BOOST_AUTO_TEST_SUITE(ambient_test)

using namespace boost::unit_test;
using namespace usml::ocean;
using namespace usml::types;

/**
 * @ingroup ocean_test
 * @{
 */

/**
 * Computes constant ambient noise levels as a fundamental test of code
 * functionality.
 */
BOOST_AUTO_TEST_CASE(ambient_constant_test) {
    cout << "=== ambient_wenz_test: generate constant curves ===" << endl;

    wposition1 point;
    seq_vector::csptr frequencies(new seq_log(1.0, pow(10.0, 0.1), 1e6));
    vector<double> noise(frequencies->size());

    // open output file and write header

    const char *filename =
        USML_TEST_DIR "/ocean/test/ambient_constant_test.csv";
    std::ofstream ofile(filename);
    for (double value : *frequencies) {
        ofile << value << ",";
    }
    ofile << endl;

    // compute ambient noise and save to *.csv file

    double level;
    ambient_constant model(60.0);
    model.ambient(point, frequencies, &noise);
    for (double value : noise) {
        level = 10.0 * log10(value);
        ofile << level << ",";
    }
    ofile << endl;
    ofile.close();
    cout << "results written to: " << filename << endl;

    // check levels at specific milestones

    size_t n = frequencies->find_nearest(1e3);
    level = 10.0 * log10(noise(n));
    BOOST_CHECK_SMALL(level - 60.0, 1e-3);
}

/**
 * Computes ambient noise levels for ambient_wenz model for a variety of
 * sea states, shipping levels, and rain rates. Compares results to analytic
 * solutions at specific milestones. The accuracy is limited by the facts that
 * the frequency is matched to a nearest neighbor and that the results are
 * power summed before comparison, but the analytic solutions are not.
 */
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
BOOST_AUTO_TEST_CASE(ambient_wenz_test) {
    cout << "=== ambient_wenz_test: generate wenz curves ===" << endl;

    wposition1 point;

    seq_vector::csptr frequencies(new seq_log(1.0, pow(10.0, 0.1), 1e6));
    vector<double> noise(frequencies->size());

    // open output file and write header

    const char *filename = USML_TEST_DIR "/ocean/test/ambient_wenz_test.csv";
    std::ofstream ofile(filename);
    ofile << "sea state,ship level,rain rate";
    for (double value : *frequencies) {
        ofile << "," << value;
    }
    ofile << endl;

    // compute noise for each combination of sea state, ship level, and rain

    for (int rain_rate = 0; rain_rate <= 3; ++rain_rate) {
        for (int shipping_level = 0; shipping_level <= 7; ++shipping_level) {
            for (int sea_state = 0; sea_state <= 6; ++sea_state) {
                ofile << sea_state << "," << shipping_level << "," << rain_rate;

                // NAVOCEANO model for converting sea state to wind speed

                double knots = 5.50 * sea_state - 2.70;
                if (sea_state == 0) {
                    knots = 3.25 * sea_state + 1.92;
                }
                double wind_speed = knots * 0.51444444;

                // compute ambient noise and save to *.csv file

                ambient_wenz model(wind_speed, shipping_level, rain_rate);
                model.ambient(point, frequencies, &noise);
                for (double value : noise) {
                    double level = 10.0 * log10(value);
                    ofile << "," << level;
                }
                ofile << endl;

                // check levels at specific milestones

                if (sea_state == 2 && shipping_level == 5) {
                    size_t n;
                    double level;
                    if (rain_rate == 0) {
                        n = frequencies->find_nearest(
                            1.0);  // peak of turbulence
                        level = 10.0 * log10(noise(n));
                        BOOST_CHECK_SMALL(level - 107.0, 0.1);

                        n = frequencies->find_nearest(
                            30.0);  // peak of shipping
                        level = 10.0 * log10(noise(n));
                        BOOST_CHECK_SMALL(level - 81.0, 0.1);

                        n = frequencies->find_nearest(1e4);  // middle of wind
                        level = 10.0 * log10(noise(n));
                        BOOST_CHECK_SMALL(level - 40.0, 0.3);

                        n = frequencies->find_nearest(1e6);  // peak of thermal
                        level = 10.0 * log10(noise(n));
                        BOOST_CHECK_SMALL(level - 45.0, 0.1);

                    } else if (rain_rate == 1) {
                        n = frequencies->find_nearest(1e3);  // middle of rain
                        level = 10.0 * log10(noise(n));
                        BOOST_CHECK_SMALL(level - 77.5, 0.1);
                    }
                }
            }
        }
    }
    ofile.close();
    cout << "results written to: " << filename << endl;
}
/// @}
BOOST_AUTO_TEST_SUITE_END()
