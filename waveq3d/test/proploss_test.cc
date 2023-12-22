/**
 * @example waveq3d/test/proploss_test.cc
 */

#include <usml/eigenrays/eigenrays.h>
#include <usml/netcdf/netcdf_files.h>
#include <usml/ocean/ocean.h>
#include <usml/waveq3d/waveq3d.h>

#include <boost/test/unit_test.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>

BOOST_AUTO_TEST_SUITE(proploss_test)

using namespace boost::unit_test;
using namespace usml::waveq3d;

// set default spreading model
const auto spreading_type = wave_queue::HYBRID_GAUSSIAN;
// const auto spreading_type = wave_queue::CLASSIC_RAY;

/**
 * @ingroup waveq3d_test
 * @{
 */

static const double time_step = 0.100;
static const double bot_depth = 1e5;

/**
 * Tests transmission loss intensity accuracy for the "classic" spreading loss
 * model, in an environment with no interface interactions. The modeled
 * transmission loss should be 20*log10(R), where R is the range to target,
 * regardless of frequency.
 *
 *  - Source:       15,000 meters deep
 *  - Target:       15,000 meters deep, latitude change is 0.01-18.01 deg
 *  - Frequency:    10, 100, 1000, & 10000 Hz
 *  - Sound Speed:  1500 m/s
 *  - Time Step:    100 msec
 *	- Source D/E:	-10 deg to 10 deg in 1 deg increments
 *	- Source AZ:	-10 deg to 10 deg in 2 deg increments
 *
 * BOOST errors are automatically generated if the modeled eigenrays deviate
 * from the theory by more than 0.2 dB. The eigenrays for all targets are
 * written to the proploss_classic.csv spreadsheet file.
 */
BOOST_AUTO_TEST_CASE(proploss_classic) {
    cout << "=== proploss_test: proploss_classic ===" << endl;
    const char* csvname = USML_TEST_DIR "/waveq3d/test/proploss_classic.csv";
    const double c0 = 1500.0;
    const double src_lat = 45.0;
    const double src_lng = -45.0;
    const double src_alt = -15000.0;
    const double time_max = 8.0;

    // initialize propagation model

    wposition::compute_earth_radius(src_lat);
    attenuation_model::csptr attn(new attenuation_constant(0.0));
    profile_model::csptr profile(new profile_linear(c0, attn));
    boundary_model::csptr bottom(new boundary_flat(bot_depth));
    boundary_model::csptr surface(new boundary_flat());
    ocean_model::csptr ocean(new ocean_model(surface, bottom, profile));

    seq_vector::csptr freq_list(new seq_log(10.0, 10.0, 4));
    cout << "frequencies: " << (*freq_list) << endl;
    wposition1 pos(src_lat, src_lng, src_alt);
    seq_vector::csptr de(new seq_linear(-10.7, 1.0, 10.0));
    seq_vector::csptr az(new seq_linear(-10.5, 2.0, 10.0));

    // build a series of targets at different ranges

    wposition target(10, 1, src_lat, src_lng, src_alt);
    for (size_t n = 0; n < target.size1(); ++n) {
        target.latitude(n, 0, src_lat + 0.01 * ((double)n + 2.0));
    }

    eigenray_collection model(freq_list, pos, target, 1);
    wave_queue wave(ocean, freq_list, pos, de, az, time_step, &target,
                    wave_queue::CLASSIC_RAY);
    wave.add_eigenray_listener(&model);

    // propagate rays & record to log file

    cout << "propagate wavefronts" << endl;
    while (wave.time() < time_max) {
        wave.step();
    }
    model.sum_eigenrays();

    // save results to spreadsheet and compare to analytic results

    std::ofstream os(csvname);
    os << "target,time,intensity,phase,src de,src az,trg de,trg az,surf,bot"
       << endl;
    os << std::setprecision(18);

    cout << "writing spreadsheets to " << csvname << endl;

    for (size_t n = 0; n < target.size1(); ++n) {
        const eigenray_list& raylist = model.eigenrays(n, 0);
        for (const eigenray_model::csptr& ray : raylist) {
            os << n << "," << ray->travel_time << "," << ray->intensity(0)
               << "," << ray->phase(0) << "," << ray->source_de << ","
               << ray->source_az << "," << ray->target_de << ","
               << ray->target_az << "," << ray->surface << "," << ray->bottom
               << endl;
            double range_list = c0 * ray->travel_time;
            double pl = 20.0 * log10(range_list - 2.0);
            cout << "range=" << range_list << " theory=" << pl
                 << " model=" << ray->intensity << endl;
            for (size_t f = 0; f < freq_list->size(); ++f) {
                BOOST_CHECK_SMALL(ray->intensity(f) - pl, 0.2);
            }
        }
    }
}

