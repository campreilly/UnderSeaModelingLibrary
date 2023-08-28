/**
 * @example waveq3d/test/waveq3d_test.cc
 */

#include <usml/eigenrays/eigenrays.h>
#include <usml/ocean/ocean.h>
#include <usml/types/types.h>
#include <usml/ublas/math_traits.h>
#include <usml/waveq3d/wave_queue.h>
#include <usml/waveq3d/wave_thresholds.h>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/geometry/geometry.hpp>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <list>
#include <sstream>
#include <string>

BOOST_AUTO_TEST_SUITE(waveq3d_test)

using namespace usml::eigenrays;
using namespace usml::ocean;
using namespace usml::waveq3d;

static const double time_step = 0.100;
static const double src_lat = 45.0;  // location = mid-Atlantic
static const double src_lng = -45.0;
static const double c0 = 1500.0;  // constant sound speed

/**
 * @ingroup waveq3d_test
 * @{
 */

/**
 * Tests the accuracy of eigenray dead reckoning in a deep sound channel.
 * Creates a source at 45N 45E and a receiver 1 deg east of that position.
 * Then it creates a second receiver 0.01 deg closer than that receiver.
 * Compares the eigenrays created with wave_queue to those estimated by
 * dirpath_collection::dead_reckon().  When debugging is turned on, it also
 * writes the wavefront, eigenrays, dirpaths, and dead reckoned eigenrays
 * to netCDF files for analysis in Matlab.
 *
 * Angle estimates are only expected to be accurate within 1 deg, because they
 * are just copied from the 46E location to the 45.99E one. So we expect some
 * errors in the angle estimate.
 *
 * The dead reckoned eigenrays are written to disk without a source_id or any
 * target_ids just to test our ability to leave these as defaults.
 */
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
BOOST_AUTO_TEST_CASE(dead_reckon) {
    cout << "=== waveq3d_test: dead_reckon ===" << endl;
    const char *ncold = USML_TEST_DIR "/waveq3d/test/dead_reckon_old.nc";
    const char *ncnew = USML_TEST_DIR "/waveq3d/test/dead_reckon_new.nc";

    // build a simple ocean with a deep sound channel

    boundary_model::csptr bottom(new boundary_flat(4000.0));
    boundary_model::csptr surface(new boundary_flat());
    profile_model::csptr profile(new profile_munk());
    ocean_model::csptr ocean(new ocean_model(surface, bottom, profile));

    // define propagation parameters

    const double time_max = 90.0;
    const double time_step = 0.1;
    seq_vector::csptr freq(new seq_log(10.0, 2.0, 1280.0));
    wposition1 pos(45.0, 45.0, -1000.0);
    seq_vector::csptr de(new seq_rayfan(-10.0, 10.0));
    seq_vector::csptr az(new seq_linear(89.0, 0.5, 91.0));

    // create target 1 deg away in longitude, and a 2nd 0.01 deg closer

    wposition targets(1, 2, 45.0, 46.0, -1200.0);
    targets.longitude(0, 1, 45.99);
    wposition new_targets(1, 1, 45.0, 45.99, -1200.0);

    // test ability to write target id values to disk

    matrix<int> target_ids(1, 2);
    for (int n = 0; n < target_ids.size2(); n++) {
        target_ids(0, n) = n + 2;
    }

    // propagate wavefronts and collect eigenrays

    eigenray_collection eigenrays(freq, pos, &targets, 1, target_ids);
    wave_queue wave(ocean, freq, pos, de, az, time_step, &targets);
    wave.add_eigenray_listener(&eigenrays);
    cout << "propagate wavefronts for " << time_max << " secs" << endl;
    while (wave.time() < time_max) {
        wave.step();
    }
    eigenrays.sum_eigenrays();
    cout << "writing eigenrays to " << ncold << endl;
    eigenrays.write_netcdf(ncold);

    // create and dead reckon direct paths, then create an eigenray_collection
    // for these rays so that we can write them to disk

    eigenray_list eigen_reckon = eigenrays.dead_reckon(
        0, 0, pos, wposition1(new_targets, 0, 0), profile);
    eigenray_collection newrays(freq, pos, &new_targets);
    for (const auto &ray : eigen_reckon) {
        newrays.add_eigenray(0, 0, ray);
    }
    newrays.sum_eigenrays();
    cout << "writing new eigenrays to " << ncnew << endl;
    newrays.write_netcdf(ncnew);

    // compare dead reckoned result to modeled result

    auto theory = eigenrays.eigenrays(0, 1).begin();
    for (const auto &ray : eigen_reckon) {
        BOOST_CHECK_SMALL(ray->time - (*theory)->time, 0.01);
        BOOST_CHECK_SMALL(ray->source_de - (*theory)->source_de, 1.0);
        BOOST_CHECK_SMALL(ray->source_az - (*theory)->source_az, 1.0);
        BOOST_CHECK_SMALL(ray->target_de - (*theory)->target_de, 1.0);
        BOOST_CHECK_SMALL(ray->target_az - (*theory)->target_az, 1.0);
        BOOST_CHECK_EQUAL(ray->surface, (*theory)->surface);
        BOOST_CHECK_EQUAL(ray->bottom, (*theory)->bottom);
        BOOST_CHECK_EQUAL(ray->caustic, (*theory)->caustic);
        BOOST_CHECK_EQUAL(ray->upper, (*theory)->upper);
        BOOST_CHECK_EQUAL(ray->lower, (*theory)->lower);
        ++theory;
        if (theory == eigenrays.eigenrays(0, 1).end()) {
            break;
        }
    }
}

