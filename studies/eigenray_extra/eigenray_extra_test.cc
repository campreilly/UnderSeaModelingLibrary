
/**
 * @file eigenray_extra_test.cc
 *
 * Perform eigenrays tests that run too slow to be included
 * in the normal suite of regression tests.
 */
#define BOOST_TEST_MAIN
#include <usml/eigenrays/eigenray_collection.h>
#include <usml/eigenrays/eigenray_model.h>
#include <usml/ocean/attenuation_constant.h>
#include <usml/ocean/attenuation_model.h>
#include <usml/ocean/boundary_flat.h>
#include <usml/ocean/boundary_model.h>
#include <usml/ocean/ocean_model.h>
#include <usml/ocean/profile_linear.h>
#include <usml/ocean/profile_model.h>
#include <usml/types/seq_linear.h>
#include <usml/types/seq_log.h>
#include <usml/types/seq_rayfan.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition.h>
#include <usml/types/wposition1.h>
#include <usml/ublas/math_traits.h>
#include <usml/usml_config.h>
#include <usml/waveq3d/wave_queue.h>

#include <boost/test/unit_test.hpp>
#include <cmath>
#include <cstddef>
#include <iostream>

BOOST_AUTO_TEST_SUITE(eigenray_extra_test)

using namespace boost::unit_test;
using namespace usml::eigenrays;
using namespace usml::waveq3d;

static const double time_step = 0.100;
static const double f0 = 2000;
static const double src_lat = 45.0;  // location = mid-Atlantic
static const double src_lng = -45.0;
static const double c0 = 1500.0;  // constant sound speed
static const double bot_depth = 1e5;