/**
 * Tests transmission loss intensity accuracy for the "hybrid gaussian"
 * spreading loss model, as a function of frequency, in an environment with
 * surface and bottom reflections. The modeled transmission loss should be
 * 20*log10(R), where R is the range to target, regardless of frequency.
 *
 * Gaussian beam widths increase as frequency decreases. At lower frequencies,
 * this overestimates transmission loss if there aren't enough rays to fully
 * "support" the Gaussian beam summation at the target location. To illustrate
 * this point, we intensionally limit the azimuthal coverage of the ray fan to
 * +/- 5 degrees, and compute transmission losses from 100 to 1000 Hz. This
 * configuration causes an accuracy error that decreases as frequency increases.
 *
 * - Scenario parameters
 *   - Profile: constant 1500 m/s sound speed, no absorption
 *   - Bottom: 3000 meters
 *   - Source: 45N, 45W, -1000 meters
 *   - Target: 45.02N, 45W, -1000 meters
 *   - Time Step: 100 msec
 *   - Launch D/E: 1 degree linear spacing from -90 to 90 degrees
 *	 - Launch AZ 1 degree linear spacing from -6 to 6 degrees
 *
 * - Analytic Results
 *   - Direct Path: 1.484018789 sec, -0.01 deg launch, 66.95 dB
 *   - Surface Bounce: 1.995102731 sec, 41.93623171 deg launch, 69.52 dB
 *   - Bottom Bounce: 3.051676949 sec, -60.91257162 deg launch, 73.21 dB
 *
 * BOOST errors are automatically generated if the modeled eigenrays deviate
 * from the theory by more than 2.0 dB. The eigenrays for all acoustic paths are
 * written to the proploss_gaussian.csv spreadsheet file.
 */
BOOST_AUTO_TEST_CASE(proploss_gaussian) {
    cout << "=== proploss_test: proploss_gaussian ===" << endl;
    const char* ncproploss = USML_TEST_DIR "/waveq3d/test/proploss_gaussian.nc";
    const char* csvname = USML_TEST_DIR "/waveq3d/test/proploss_gaussian.csv";
    const double c0 = 1500.0;
    const double src_lat = 45.0;
    const double src_lng = -45.0;
    const double src_alt = -1000.0;
    const double trg_lat = 45.02;
    const double time_max = 3.5;

    // initialize propagation model

    attenuation_model::csptr attn(new attenuation_constant(0.0));
    profile_model::csptr profile(new profile_linear(c0, attn));
    boundary_model::csptr surface(new boundary_flat());
    boundary_model::csptr bottom(new boundary_flat(3000.0));
    ocean_model::csptr ocean(new ocean_model(surface, bottom, profile));

    seq_vector::csptr freq_list(new seq_linear(100.0, 100.0, 1000.0));
    cout << "frequencies: " << (*freq_list) << endl;
    wposition1 pos(src_lat, src_lng, src_alt);
    seq_vector::csptr de(new seq_linear(-90.0, 1.0, 90.0));
    seq_vector::csptr az(new seq_linear(-5.0, 1.0, 5.0));

    // build a single target

    wposition target(1, 1, trg_lat, src_lng, src_alt);

    eigenray_collection model(freq_list, pos, target);
    wave_queue wave(ocean, freq_list, pos, de, az, time_step, &target);
    wave.add_eigenray_listener(&model);

    // propagate rays & record to log file

    cout << "propagate wavefront for " << time_max << endl;
    while (wave.time() < time_max) {
        wave.step();
    }
    model.sum_eigenrays();
    cout << "writing eigenray_collection to " << ncproploss << endl;
    model.write_netcdf(ncproploss, "proploss_freq test");

    // save results to spreadsheet and compare to analytic results

    cout << "writing data to " << csvname << endl;
    std::ofstream os(csvname);
    os << "frequency,theory_direct,model_direct,diff_direct,"
          "theory_surface,model_surface,diff_surface,"
          "theory_bottom,theory_bottom,diff_bottom"
       << endl;
    os << std::setprecision(18);

    eigenray_model::csptr direct_ray;
    eigenray_model::csptr surface_ray;
    eigenray_model::csptr bottom_ray;
    double direct_pl{0};
    double surface_pl{0};
    double bottom_pl{0};
    const eigenray_list raylist = model.eigenrays(0, 0);
    for (const eigenray_model::csptr& ray : raylist) {
        double range_list = c0 * ray->travel_time;
        double pl = 20.0 * log10(range_list - 2.0);
        if (ray->bottom == 1 && ray->surface == 0) {
            bottom_ray = ray;
            bottom_pl = pl;
        } else if (ray->bottom == 0 && ray->surface == 1) {
            surface_ray = ray;
            surface_pl = pl;
        } else {
            direct_ray = ray;
            direct_pl = pl;
        }
    }
    for (size_t j = 0; j < freq_list->size(); ++j) {
        auto diff_direct = direct_ray->intensity(j) - direct_pl;
        auto diff_surface = surface_ray->intensity(j) - surface_pl;
        auto diff_bottom = bottom_ray->intensity(j) - bottom_pl;
        os << (*freq_list)(j) << "," << direct_pl << ","
           << direct_ray->intensity(j) << "," << diff_direct << ","
           << surface_pl << "," << surface_ray->intensity(j) << ","
           << diff_surface << "," << bottom_pl << ","
           << bottom_ray->intensity(j) << "," << diff_bottom << endl;
        if (j > 0) {
            BOOST_CHECK_SMALL(direct_ray->intensity(j) - direct_pl, 2.0);
            BOOST_CHECK_SMALL(surface_ray->intensity(j) - surface_pl, 2.0);
            BOOST_CHECK_SMALL(bottom_ray->intensity(j) - bottom_pl, 2.0);
        }
    }
}