/**
 * Tests the accuracy of the eigenverb contributions against analytic solution
 *
 *   - Profile: constant 1500 m/s sound speed, Thorp absorption
 *   - Bottom: 1000 meters, sand
 *   - Source: 45N, 45W, on surface, 1000 Hz
 *   - Interfaces: bottom, surface, and volume
 *   - Time Step: 100 msec
 *   - Launch D/E: 5 degree linear spacing from -60 to 60 degrees
 *   - Launch AZ: 10 degree linear spacing from -40 to 40 degrees
 *
 * Automatically checks the accuracy of the eigenverbs for the bottom
 * to the analytic solution in the reverberation paper.
 *
 * To maximize accuracy we compute path length and angles on a round earth
 * with a flat bottom, using eqn. (25) - (27) from the verification test report.
 * For a path with a given DE (where negative is down), the path length for
 * the first interaction with the bottom is found by solving eqn. (25) for L:
 * <pre>
 *     Rb^2 = R^2 + L^2 - 2 R L sin(DE)
 *     L^2 - 2 R L sin(DE) + (R^2 - Rb^2) = 0
 * </pre>
 * where
 *
 * 		- R = source distance from earth center,
 *      - Rb = bottom distance from earth center,
 *		- DE = launch D/E angle, and
 *		- P = path length.
 *
 * The quadratic equation solution for the path length is
 * <pre>
 * 		p = R sin(abs(DE))
 * 		q = R^2 - Rb^2
 * 		L = p - sqrt( p*p - q )
 * </pre>
 * The negative root has been chosen to make an acute angle between Rs and Rb.
 * The angle between Rs and Rb is given by:
 * <pre>
 * 		L^2 = R^2 + Rb^2 - 2 R Rb cos(alpha) ;
 * 		alpha = acos[ ( Rs^2 + Rb^2 - L^2 ) / (2 Rs Rb) ]
 * </pre>
 * The time of arrival and grazing angle are given by:
 * <pre>
 * 		time = L / c ;
 * 		grazing = DE - alpha
 * </pre>
 * The length and width of the eigenverb are computed using
 * <pre>
 * 		length = L * dDe / sin(grazing)
 * 		width = L * dAZ * cos(DE)
 * 	</pre>
 * where
 * 		- dDE = initial spacing between rays in DE direction (radians)
 * 		- dAZ = initial spacing between rays in AZ direction (radians)
 *
 * @xref Sean Reilly, David Thibaudeau, Ted Burns,
 * 		 "Fast computation of reverberation using Gaussian beam
 *		 reflections", report prepared for NAWCTSD
 * @xref Sean Reilly, Gopu Potty, "Verification Tests for Hybrid Gaussian
 *		 Beams in Spherical/Time Coordinates", 10 May 2012.
 */
