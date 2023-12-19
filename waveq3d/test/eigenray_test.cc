/**
 * @example waveq3d/test/eigenray_test.cc
 */
#include <usml/ocean/ocean.h>
#include <usml/waveq3d/waveq3d.h>
#include <usml/eigenrays/eigenrays.h>

#include <boost/test/unit_test.hpp>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>

BOOST_AUTO_TEST_SUITE(waveq3d_eigenray_test)

using namespace boost::unit_test;
using namespace usml::waveq3d;
using namespace usml::eigenrays;

/**
 * @ingroup waveq3d_test
 * @{
 */

static const double time_step = 0.100;
static const double f0 = 2000;
static const double src_lat = 45.0;  // location = mid-Atlantic
static const double src_lng = -45.0;
static const double c0 = 1500.0;  // constant sound speed
static const double bot_depth = 1e5;

/**
 * Tests the basic features of the eigenray model for a simple target.
 *
 * - Multi-path arrivals from direct-path, surface, and bottom reflected paths.
 * - Estimation of travel times, source angles, target angles.
 * - Estimation of propagation loss and phase.
 *
 * This test models direct-path, surface-reflected, and bottom reflected paths
 * to a single point in a flat bottomed isovelocity ocean on a round earth.
 * This test limits the D/E ray fan from -60 to 60 so that extrapolation of
 * ray paths outside of the fan can also be tested. (Note that it is the
 * bottom bounce path that requires rays outside of this range.)
 *
 * - Scenario parameters
 *   - Profile: constant 1500 m/s sound speed, no absorption
 *   - Bottom: 3000 meters
 *   - Source: 45N, 45W, -1000 meters, 10 kHz
 *   - Target: 45.02N, 45W, -1000 meters
 *   - Time Step: 100 msec
 *   - Launch D/E: 5 degree linear spacing from -60 to 60 degrees
 *
 * - Analytic Results
 *   - Direct Path: 1.484018789 sec, -0.01 deg launch, 66.9506 dB
 *   - Surface Bounce: 1.995102731 sec, 41.93623171 deg launch, 69.5211 dB
 *   - Bottom Bounce: 3.051676949 sec, -60.91257162 deg launch, 73.2126 dB
 *
 * With a time step of 100 msec and an angular D/E spacing of 5.0 deg,
 * the interpolated results are expected to match the analytic values
 * within 2 msec and 0.01 deg.
 *
 * When the wave_queue::compute_offsets() fallback calculation of
 * offset(n) = -gradient(n) / hessian(n,n) is limited to 1/2 of the
 * beamwidth, the extrapolated bottom bounce path has large errors
 * in D/E angle. But if this clipping is not included, then the
 * eigenray_extra_test/eigenray_lloyds test will generate significant
 * errors in D/E. But the travel time on the extrapolated bottom bounce path
 * remains accurate to within 20 msec.  Developers should be aware of these
 * limitations when attempting to use targets outside of the rayfan.
 *
 * This test also looks at the accuracy of the propagation loss (PL) values
 * for this scenario.  This requires enough rays in the azimuthal (AZ)
 * direction to fill-in all AZ components of the Gaussian beams.  An azimuthal
 * ray fan from -4.0 to 4.0 degrees in 1 deg increments meets this requirement.
 * This configuration produces propagation loss values within 0.1 dB of the
 * 20*log10(R) theoretical value (where R is the distance
 * traveled). The exception to this PL accuracy level is the bottom bounce
 * case.  This is because "out of beam" effects are expected to yield
 * significantly weaker PL values than predicted by 20*log10(R).
 *
 * This test writes multi-path eigenrays in CSV format to eigenray_basic.csv
 * and in netCDF format to eigenray_basic.nc.  It also records the wavefronts
 * to eigenray_basic_wave.nc so that a ray trace can be plotted in Matlab.
 */