/**
 * This test demonstrates ability to adjust source position if it is within
 * 0.1 meters of being above the ocean surface or below the ocean bottom.
 * The boundary reflection logic does not perform correctly if the
 * wavefront starts on the wrong side of either boundary.
 */
BOOST_AUTO_TEST_CASE(proploss_limits) {
    cout << "=== proploss_test: proploss_limits ===" << endl;

    const double src_lat = 45.0;
    const double src_lng = -45.0;
    const double src_alt = 0.0;
    const double depth = -1000;

    // initialize propagation model

    profile_model::csptr profile(new profile_linear());
    boundary_model::csptr surface(new boundary_flat());
    boundary_model::csptr bottom(new boundary_flat(1000.0));
    ocean_model::csptr ocean(new ocean_model(surface, bottom, profile));

    seq_vector::csptr freq(new seq_linear(1000.0, 1.0, 1));
    wposition1 pos(src_lat, src_lng, src_alt);
    seq_vector::csptr de(new seq_rayfan());
    seq_vector::csptr az(new seq_linear(-20.0, 5.0, 20.0));

    // try building a source above ocean surface

    pos.altitude(10.0);
    wave_queue wave1(ocean, freq, pos, de, az, time_step);
    BOOST_CHECK_CLOSE(wave1.source_pos().altitude(), -0.1, 1e-6);

    // try building a source above ocean surface

    pos.altitude(depth - 10.0);
    wave_queue wave2(ocean, freq, pos, de, az, time_step);
    BOOST_CHECK_CLOSE(wave2.source_pos().altitude(), depth + 0.1, 1e-6);
}