BOOST_AUTO_TEST_CASE(eigenverb_accuracy) {
    cout << "=== eigenverb_collection_test: eigenverb_accuracy ===" << endl;
    const char *ncname = USML_TEST_DIR "/waveq3d/test/eigenverb_accuracy_";
    const double time_max = 3.5;
    const double depth = 1000.0;
    const double de_spacing = 5.0;
    const double az_spacing = 10.0;

    // initialize propagation model

    profile_model::csptr profile(new profile_linear(c0));
    boundary_model::csptr surface(new boundary_flat());
    reflect_loss_model::csptr bottom_loss(
        new reflect_loss_rayleigh(bottom_type_enum::sand));
    boundary_model::csptr bottom(new boundary_flat(depth, bottom_loss));
    auto *my_ocean = new ocean_model(surface, bottom, profile);
    volume_model::csptr layer(new volume_flat(300.0, 10.0, -40.0));
    my_ocean->add_volume(layer);
    ocean_model::csptr ocean(my_ocean);

    seq_vector::csptr freq(new seq_log(1000.0, 10.0, 1));
    wposition1 pos(src_lat, src_lng, 0.0);
    seq_vector::csptr de(new seq_linear(-80.0, de_spacing, 60.0));
    seq_vector::csptr az(new seq_linear(-40.0, az_spacing, 40.1));

    // build a wavefront that just generates eigenverbs

    eigenverb_collection eigenverbs(ocean->num_volume());
    wave_queue wave(ocean, freq, pos, de, az, time_step);
    wave.add_eigenverb_listener(&eigenverbs);

    while (wave.time() < time_max) {
        wave.step();
    }

    // record eigenverbs for each interface to their own disk file

    for (int n = 0; n < eigenverbs.num_interfaces(); ++n) {
        std::ostringstream filename;
        filename << ncname << n << ".nc";
        eigenverbs.write_netcdf(filename.str().c_str(), n);
    }

    // test the accuracy of the eigenverb contributions
    // just tests downward facing rays to the bottom, along az=0

    for (const auto &verb : eigenverbs.eigenverbs(eigenverb_model::BOTTOM)) {
        if (verb->source_de < 0.0 && verb->source_az == 0.0) {
            // compute path length to first bottom bounce on a spherical earth

            int segments = verb->bottom + verb->surface + 1;
            double R = wposition::earth_radius;
            double Rb = wposition::earth_radius - depth;
            double p = R * sin(abs(verb->source_de));
            double q = R * R - Rb * Rb;
            double path_length = p - sqrt(p * p - q);  // quadratic equation

            // compute gazing angle, complete path length, and time of arrival

            double alpha = acos((R * R + Rb * Rb - path_length * path_length) /
                                (2 * R * Rb));
            double grazing = abs(verb->source_de) - alpha;
            path_length *= segments;
            double time = path_length / c0;

            // compute height, width, and area area centered on ray

            const double de_angle = verb->source_de;
            const double de_plus = de_angle + 0.25 * to_radians(de_spacing);
            const double de_minus = de_angle - 0.25 * to_radians(de_spacing);

            const double az_angle = verb->source_az;
            const double az_plus = az_angle + 0.25 * to_radians(az_spacing);
            const double az_minus = az_angle - 0.25 * to_radians(az_spacing);

            const double area =
                (sin(de_plus) - sin(de_minus)) * (az_plus - az_minus);
            const double de_delta = de_plus - de_minus;  // average height
            const double az_delta = area / de_delta;     // average width

            double verb_length = path_length * de_delta / sin(grazing);
            double verb_width = path_length * az_delta;

            // compare to results computed by model

            cout << std::fixed << std::setprecision(4)
                 << "de=" << to_degrees(verb->source_de)
                 << " s=" << verb->surface << " b=" << verb->bottom
                 << "\tL=" << verb->length << " theory=" << verb_length
                 << "\tW=" << verb->width << " theory=" << verb_width
                 << endl;
            BOOST_CHECK_SMALL(verb->time - time, 1e-3);
            BOOST_CHECK_SMALL(verb->grazing - grazing, 1e-6);
            BOOST_CHECK_SMALL(verb->direction - verb->source_az, 1e-6);
            BOOST_CHECK_SMALL(verb->length - verb_length, 0.1);
            BOOST_CHECK_SMALL(verb->width - verb_width, 0.1);
        }
    }
}

/**
 * Tests the the eigenverb generation process using conditions like
 * those used in the eigenverb_demo.m scenario.
 *
 *   - Profile: constant 1500 m/s sound speed, no absorption
 *   - Bottom: 200 meters, sand
 *   - Source: 45N, 45W, on surface, 1000 Hz
 *   - Interfaces: bottom and surface, but limit to 2 bounces
 *   - Time Step: 100 msec
 *   - Launch D/E: 91 tangent spaced rays from -90 to +90 degrees
 *   - Launch AZ: Rays in the range [0,360) with 20 degree spacing.
 *
 * The primary motivation for this test is to generate an eigenverb netCDF
 * file that can be used to support off-line comparisons to the
 * eigenverb_demo.m scenario. The secondary motivation is to test un-even
 * ray spacing and test limiting the outputs to direct path. In addition to
 * these goals, it also automatically checks the accuracy of the eigenverbs
 * for the bottom to the analytic solution in the reverberation paper.
 */