BOOST_AUTO_TEST_CASE(eigenray_basic) {
    cout << "=== eigenray_test: eigenray_basic ===" << endl;
    const char* csvname = USML_TEST_DIR "/waveq3d/test/eigenray_basic.csv";
    const char* ncname = USML_TEST_DIR "/waveq3d/test/eigenray_basic.nc";
    const char* ncname_wave =
        USML_TEST_DIR "/waveq3d/test/eigenray_basic_wave.nc";
    const double src_alt = -1000.0;
    const double trg_lat = 45.02;
    const double time_max = 3.5;

    // initialize propagation model

    wposition::compute_earth_radius(src_lat);
    boundary_model::csptr bottom(new boundary_flat(3000.0));
    boundary_model::csptr surface(new boundary_flat());
    attenuation_model::csptr attn(new attenuation_constant(0.0));
    profile_model::csptr profile(new profile_linear(c0, attn));
    ocean_model::csptr ocean(new ocean_model(surface, bottom, profile));

    seq_vector::csptr freq(new seq_log(10e3, 2.0, 3));
    wposition1 pos(src_lat, src_lng, src_alt);
    seq_vector::csptr de(new seq_linear(-60.0, 5.0, 60.0));
    seq_vector::csptr az(new seq_linear(-4.0, 1.0, 4.0));

    // build a single target

    wposition target(1, 1, trg_lat, src_lng, src_alt);

    eigenray_collection collection(freq, pos, target, 1);
    wave_queue wave(ocean, freq, pos, de, az, time_step, &target);
    wave.add_eigenray_listener(&collection);

    // propagate rays and record wavefronts to disk.

    cout << "propagate wavefronts for " << time_max << " seconds" << endl;
    cout << "writing wavefronts to " << ncname_wave << endl;

    wave.init_netcdf(ncname_wave);
    wave.save_netcdf();
    while (wave.time() < time_max) {
        wave.step();
        wave.save_netcdf();
    }
    wave.close_netcdf();

    // compute coherent propagation loss and write eigenrays to disk

    collection.sum_eigenrays();
    cout << "writing eigenray_collection to " << ncname << endl;
    collection.write_netcdf(ncname, "eigenray_basic test");

    // save results to spreadsheet and compare to analytic results

    cout << "writing tables to " << csvname << endl;
    std::ofstream os(csvname);
    os << "time,intensity,phase,s_de,s_az,t_de,t_az,srf,btm,cst" << endl;
    os << std::setprecision(18);
    cout << std::setprecision(18);

    const eigenray_list raylist = collection.eigenrays(0, 0);
    int test_case = 0;
    BOOST_CHECK_EQUAL(raylist.size(), 3);
    for (eigenray_model::csptr ray : raylist) {
        cout << "ray #" << test_case << " tl=" << ray->intensity(0)
             << " t=" << ray->travel_time << " de=" << -ray->target_de
             << " error:";
        os << ray->travel_time << "," << ray->intensity(0) << ","
           << ray->phase(0) << "," << ray->source_de << "," << ray->source_az
           << "," << ray->target_de << "," << ray->target_az << ","
           << ray->surface << "," << ray->bottom << "," << ray->caustic << endl;
        switch (test_case) {
            case 0:
                cout << " tl=" << (ray->intensity(0) - 66.9506)
                     << " t=" << (ray->travel_time - 1.484018789) << " de="
                     << max(abs(ray->source_de + 0.01),
                            abs(ray->target_de - 0.01))
                     << endl;
                BOOST_CHECK_SMALL(ray->intensity(0) - 66.9506, 0.1);
                BOOST_CHECK_SMALL(ray->travel_time - 1.484018789, 0.002);
                BOOST_CHECK_SMALL(ray->phase(0) - 0.0, 1e-6);
                BOOST_CHECK_SMALL(ray->source_de + 0.01, 0.01);
                BOOST_CHECK_SMALL(ray->target_de - 0.01, 0.01);
                break;
            case 1:
                cout << " tl=" << (ray->intensity(0) - 69.5211)
                     << " t=" << (ray->travel_time - 1.995102731) << " de="
                     << max(abs(ray->source_de - 41.93623171),
                            abs(ray->target_de + 41.93623171))
                     << endl;
                BOOST_CHECK_SMALL(ray->intensity(0) - 69.5211, 0.1);
                BOOST_CHECK_SMALL(ray->travel_time - 1.995102731, 0.002);
                BOOST_CHECK_SMALL(ray->phase(0) + M_PI, 1e-6);
                BOOST_CHECK_SMALL(ray->source_de - 41.93623171, 0.01);
                BOOST_CHECK_SMALL(ray->target_de + 41.93623171, 0.01);
                break;
            case 2:  // note that extrapolation is less accurate
                cout << " tl=" << (ray->intensity(0) - 73.2126)
                     << " t=" << (ray->travel_time - 3.051676949) << " de="
                     << max(abs(ray->source_de + 60.91257162),
                            abs(ray->target_de - 60.91257162))
                     << endl;
                BOOST_CHECK_SMALL(ray->intensity(0) - 73.2126, 4.0);
                BOOST_CHECK_SMALL(ray->travel_time - 3.051676949, 0.02);
                BOOST_CHECK_SMALL(ray->phase(0) - 0.0, 1e-6);
                break;
            default:
                break;
        }
        BOOST_CHECK_SMALL(ray->source_az - 0.0, 1e-6);
        BOOST_CHECK_SMALL(ray->target_az - 0.0, 1e-6);
        ++test_case;
    }
}