/**
 * Compares modeled propagation loss as a function of range and frequency to the
 * Lloyd's mirror analytic expression for surface reflection in an isovelocity
 * ocean. In this test, we expect to see the errors between the model and theory
 * grow at short ranges. The discreet time step creates a discontinuity between
 * the direct and reflected paths near the ocean surface, and the size of this
 * discontinuity increases at higher D/E angles. The model must extrapolate the
 * transmission loss across this fold in the wavefront, and we beleive this
 * extrapolation causes the observed errors. Decreasing the size of the time
 * step decreases these errors.
 *
 * - Scenario parameters
 *   - Source:       25 meters deep
 *   - Target:       200 meters deep, range is 200-10,000 m
 *   - Frequency:    2000 Hz
 *   - Sound Speed:  1500 m/s
 *   - Time Step:    50 msec
 *	 - Source D/E:	-90 deg to 90 deg, 181 rays with tangental spacing
 *   - Source AZ:	-20 deg to 20 deg in 5 deg increments
 *
 * The Lloyd's analytic analytic result is:
 * \f[
 *      p(r,z) = \frac{ e^{ikR_1} }{ R_1 } - \frac{ e^{ikR_2} }{ R_2 }
 * \f]\f[
 *      R_1 = \sqrt{ r^2 + ( z - z_s )^2 }
 * \f]\f[
 *      R_2 = \sqrt{ r^2 + ( z + z_s )^2 }
 * \f]\f[
 *      PL = -10 log_{10}( |p(r,z)|^2 )
 * \f]\f[
 *      \theta = arg( p(r,z) )
 * \f]
 * where:
 *      - \f$ r         \f$ = target range
 *      - \f$ z         \f$ = target depth
 *      - \f$ z_s       \f$ = source depth
 *      - \f$ R_1       \f$ = slant range to source
 *      - \f$ R_2       \f$ = slant range to image of source above water
 *      - \f$ k         \f$ = acoustic wave number = \f$ 2 \pi f / c \f$
 *      - \f$ p(r,z)    \f$ = complex pressure
 *      - \f$ PL        \f$ = propagation loss in dB.
 *      - \f$ \theta    \f$ = phase in radians
 *
 * This test computes three statistics to measure the difference between
 * the model and the analytic solution.
 *
 * - "bias" is the mean difference and it measures offsets in level.
 * - "dev" is an estimate of the sqrt of the variance and it is a measure of
 *   the difference in flucuations of between the models.
 * - "detcoef" is the coefficient of determination and its measure of the
 *   fraction of the model that predicts the analytic solution.
 *
 * @xref F.B. Jensen, W.A. Kuperman, M.B. Porter, H. Schmidt,
 * "Computational Ocean Acoustics", pp. 16-19.
 */
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
BOOST_AUTO_TEST_CASE(proploss_lloyds_range) {
    cout << "=== proploss_test: proploss_lloyds_range ===" << endl;
    const char* csvname =
        USML_TEST_DIR "/waveq3d/test/proploss_lloyds_range.csv";
    const char* ncname = USML_TEST_DIR "/waveq3d/test/proploss_lloyds_range.nc";
    const char* ncname_wave =
        USML_TEST_DIR "/waveq3d/test/proploss_lloyds_range_wave.nc";

    const double c0 = 1500.0;
    const double src_lat = 45.0;
    const double src_lng = -45.0;
    const double src_alt = -25.0;
    const double trg_alt = -200.0;
    const double time_max = 8.0;

    // initialize propagation model

    attenuation_model::csptr attn(new attenuation_constant(0.0));
    profile_model* ssp = new profile_linear(c0, attn);
    ssp->flat_earth(true);
    profile_model::csptr profile(ssp);
    boundary_model::csptr surface(new boundary_flat());
    boundary_model::csptr bottom(new boundary_flat(1e5));
    ocean_model::csptr ocean(new ocean_model(surface, bottom, profile));

    seq_vector::csptr freq_list(new seq_linear(500.0, 500.0, 4));
    cout << "frequencies: " << (*freq_list) << endl;
    wposition1 pos(src_lat, src_lng, src_alt);
    seq_vector::csptr de(new seq_rayfan());
    seq_vector::csptr az(new seq_linear(-20.0, 5.0, 20.0));

    // build a series of targets at different ranges

    seq_linear range_list(200.0, 5.0, 10e3);  // range in meters
    wposition target(range_list.size(), 1, src_lat, src_lng, trg_alt);
    for (size_t n = 0; n < target.size1(); ++n) {
        double degrees =
            src_lat + range_list(n) / (1852.0 * 60.0);  // range in latitude
        target.latitude(n, 0, degrees);
    }

    eigenray_collection model(freq_list, pos, target);
    wave_queue wave(ocean, freq_list, pos, de, az, time_step, &target,
                    spreading_type);
    wave.add_eigenray_listener(&model);

    // propagate rays & record to log file

    cout << "propagate wavefronts" << endl;
    cout << "writing wavefronts to " << ncname_wave << endl;
    wave.init_netcdf(ncname_wave);
    wave.save_netcdf();
    while (wave.time() < time_max) {
        wave.step();
        wave.save_netcdf();
    }
    wave.close_netcdf();
    cout << "writing eigenrays to " << ncname << endl;
    model.sum_eigenrays();
    model.write_netcdf(ncname);

    // save results to spreadsheet and compare to analytic results

    cout << "writing spreadsheets to " << csvname << endl;
    std::ofstream os(csvname);
    os << "freq,range,model,theory,m1srf,m1btm,m1amp,m1time,t1amp,"
          "t1time,m2srf,m2btm,m2amp,m2time,t2amp,t2time"
       << endl;
    os << std::setprecision(18);

    for (size_t f = 0; f < freq_list->size(); ++f) {
        const auto freq = (*freq_list)(f);
        const auto wavenum = TWO_PI * freq / c0;
        double mean_model = 0.0;
        double mean_analytic = 0.0;

        vector<double> tl_model(range_list.size());
        vector<double> tl_analytic(range_list.size());

        const double z1 = trg_alt - src_alt;
        const double z2 = trg_alt + src_alt;

        for (size_t n = 0; n < range_list.size(); ++n) {
            const auto range = range_list(n);
            tl_model[n] = -model.total(n, 0).intensity(f);

            // compute analytic solution

            const double R1 = hypot(range, z1);
            const double R2 = hypot(range, z2);
            complex<double> p1(0.0, wavenum * R1);
            complex<double> p2(0.0, wavenum * R2);
            p1 = exp(p1) / R1;
            p2 = -exp(p2) / R2;
            tl_analytic[n] = 10.0 * log10(norm(p1 + p2));

            // print to log file

            os << freq << "," << range << "," << tl_model[n] << ","
               << tl_analytic[n];
            const eigenray_list& raylist = model.eigenrays(n, 0);
            auto path_num = 0;
            for (const eigenray_model::csptr& ray : raylist) {
                // write path data to spreadsheet

                os << "," << ray->surface << "," << ray->bottom << ","
                   << -ray->intensity(0) << "," << ray->travel_time;
                if (ray->surface == 0) {
                    os << "," << 10.0 * log10(norm(p1)) << "," << R1 / c0;
                } else {
                    os << "," << 10.0 * log10(norm(p2)) << "," << R2 / c0;
                }

                // check path type data

                BOOST_REQUIRE_LT(path_num, 2);
                BOOST_CHECK_EQUAL(ray->surface, path_num);
                BOOST_CHECK_EQUAL(ray->bottom, 0);
                BOOST_CHECK_EQUAL(ray->caustic, 0);
                BOOST_CHECK_EQUAL(ray->upper, 0);
                BOOST_CHECK_EQUAL(ray->lower, 0);
                ++path_num;
            }
            os << endl;
            mean_model += tl_model[n];
            mean_analytic += tl_analytic[n];
        }

        mean_model /= (double)range_list.size();
        mean_analytic /= (double)range_list.size();

        // compute statistics of difference between curves

        double bias = 0.0;
        double dev = 0.0;
        double Sxx = 0.0;
        double Syy = 0.0;
        double Sxy = 0.0;
        for (size_t n = 0; n < range_list.size(); ++n) {
            const double diff = (tl_model[n] - tl_analytic[n]);
            bias += diff;
            dev += (diff * diff);
            const double diff_analytic = (tl_analytic[n] - mean_analytic);
            Sxx += (diff_analytic * diff_analytic);
            const double diff_model = (tl_model[n] - mean_model);
            Syy += (diff_model * diff_model);
            Sxy += (diff_analytic * diff_model);
        }
        bias /= (double)range_list.size();
        dev = sqrt(dev / (double)range_list.size() - bias * bias);
        double detcoef = Sxy * Sxy / (Sxx * Syy) * 100.0;

        cout << std::setprecision(4);
        cout << "freq = " << (*freq_list)(f) << " Hz"
             << " bias = " << bias << " dB"
             << " dev = " << dev << " dB"
             << " detcoef = " << detcoef << "%" << endl;

        if (freq > 250.0) {
            BOOST_CHECK(abs(bias) <= 0.5);
        }
        BOOST_CHECK(dev <= 4.0);
        BOOST_CHECK(detcoef >= 80.0);
    }
}