BOOST_AUTO_TEST_CASE(eigenverb_demo) {
    cout << "=== envelope_test: eigenverb_demo ===" << endl;
    const char *ncname = USML_TEST_DIR "/waveq3d/test/eigenverb_demo_";
    const char *ncname_wave =
        USML_TEST_DIR "/waveq3d/test/eigenverb_demo_wave.nc";
    const double time_max = 4.0;
    const double depth = 200.0;

    // initialize propagation model

    attenuation_model::csptr attn(new attenuation_constant(0.0));
    profile_model::csptr profile(new profile_linear(c0, attn));
    boundary_model::csptr surface(new boundary_flat());
    reflect_loss_model::csptr bottom_loss(
        new reflect_loss_rayleigh(bottom_type_enum::sand));
    boundary_model::csptr bottom(new boundary_flat(depth, bottom_loss));
    ocean_model::csptr ocean(new ocean_model(surface, bottom, profile));

    seq_vector::csptr freq(new seq_log(1000.0, 10.0, 1));
    wposition1 pos(src_lat, src_lng, 0.0);
    seq_vector::csptr de(new seq_rayfan(-90.0, 90.0, 181));
    seq_vector::csptr az(new seq_linear(0.0, 20.0, 359.0));

    // build a wavefront that just generates eigenverbs

    eigenverb_collection eigenverbs(ocean->num_volume());
    wave_queue wave(ocean, freq, pos, de, az, time_step);
    wave.add_eigenverb_listener(&eigenverbs);
    wave.max_bottom(2);
    wave.max_surface(2);

    cout << "writing wavefronts to " << ncname_wave << endl;
    wave.init_netcdf(ncname_wave);
    wave.save_netcdf();
    while (wave.time() < time_max) {
        wave.step();
        wave.save_netcdf();
    }
    wave.close_netcdf();

    // record eigenverbs for each interface to their own disk file

    for (int n = 0; n < eigenverbs.num_interfaces(); ++n) {
        std::ostringstream filename;
        filename << ncname << n << ".nc";
        eigenverbs.write_netcdf(filename.str().c_str(), n);
    }

    // test the accuracy of the eigenverb contributions
    // just tests downward facing rays to the bottom, along az=0
    // because those are the rays which we have analytic solutions for

    for (const auto &verb : eigenverbs.eigenverbs(eigenverb_model::BOTTOM)) {
        if (verb->source_de < 0.0 && verb->source_az == 0.0 &&
            verb->surface == 0 && verb->bottom == 0) {
            // compute path length to first bottom bounce on a spherical earth

            int segments = verb->bottom + verb->surface + 1;
            double R = wposition::earth_radius;
            double Rb = wposition::earth_radius - depth;
            double p = R * sin(abs(verb->source_de));
            double q = R * R - Rb * Rb;
            double path_length = p - sqrt(p * p - q);  // quadratic equation

            // compute gazing angle, complete path length, and time of arrival

            double alpha = acos((R * R + Rb * Rb - path_length * path_length) /
                                (2 * R * Rb));
            double grazing = abs(verb->source_de) - alpha;
            path_length *= segments;
            double time = path_length / c0;

            // compute height, width, and area area centered on ray

            const double de_angle = verb->source_de;
            const double de_plus =
                de_angle + 0.25 * to_radians(de->increment(verb->de_index));
            const double de_minus =
                de_angle - 0.25 * to_radians(de->increment(verb->de_index - 1));

            const double az_angle = verb->source_az;
            const double az_plus =
                az_angle + 0.25 * to_radians(az->increment(verb->az_index));
            const double az_minus =
                az_angle - 0.25 * to_radians(az->increment(verb->az_index - 1));

            const double area =
                (sin(de_plus) - sin(de_minus)) * (az_plus - az_minus);
            const double de_delta = de_plus - de_minus;  // average height
            const double az_delta = area / de_delta;     // average width

            double verb_length = path_length * de_delta / sin(grazing);
            double verb_width = path_length * az_delta;

            // compare to results computed by model
            // - accuracy of length/width just based on prior measurements,
            // - length/width errors as high as 0.1 meters would still be good

            cout << std::fixed << std::setprecision(4)
                 << "de=" << to_degrees(verb->source_de)
                 << " s=" << verb->surface << " b=" << verb->bottom
                 << "\tL=" << verb->length << " theory=" << verb_length
                 << "\tW=" << verb->width << " theory=" << verb_width
                 << endl;
            BOOST_CHECK_SMALL(verb->time - time, 1e-3);
            BOOST_CHECK_SMALL(verb->grazing - grazing, 1e-6);
            BOOST_CHECK_SMALL(verb->direction - verb->source_az, 1e-6);
            BOOST_CHECK_SMALL(verb->length - verb_length, 0.1);
            BOOST_CHECK_SMALL(verb->width - verb_width, 0.5);
        }
    }
}

/// @}
BOOST_AUTO_TEST_SUITE_END()