/**
 * Tests the model's ability to accurately estimate geometric terms for
 * the direct path and surface reflected eigenrays on a spherical earth.
 * The concave shape of the earth's surface causes the analytic solution
 * for the surface reflected path to have up to three roots at long ranges.
 * This test compares results for a single target at 1.2 deg to those
 * analystic solutions computed externally in a spreadsheet.
 *
 * - Scenario parameters
 *   - Profile: constant 1500 m/s sound speed, no absorption
 *   - Bottom: "infinitely" deep
 *   - Source: 200 meters deep, 2000 Hz
 *   - Target: 150 meters deep, 1.2 degrees north of source
 *   - Time Step: 100 msec
 *   - Launch D/E: 0.05 degree spacing from -1 to +1 degrees
 *
 * - Analytic Results
 *   - Direct: 89.05102557 sec, -0.578554378 deg launch, 0.621445622 deg target
 *   - Surf#1: 89.05369537 sec, 0.337347599 deg launch, 0.406539112 deg target
 *   - Surf#2: 89.05379297 sec, -0.053251329 deg launch, 0.233038477 deg target
 *   - Surf#3: 89.05320459 sec, -0.433973977 deg launch, -0.48969753 deg target
 *
 * When the model is run with these parameters, the travel times are accurate
 * to within 0.02 msec, the source D/E angles are accurate to within 0.05
 * degrees, and the target D/E angles are accurate to within 0.1 degrees.
 * But note that, if the spacing between launch angles is too small,
 * Surface 3 occurs between the same two rays as the Direct Path.  There
 * is a fundamental limitation of the model's eigenray searching logic that
 * only allows one ray path to be found between any two launch angles.
 * In this test, a wider launch angle spacing would cause the model to fail
 * to find the Surface 3 path.
 */