/**
 * Compares modeled propagation loss as a function of depth to the Lloyd's
 * mirror analytic expression for surface reflection in an isovelocity ocean.
 * This forces the model to deal with target points near the surface where the
 * up-going and down-going wavefronts must be extrapolated from two ray
 * families that have different numbers of surface bounces.
 *
 * - Scenario parameters
 *   - Source:       25 meters deep
 *   - Target:       Range 2000 meters, depth is 1-50 meters
 *   - Frequency:    2000 Hz
 *   - Sound Speed:  1500 m/s
 *   - Time Step:    50 msec
 *	 - Source D/E:	-90 deg to 90 deg, 181 rays with tangent spacing
 *	 - Source AZ:	-2 deg to 2 deg in 1 deg increments
 *
 * The analytic result is the same as that for proploss_lloyds_range().
 *
 * Targets near the surface must be extrapolated from the wavefronts below
 * them.  Because the Gaussian profile rolls off at edge of a ray family,
 * it is important to test the impact of that phenomena on propagation loss at
 * the interface.
 *
 * This test computes three statistics to measure the difference between
 * the model and the analytic solution.
 *
 * - "bias" is the mean difference and it measures offsets in level.
 * - "dev" is an estimate of the sqrt of the variance and it is a measure of
 *   the difference in fluctuations of between the models.
 * - "detcoef" is the coefficient of determination and it measure of the
 *   fraction of the model that is predicts the analytic solution.
 *
 * An automatic error is thrown if abs(bias) > 0.5 dB, dev > 4 db, or
 * detcoef < 80%.
 *
 * @xref F.B. Jensen, W.A. Kuperman, M.B. Porter, H. Schmidt,
 * "Computational Ocean Acoustics", pp. 16-19.
 */
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
BOOST_AUTO_TEST_CASE(proploss_lloyds_depth) {
    cout << "=== proploss_test: proploss_lloyds_depth ===" << endl;
    const char* csvname =
        USML_TEST_DIR "/waveq3d/test/proploss_lloyds_depth.csv";
    const char* ncname = USML_TEST_DIR "/waveq3d/test/proploss_lloyds_depth.nc";
    const char* ncname_wave =
        USML_TEST_DIR "/waveq3d/test/proploss_lloyds_depth_wave.nc";

    const double c0 = 1500.0;
    const double src_lat = 45.0;
    const double src_lng = -45.0;
    const double src_alt = -25.0;
    const double range = 10e3;
    const double time_max = 8.0;

    // initialize propagation model

    attenuation_model::csptr attn(new attenuation_constant(0.0));
    profile_model* ssp = new profile_linear(c0, attn);
    ssp->flat_earth(true);
    profile_model::csptr profile(ssp);
    boundary_model::csptr surface(new boundary_flat());
    boundary_model::csptr bottom(new boundary_flat(1e5));
    ocean_model::csptr ocean(new ocean_model(surface, bottom, profile));

    seq_vector::csptr freq_list(new seq_linear(500.0, 500.0, 4));
    cout << "frequencies: " << (*freq_list) << endl;
    wposition1 pos(src_lat, src_lng, src_alt);
    seq_vector::csptr de(new seq_rayfan());
    seq_vector::csptr az(new seq_linear(-20.0, 5.0, 20.0));

    // build a series of targets at different depths

    double degrees =
        src_lat + to_degrees(range / (wposition::earth_radius + src_alt));
    seq_linear depth_list(-0.1, -0.5, -50.1);  // depth in meters
    wposition target(depth_list.size(), 1, degrees, src_lng, 0.0);
    for (size_t n = 0; n < target.size1(); ++n) {
        target.altitude(n, 0, depth_list(n));
    }

    eigenray_collection model(freq_list, pos, target);
    wave_queue wave(ocean, freq_list, pos, de, az, time_step, &target,
                    spreading_type);
    wave.add_eigenray_listener(&model);

    // propagate rays & record to log file

    cout << "propagate wavefronts" << endl;
    cout << "writing wavefronts to " << ncname_wave << endl;
    wave.init_netcdf(ncname_wave);
    wave.save_netcdf();
    while (wave.time() < time_max) {
        wave.step();
        wave.save_netcdf();
    }
    wave.close_netcdf();
    cout << "writing eigenrays to " << ncname << endl;
    model.sum_eigenrays();
    model.write_netcdf(ncname);

    // save results to spreadsheet and compare to analytic results

    cout << "writing spreadsheets to " << csvname << endl;
    std::ofstream os(csvname);
    os << "freq,depth,model,theory,m1srf,m1btm,m1amp,m1time,t1amp,"
          "t1time,m2srf,m2btm,m2amp,m2time,t2amp,t2time"
       << endl;
    os << std::setprecision(18);

    for (size_t f = 0; f < freq_list->size(); ++f) {
        const auto freq = (*freq_list)(f);
        const auto wavenum = TWO_PI * freq / c0;
        double mean_model = 0.0;
        double mean_analytic = 0.0;

        vector<double> tl_model(depth_list.size());
        vector<double> tl_analytic(depth_list.size());

        for (size_t n = 0; n < depth_list.size(); ++n) {
            auto depth = depth_list(n);
            const double z1 = depth - src_alt;
            const double z2 = depth + src_alt;
            tl_model[n] = -model.total(n, 0).intensity(f);

            // compute analytic solution

            const double R1 = hypot(range, z1);
            const double R2 = hypot(range, z2);
            complex<double> p1(0.0, wavenum * R1);
            complex<double> p2(0.0, wavenum * R2);
            p1 = exp(p1) / R1;
            p2 = -exp(p2) / R2;
            tl_analytic[n] = 10.0 * log10(norm(p1 + p2));

            // print to log file

            os << freq << "," << depth << "," << tl_model[n] << ","
               << tl_analytic[n];
            const eigenray_list& raylist = model.eigenrays(n, 0);
            auto path_num = 0;
            for (const eigenray_model::csptr& ray : raylist) {
                // write path data to spreadsheet

                os << "," << ray->surface << "," << ray->bottom << ","
                   << -ray->intensity(0) << "," << ray->travel_time;
                if (ray->surface == 0) {
                    os << "," << 10.0 * log10(norm(p1)) << "," << R1 / c0;
                } else {
                    os << "," << 10.0 * log10(norm(p2)) << "," << R2 / c0;
                }

                // check path type data

                BOOST_REQUIRE_LT(path_num, 2);
                BOOST_CHECK_EQUAL(ray->surface, path_num);
                BOOST_CHECK_EQUAL(ray->bottom, 0);
                BOOST_CHECK_EQUAL(ray->caustic, 0);
                BOOST_CHECK_EQUAL(ray->upper, 0);
                BOOST_CHECK_EQUAL(ray->lower, 0);
                ++path_num;
            }
            os << endl;
            mean_model += tl_model[n];
            mean_analytic += tl_analytic[n];
        }

        mean_model /= (double)depth_list.size();
        mean_analytic /= (double)depth_list.size();

        // compute statistics of difference between curves

        double bias = 0.0;
        double dev = 0.0;
        double Sxx = 0.0;
        double Syy = 0.0;
        double Sxy = 0.0;
        for (size_t n = 0; n < depth_list.size(); ++n) {
            const double diff = (tl_model[n] - tl_analytic[n]);
            bias += diff;
            dev += (diff * diff);
            const double diff_analytic = (tl_analytic[n] - mean_analytic);
            Sxx += (diff_analytic * diff_analytic);
            const double diff_model = (tl_model[n] - mean_model);
            Syy += (diff_model * diff_model);
            Sxy += (diff_analytic * diff_model);
        }
        bias /= (double)depth_list.size();
        dev = sqrt(dev / (double)depth_list.size() - bias * bias);
        double detcoef = Sxy * Sxy / (Sxx * Syy) * 100.0;

        cout << std::setprecision(4);
        cout << "freq = " << (*freq_list)(f) << " Hz"
             << " bias = " << bias << " dB"
             << " dev = " << dev << " dB"
             << " detcoef = " << detcoef << "%" << endl;

        if (freq > 250.0) {
            BOOST_CHECK(abs(bias) <= 0.7);
        }
        BOOST_CHECK(dev <= 5.0);
        BOOST_CHECK(detcoef >= 80.0);
    }
}

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
 *   - Target: 0, 10, 100, 1000 meters deep
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
 * we will limit the range to an area where only two roots are supported.
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
 * beamwidth, then this test has large errors in D/E angle. This test
 * illustrates the importance of this limitation.
 *
 * @xref Weisstein, Eric W. "Newton's Method." From MathWorld--A Wolfram
 *       Web Resource. http://mathworld.wolfram.com/NewtonsMethod.html
 */
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
BOOST_AUTO_TEST_CASE(proploss_lloyds_spherical) {
    cout << "=== proploss_test: lloyds_spherical ===" << endl;
    const char* ncname_wave =
        USML_TEST_DIR "/waveq3d/test/proploss_lloyds_spherical_wave.nc";
    const char* ncname =
        USML_TEST_DIR "/waveq3d/test/proploss_lloyds_spherical.nc";
    const char* analytic_name =
        USML_TEST_DIR "/waveq3d/test/proploss_lloyds_spherical_analytic.nc";

    const double time_step = 0.100;
    const double f0 = 2000;
    const double src_lat = 45.0;
    const double src_lng = -45.0;
    const double src_alt = -200.0;
    const double c0 = 1500.0;
    const double bot_depth = 1e5;

    const double time_max = 120.0;  // let rays plots go into region w/ 2 roots

    const double rmax = 45.0 / 60.0;  // limit to area where N/R converges
    const double rmin = 1.0 / 60.0;   // 1 nmi min range
    const double rinc = 1.0 / 60.0;   // 1 nmi range inc
    const seq_linear range(rmin, rinc, rmax);  // range in latitude

    static double depth[] = {0.0, 10.0, 100.0, 1000.0};
    size_t num_depths = sizeof(depth) / sizeof(double);

    // compute eigenrays for this ocean

    wposition::compute_earth_radius(src_lat);
    attenuation_model::csptr attn(new attenuation_constant(0.0));
    profile_model::csptr profile(new profile_linear(c0, attn));
    boundary_model::csptr surface(new boundary_flat());
    boundary_model::csptr bottom(new boundary_flat(bot_depth));
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

    eigenray_collection model(freq, pos, target);
    eigenray_collection analytic(freq, pos, target);
    wave_queue wave(ocean, freq, pos, de, az, time_step, &target,
                    spreading_type);
    wave.add_eigenray_listener(&model);

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

    model.sum_eigenrays();
    cout << "writing eigenrays to " << ncname << endl;
    model.write_netcdf(ncname);

    // compare each target location to analytic results

    cout << "testing eigenrays" << endl;
    for (size_t t1 = 0; t1 < range.size(); ++t1) {
        for (size_t t2 = 0; t2 < num_depths; ++t2) {
            double time;
            double sde;
            double tde;
            double intensity;
            double phase;

            // setup analytic equations for this target

            const double R = wposition::earth_radius;
            const double xi = to_radians(target.latitude(t1, t2) - src_lat);
            const double d1 = -src_alt;
            const double d2 = -target.altitude(t1, t2);
            const double D1 = R - d1;
            const double D2 = R - d2;

            const eigenray_list& raylist = model.eigenrays(t1, t2);
            // BOOST_REQUIRE_EQUAL(raylist.size(), 2);

            for (const eigenray_model::csptr& ray : raylist) {
                // compare direct-path model to analytic results

                if (ray->surface == 0 || depth[t2] < 1e-3) {
                    // compute analytic results

                    const double L =
                        sqrt(D1 * D1 + D2 * D2 - 2.0 * D1 * D2 * cos(xi));
                    time = L / c0;
                    intensity = 20*log10(L);
                    sde = to_degrees(
                        -asin((L * L + D1 * D1 - D2 * D2) / (2.0 * L * D1)));
                    tde = to_degrees(
                        asin((L * L + D2 * D2 - D1 * D1) / (2.0 * L * D2)));
                    phase = 0.0;
                    if (ray->surface == 1) {
                        tde *= -1.0;
                        phase = -M_PI;
                    }

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
                    intensity = 20*log10(a1 + a2);
                    sde = to_degrees(
                        -asin((a1 * a1 + D1 * D1 - R * R) / (2.0 * a1 * D1)));
                    tde = to_degrees(
                        asin((a2 * a2 + D2 * D2 - R * R) / (2.0 * a2 * D2)));
                    phase = -M_PI;
                }

                // record analytic solution for this ray path

                auto* new_ray = new eigenray_model(*ray);
                new_ray->travel_time = time;
                std::fill(new_ray->intensity.begin(), new_ray->intensity.end(),
                          intensity);
                std::fill(new_ray->phase.begin(), new_ray->phase.end(), phase);
                new_ray->source_de = sde;
                new_ray->source_az = 0.0;
                new_ray->target_de = tde;
                new_ray->target_az = 0.0;
                analytic.add_eigenray(t1, t2, eigenray_model::csptr(new_ray));

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
            }  // loop through eigenrays for each target
        }      // loop through target depths
    }          // loop through target ranges

    analytic.write_netcdf(analytic_name);
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