/**
 * Tests the model's ability to accurately estimate geometric terms for
 * Lloyd's Mirror eigenrays on a spherical earth.  Performing this test in
 * spherical coordinates eliminates potential sources of error for the
 * proploss_test.cc suite, which compares its results to Cartesian test cases.
 *
 * - Scenario parameters
 *   - Profile: constant 1500 m/s sound speed, no absorption
 *   - Bottom: "infinitely" deep
 *   - Source: 200 meters deep, 2000 Hz
 *   - Target: WOA5 depths from 1-500 meters, range is 5-45 nmi
 *   - Time Step: 100 msec
 *   - Launch D/E: 181 tangent spaced rays from -90 to 90 degrees
 *
 * This test computes travel times and eigenray angles for a combination
 * of direct and surface-reflected paths in an isovelocity ocean on a
 * round earth. It searches for zones of inaccuracies in the the wavefront
 * model by comparing the modeled results to analytic solutions at a
 * variety of depths and ranges.
 *
 * To compute the analytic solution we start with:
 *
 *  - R = earth's radius
 *  - \f$ c_0 \f$ = speed of sound in the ocean
 *  - \f$ d_1 \f$ = source depth
 *  - \f$ d_2 \f$ = target depth
 *  - \f$ \xi \f$ = latitude change from source to receiver
 *
 * The laws of sines and cosines are then used to compute an analytic
 * solution for all direct-path eigenray terms:
 * \f[
 *      L^2 = D_1^2 + D_2^2 - 2 D_1 D_2 cos( \xi )
 * \f]\f[
 *      \mu_{source} = arccos \left( \frac{L^2+D_1^2+D_2^2}{2 L D_1} \right) -
 * 90 \f]\f[ \mu_{target} = arccos \left( \frac{L^2+D_2^2+D_1^2}{2 L D_2}
 * \right) - 90 \f]\f[ \tau_{direct} = L / c_0 \f] where:
 *  - \f$ L \f$ = length of direct-path (meters)
 *  - \f$ D_1 = R - d_1 \f$ = distance from earth center to source (meters)
 *  - \f$ D_2 = R - d_2 \f$ = distance from earth center to target (meters)
 *  - \f$ \mu_{source} \f$ = direct-path D/E angle at source (degrees)
 *  - \f$ \mu_{target} \f$ = direct-path D/E angle at target (degrees)
 *  - \f$ \tau_{direct} \f$ = direct-path travel time from source to target
 * (sec)
 *
 * The surface-reflected path is very complicated in spherical coordinates.
 * One way to find it is to search for the roots to the transcendental equation:
 * \f[
 *      f( \xi_1 ) = D_1 sin( \xi_1 ) - D_2 sin( \xi - \xi_1 ) + \frac{D_1
 * D_2}{R} sin( \xi - 2 \xi_1 ) = 0 \f] where
 *  - \f$ \xi_1 \f$ = latitude change from source to point of reflection
 *  - \f$ \xi_2 = \xi -\xi_1 \f$ = latitude change from reflection point to
 * target
 *
 * This test uses the Newton-Raphson method to iterate over successive values of
 * \f$ \xi_1 \f$ until a solution \f$ f( \xi_1 ) \approx 0 \f$ is found.
 * \f[
 *      f'( \xi_1 ) = D_1 cos( \xi_1 ) + D_2 cos( \xi - \xi_1 ) - 2 \frac{D_1
 * D_2}{R} cos( \xi - 2 \xi_1 ) \f]\f[ \xi_{1 new} = \xi_1 - \frac{ f( \xi_1 )
 * }{ f'( \xi_1 ) } \f]
 *
 * Plots of the transcendental equation indicate that the solution for
 * \f$ \xi_1 \f$ can have up to three roots, at long ranges, for depths near
 * that of the source.  For the purposes of analytic solution computation,
 * we will limit the range to an area where only one root is supported.
 * For a source at 200 meters, that corresponds to ranges below
 * approximately 0.8 degrees.
 *
 * Once \f$ \xi_1 \f$ and \f$ \xi_2 \f$ are known, the laws of sines and
 * cosines are used to compute an analytic solution for all surface
 * reflected eigenray terms:
 * \f[
 *      a_1^2 = R^2 + D_1^2 - 2 R D_1 cos( \xi_1 )
 * \f]\f[
 *      a_2^2 = R^2 + D_2^2 - 2 R D_2 cos( \xi_2 )
 * \f]\f[
 *      \eta_{source} = arccos \left( \frac{a_1^2+D_1^2-R^2}{2 a_1 D_1} \right)
 * - 90 \f]\f[ \eta_{target} = arccos \left( \frac{a_2^2+D_2^2-R^2}{2 a_2 D_2}
 * \right) - 90 \f]\f[ \tau_{surface} = ( a_1 + a_2 ) / c_0 \f] where:
 *  - \f$ a_1 \f$ = distance from source to point of reflection (meters)
 *  - \f$ a_2 \f$ = distance from point of reflection to target  (meters)
 *  - \f$ \eta_{source} \f$ = surface-reflected D/E angle at source (degrees)
 *  - \f$ \eta_{target} \f$ = surface-reflected D/E angle at target (degrees)
 *  - \f$ \tau_{surface} \f$ = surface-reflected travel time from source to
 * target (sec)
 *
 * Errors are automatically generated if the modeled eigenrays
 * deviate from the analytic results by more than 0.5 millisecs in time or
 * 0.2 degrees in angle.
 *
 * When the wave_queue::compute_offsets() fallback calculation of
 * offset(n) = -gradient(n) / hessian(n,n) is not limited to 1/2 of the
 * beamwidth.  This test has large errors in D/E angle.  This illustrates
 * the importance of this limitation.
 *
 * @xref Weisstein, Eric W. "Newton's Method." From MathWorld--A Wolfram
 *       Web Resource. http://mathworld.wolfram.com/NewtonsMethod.html
 */
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
BOOST_AUTO_TEST_CASE(
    eigenray_lloyds) {  // NOLINT(readability-function-cognitive-complexity)
    cout << "=== eigenray_extra_test: eigenray_lloyds ===" << endl;
    const char* ncname_wave =
        USML_STUDIES_DIR "/eigenray_extra/eigenray_lloyds_wave.nc";
    const char* ncname = USML_STUDIES_DIR "/eigenray_extra/eigenray_lloyds.nc";
    const char* analytic_name =
        USML_STUDIES_DIR "/eigenray_extra/eigenray_lloyds_analytic.nc";

    const double src_alt = -200.0;  // source depth = 200 meters
    const double time_max = 120.0;  // let rays plots go into region w/ 2 roots

    const double rmax = 45.0 / 60.0;  // limit to area where N/R converges
    const double rmin = 1.0 / 60.0;   // 1 nmi min range
    const double rinc = 1.0 / 60.0;   // 1 nmi range inc
    const seq_linear range(rmin, rinc, rmax);  // range in latitude

    static double depth[] = {0, 10, 100, 1000};
    size_t num_depths = sizeof(depth) / sizeof(double);

    //*********************************************************************
    // compute eigenrays for this ocean

    wposition::compute_earth_radius(src_lat);  // init area of ops
    attenuation_model::csptr attn(
        new attenuation_constant(0.0));  // no absorption
    profile_model::csptr profile(new profile_linear(c0, attn));  // iso-velocity
    boundary_model::csptr surface(new boundary_flat());  // default surface
    boundary_model::csptr bottom(new boundary_flat(bot_depth));  // flat bottom
    ocean_model::csptr ocean(new ocean_model(surface, bottom, profile));

    seq_vector::csptr freq(new seq_log(f0, 1.0, 1));
    wposition1 pos(src_lat, src_lng, src_alt);  // build ray source
    seq_vector::csptr de(new seq_rayfan());
    seq_vector::csptr az(new seq_linear(-4.0, 1.0, 4.0));

    // build a grid of targets at different ranges and depths

    wposition target(range.size(), num_depths, src_lat, src_lng, src_alt);
    for (size_t t1 = 0; t1 < range.size(); ++t1) {
        for (size_t t2 = 0; t2 < num_depths; ++t2) {
            target.latitude(t1, t2, src_lat + range(t1));
            target.altitude(t1, t2, -depth[t2]);
        }
    }

    // create wavefront used to create eigenrays

    eigenray_collection loss(freq, pos, target);
    wave_queue wave(ocean, freq, pos, de, az, time_step, &target);
    wave.add_eigenray_listener(&loss);

    // propagate rays & record to log files

    cout << "propagate wavefronts" << endl;
    cout << "writing wavefronts to " << ncname_wave << endl;
    wave.init_netcdf(ncname_wave);  // open a log file for wavefront data
    wave.save_netcdf();             // write ray data to log file
    while (wave.time() < time_max) {
        wave.step();
        wave.save_netcdf();  // write ray data to log file
    }
    wave.close_netcdf();  // close log file for wavefront data

    loss.sum_eigenrays();
    cout << "writing eigenrays to " << ncname << endl;
    loss.write_netcdf(ncname);

    //*********************************************************************
    // compare each target location to analytic results

    cout << "testing eigenrays" << endl;
    for (size_t t1 = 0; t1 < range.size(); ++t1) {
        for (size_t t2 = 0; t2 < num_depths; ++t2) {
            double time;
            double sde;
            double tde;
            double phase;

            // setup analytic equations for this target

            const double R = wposition::earth_radius;
            const double xi = to_radians(target.latitude(t1, t2) - src_lat);
            const double d1 = -src_alt;
            const double d2 = -target.altitude(t1, t2);
            const double D1 = R - d1;
            const double D2 = R - d2;

            for (const eigenray_model::csptr& ray : loss.eigenrays(0, 0)) {
                //*************************************************************
                // compare direct-path model to analytic results

                if (ray->surface == 0 || depth[t2] < 1e-3) {
                    // compute analytic results

                    const double L =
                        sqrt(D1 * D1 + D2 * D2 - 2.0 * D1 * D2 * cos(xi));
                    time = L / c0;
                    sde = to_degrees(
                        -asin((L * L + D1 * D1 - D2 * D2) / (2.0 * L * D1)));
                    tde = to_degrees(
                        asin((L * L + D2 * D2 - D1 * D1) / (2.0 * L * D2)));
                    phase = 0.0;
                    if (ray->surface == 1) {
                        tde *= -1.0;
                        phase = -M_PI;
                    }

                    //*************************************************************
                    // compare surface-reflected model to analytic results

                } else {
                    // find reflection point using root of transindental
                    // equation warning: xi2 = 0 for depths < 1e-3, and this
                    // makes solution unstable

                    double xi1 = xi;
                    double xi2 = xi - xi1;
                    if (abs(d2) > 0.5) {
                        xi1 = xi / 2.0;
                        xi2 = xi - xi1;
                        double f;
                        double g;
                        double delta;
                        do {
                            f = D1 * sin(xi1) - D2 * sin(xi2) +
                                D1 * D2 / R * sin(xi2 - xi1);
                            g = D1 * cos(xi1) + D2 * cos(xi2) -
                                2.0 * D1 * D2 / R * cos(xi2 - xi1);
                            delta = -f / g;
                            xi1 += delta;
                            xi2 = xi - xi1;
                        } while (abs(delta) > 1e-6);
                    }

                    // compute analytic results

                    const double a1 =
                        sqrt(R * R + D1 * D1 - 2.0 * R * D1 * cos(xi1));
                    const double a2 =
                        sqrt(R * R + D2 * D2 - 2.0 * R * D2 * cos(xi2));
                    time = (a1 + a2) / c0;
                    sde = to_degrees(
                        -asin((a1 * a1 + D1 * D1 - R * R) / (2.0 * a1 * D1)));
                    tde = to_degrees(
                        asin((a2 * a2 + D2 * D2 - R * R) / (2.0 * a2 * D2)));
                    phase = -M_PI;
                }

                //*************************************************************
                // test the accuracy of the model
                // acknowledge that there will be bigger errors at short range

                if (range(t1) >= 0.1) {
                    BOOST_REQUIRE_SMALL(ray->travel_time - time, 0.0005);
                    BOOST_CHECK_SMALL(ray->phase(0) - phase, 1e-6);
                    BOOST_CHECK_SMALL(ray->source_de - sde, 0.3);
                    BOOST_CHECK_SMALL(ray->source_az, 1e-6);
                    BOOST_CHECK_SMALL(ray->target_de - tde, 0.3);
                    BOOST_CHECK_SMALL(ray->target_az, 1e-6);
                }

#ifdef USML_DEBUG
                cout << "lat=" << range[t1] << " depth=" << depth[t2]
                     << " path=" << ray->surface;
                cout << " time=" << time << " sd=" << sde << " tde=" << tde
                     << endl;
#endif
            }  // loop through eigenrays for each target
        }      // loop through target depths
    }          // loop through target ranges

    loss.write_netcdf(analytic_name);
}

BOOST_AUTO_TEST_SUITE_END()