BOOST_AUTO_TEST_CASE(eigenray_concave) {
    cout << "=== eigenray_test: eigenray_concave ===" << endl;
    const char* ncname_wave =
        USML_TEST_DIR "/waveq3d/test/eigenray_concave_wave.nc";
    const char* ncname = USML_TEST_DIR "/waveq3d/test/eigenray_concave.nc";

    const double src_alt = -200.0;  // source depth = 200 meters
    const double time_max = 120.0;  // let rays plots go into region w/2 roots
    const double trg_lat = 46.2;    // 1.2 degrees north of source
    const double trg_lng = src_lng;
    const double trg_alt = -150.0;  // target depth = 150 meters

    // initialize propagation model

    wposition::compute_earth_radius(src_lat);
    attenuation_model::csptr attn(new attenuation_constant(0.0));
    profile_model::csptr profile(new profile_linear(c0, attn));
    boundary_model::csptr surface(new boundary_flat());
    boundary_model::csptr bottom(new boundary_flat(bot_depth));
    ocean_model::csptr ocean(new ocean_model(surface, bottom, profile));

    seq_vector::csptr freq(new seq_log(f0, 1.0, 1));
    wposition1 pos(src_lat, src_lng, src_alt);
    seq_vector::csptr de(new seq_linear(-1.0, 0.05, 1.0));
    seq_vector::csptr az(new seq_linear(-4.0, 1.0, 4.0));

    // build a single target

    wposition target(1, 1, trg_lat, trg_lng, trg_alt);

    eigenray_collection collection(freq, pos, target, 1);
    wave_queue wave(ocean, freq, pos, de, az, time_step, &target);
    wave.add_eigenray_listener(&collection);

    // propagate rays and record wavefronts to disk.

    cout << "propagate wavefronts for " << time_max << " seconds" << endl;
    cout << "writing wavefronts to " << ncname_wave << endl;

    wave.init_netcdf(ncname_wave);
    wave.save_netcdf();
    while (wave.time() < time_max) {
        wave.step();
        wave.save_netcdf();
    }
    wave.close_netcdf();
    cout << "writing eigenrays to " << ncname << endl;
    collection.sum_eigenrays();
    collection.write_netcdf(ncname);

    // compare analytic results

    const eigenray_list raylist = collection.eigenrays(0, 0);
    BOOST_CHECK_EQUAL(raylist.size(), 4);
    for (eigenray_model::csptr ray : raylist) {
        double theory_t, theory_sde, theory_tde;

        // direct path

        if (ray->surface == 0) {
            cout << "direct: ";
            theory_t = 89.05102557;
            theory_sde = -0.578554378;
            theory_tde = 0.621445622;

            // surface reflected paths

        } else {
            if (ray->source_de > 0.0) {  // surface path #1
                cout << "surf1:  ";
                theory_t = 89.05369537;
                theory_sde = 0.337347599;
                theory_tde = 0.406539112;
            } else {  // surface path #2
                if (ray->source_de > -0.1) {
                    cout << "surf2:  ";
                    theory_t = 89.05379297;
                    theory_sde = -0.053251329;
                    theory_tde = 0.233038477;
                } else {  // surface path #3
                    cout << "surf3:  ";
                    theory_t = 89.05320459;
                    theory_sde = -0.433973977;
                    theory_tde = -0.48969753;
                }
            }
        }

        cout << "t = " << ray->travel_time << " sde = " << ray->source_de
             << " tde = " << ray->target_de
             << " error: t = " << (ray->travel_time - theory_t)
             << " sde = " << (ray->source_de - theory_sde)
             << " tde = " << (ray->target_de - theory_tde) << endl;

        BOOST_CHECK_SMALL(ray->travel_time - theory_t, 2e-5);
        BOOST_CHECK_SMALL(ray->source_de - theory_sde, 0.05);
        BOOST_CHECK_SMALL(ray->target_de - theory_tde, 0.10);
    }
}

/**
 * Illustrate the variability of transmission loss as a function of azimuth.
 * These oscillations are a side effect of the way that Guassians from multiple
 * azimuths are added together to create the total transmission loss. No
 * automated tests, just creates data files for plotting.
 */
BOOST_AUTO_TEST_CASE(eigenray_tl_az) {
    cout << "=== eigenray_test: eigenray_tl_az ===" << endl;
    const char* ncname = USML_TEST_DIR "/waveq3d/test/eigenray_tl_az.nc";
    const char* ncname_wave =
        USML_TEST_DIR "/waveq3d/test/eigenray_tl_az_wave.nc";
    const double src_alt = -1000.0;
    const double target_range = 2222.4;

    const double time_max = 1.8;
    const int num_targets = 100;
    const double angle_sprd = 16.0;
    const double az_start = -8.0;
    const double az_inc = 1.0;
    const double tar_ang_sprd = 6.0;
    const double tar_bearing = 0.0;

    // initialize propagation model

    wposition::compute_earth_radius(0.0);
    attenuation_model::csptr attn(new attenuation_constant(0.0));
    profile_model::csptr profile(new profile_linear(c0, attn));
    boundary_model::csptr bottom(new boundary_flat(3000.0));
    boundary_model::csptr surface(new boundary_flat());
    ocean_model::csptr ocean(new ocean_model(surface, bottom, profile));

    seq_vector::csptr freq(new seq_log(1000.0, 1.0, 1));
    wposition1 pos(0.0, 0.0, src_alt);
    seq_vector::csptr de(new seq_linear(-60.0, 5.0, 60.0));
    seq_vector::csptr az(
        new seq_linear(az_start, az_inc, az_start + angle_sprd));

    // build a series of targets at target_range distance away

    wposition target(num_targets, 1, 0.0, 0.0, src_alt);
    double angle = (tar_ang_sprd * M_PI / 180.0) / num_targets;
    double bearing_inc = (tar_bearing * M_PI / 180.0);
    for (size_t n = 0; n < num_targets; ++n) {
        wposition1 trg(pos, target_range, bearing_inc);
        target.latitude(n, 0, trg.latitude());
        target.longitude(n, 0, trg.longitude());
        target.altitude(n, 0, trg.altitude());
        bearing_inc = bearing_inc + angle;
    }

    eigenray_collection collection(freq, pos, target, 1);
    wave_queue wave(ocean, freq, pos, de, az, time_step, &target);
    wave.add_eigenray_listener(&collection);

    // propagate rays and record wavefronts to disk.

    cout << "propagate wavefronts for " << time_max << " seconds" << endl;
    cout << "writing wavefronts to " << ncname_wave << endl;

    wave.init_netcdf(ncname_wave);
    wave.save_netcdf();
    while (wave.time() < time_max) {
        wave.step();
        wave.save_netcdf();
    }
    wave.close_netcdf();

    // compute coherent propagation loss and write eigenrays to disk

    collection.sum_eigenrays();
    cout << "writing eigenray_collection to " << ncname << endl;
    collection.write_netcdf(ncname);
}

/**
 * When acoustic targets are along the boundaries of the wavefront,
 * the algorithm for producing eigenrays treats the azimuthal degrees
 * of zero and 360 as separate angles. This essentially produces two
 * eigenrays of exactly half of the true eigenray. This test leverages
 * the work already provided in eigenray_basic and extends to include
 * both AZ and DE branch point targets.
 *
 * A new algorithm was introduced to detect the branch points in the AZ
 * dimension and only produce one eigenray at the correct strength. Similarly
 * for acoustic targets that were directly above/below the source, logic was
 * added to produce a single eigenray that would be the sum of all eigenrays
 * that would have been previously produced.
 *
 * A BOOST_CHECK_EQUAL is used to verify that only three eigenrays are produced
 * for each target. The user is then provided with a csv file that
 * can then be used to verify the correct transmission loss has been produced
 * for each eigenray to each target.
 */
BOOST_AUTO_TEST_CASE(eigenray_branch_pt) {
    cout << "=== eigenray_test: eigenray_branch_pt ===" << endl;
    const char* csvname = USML_TEST_DIR "/waveq3d/test/eigenray_branch_pt.csv";
    const double src_alt = -1000.0;
    const double target_range = 2226.0;
    const double time_max = 3.5;
    const int num_targets = 12;

    // initialize propagation model

    wposition::compute_earth_radius(0.0);
    attenuation_model::csptr attn(new attenuation_constant(0.0));
    profile_model::csptr profile(new profile_linear(c0, attn));
    boundary_model::csptr surface(new boundary_flat());
    boundary_model::csptr bottom(new boundary_flat(3000.0));
    ocean_model::csptr ocean(new ocean_model(surface, bottom, profile));

    seq_vector::csptr freq(new seq_log(1000.0, 1.0, 1));
    wposition1 pos(0.0, 0.0, src_alt);
    seq_vector::csptr de(new seq_linear(-90.0, 1.0, 90.0));
    seq_vector::csptr az(new seq_linear(0.0, 15.0, 360.0));

    // build a pair of targets directly above and below the source

    wposition target(num_targets + 2, 1, 0.0, 0.0, src_alt);
    target.altitude(0, 0, src_alt - 500);
    target.altitude(1, 0, src_alt + 500);

    // build a series of targets at 100 km

    double angle = TWO_PI / num_targets;
    double bearing_inc = 0;
    for (size_t n = 2; n < num_targets + 2; ++n) {
        wposition1 trg(pos, target_range, bearing_inc);
        target.latitude(n, 0, trg.latitude());
        target.longitude(n, 0, trg.longitude());
        target.altitude(n, 0, trg.altitude());
        bearing_inc = bearing_inc + angle;
    }

    eigenray_collection collection(freq, pos, target, 1);
    wave_queue wave(ocean, freq, pos, de, az, time_step, &target);
    wave.add_eigenray_listener(&collection);

    // propagate rays and record wavefronts to disk.

    cout << "propagate wavefronts for " << time_max << " seconds" << endl;
    while (wave.time() < time_max) {
        wave.step();
    }
    collection.sum_eigenrays();

    // save results to spreadsheet and compare to analytic results

    cout << "writing tables to " << csvname << endl;
    std::ofstream os(csvname);
    os << "target,time,intensity,phase,s_de,s_az,t_de,t_az,srf,btm,cst" << endl;
    os << std::setprecision(18);
    cout << std::setprecision(18);

    for (int trg_num = 0; trg_num < num_targets; ++trg_num) {
        os << "#" << trg_num;
        const eigenray_list raylist = collection.eigenrays(trg_num, 0);
        int test_case = 0;
        BOOST_CHECK_EQUAL(raylist.size(), 3);
        for (eigenray_model::csptr ray : raylist) {
            os << "," << ray->travel_time << "," << ray->intensity(0) << ","
               << ray->phase(0) << "," << ray->source_de << ","
               << ray->source_az << "," << ray->target_de << ","
               << ray->target_az << "," << ray->surface << "," << ray->bottom
               << "," << ray->caustic << endl;

            // check that results predicted for eigenray_basic
            // don't check the 2 targets above and below source.

            if (trg_num > 1) {
                switch (test_case) {
                    case 0:
                        BOOST_CHECK_SMALL(ray->intensity(0) - 66.9506, 0.1);
                        BOOST_CHECK_SMALL(ray->travel_time - 1.484018789,
                                          0.002);
                        BOOST_CHECK_SMALL(ray->phase(0) - 0.0, 1e-6);
                        BOOST_CHECK_SMALL(ray->source_de + 0.01, 0.01);
                        BOOST_CHECK_SMALL(ray->target_de - 0.01, 0.01);
                        break;
                    case 1:
                        BOOST_CHECK_SMALL(ray->intensity(0) - 69.5211, 0.1);
                        BOOST_CHECK_SMALL(ray->travel_time - 1.995102731,
                                          0.002);
                        BOOST_CHECK_SMALL(ray->phase(0) + M_PI, 1e-6);
                        BOOST_CHECK_SMALL(ray->source_de - 41.93623171, 0.01);
                        BOOST_CHECK_SMALL(ray->target_de + 41.93623171, 0.01);
                        break;
                    case 2:  // note that extrapolation is less accurate
                        BOOST_CHECK_SMALL(ray->travel_time - 3.051676949, 0.02);
                        BOOST_CHECK_SMALL(ray->phase(0) - 0.0, 1e-6);
                        BOOST_CHECK_SMALL(ray->source_de + 60.91257162, 1.0);
                        BOOST_CHECK_SMALL(ray->target_de - 60.91257162, 1.0);
                        break;
                    default:
                        break;
                }
            }
            ++test_case;
        }
    }
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
