/**
 * @example waveq3d/test/proploss_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <usml/waveq3d/waveq3d.h>
#include <iostream>
#include <iomanip>
#include <fstream>

BOOST_AUTO_TEST_SUITE(proploss_test)

using namespace boost::unit_test;
using namespace usml::waveq3d;

/**
 * @ingroup waveq3d_test
 * @{
 */

static const double time_step = 0.100 ;
static const double f0 = 2000 ;
static const double bot_depth = 1e5 ;

/**
 * This test demonstrates that the  basic functions of the spreading loss model
 * are working correctly.  It computes propagation loss in a spherical spreading
 * in an isovelocity ocean with no boundary interaction.
 *
 *      - Source:       15000 meters deep
 *      - Target:       15000 meters deep, latitude change is 0.01-18.01 deg
 *      - Frequency:    10, 100, 1000, & 10000 Hz
 *      - Sound Speed:  1500 m/s
 *      - Time Step:    100 msec
 *	- Source D/E:	-10 deg to 10 deg in 1 deg increments
 *	- Source AZ:	-10 deg to 10 deg in 2 deg increments
 *
 * BOOST errors are automatically generated if the modeled eigenrays
 * deviate from the 20*log10(R) model by more than 0.2 dB.
 *
 * The eigenrays for all targets are written to the proploss_basic.csv
 * spreadsheet file.  A comparison of the model vs. theory is printed to
 * standard output.
 */
BOOST_AUTO_TEST_CASE(proploss_basic) {
    cout << "=== proploss_test: proploss_basic ===" << endl;
    const char* csvname = USML_TEST_DIR "/waveq3d/test/proploss_basic.csv";
    const double c0 = 1500.0;
    const double src_lat = 45.0;
    const double src_lng = -45.0;
    const double src_alt = -15000.0;
    const double time_max = 8.0;

    // initialize propagation model

    wposition::compute_earth_radius(src_lat);
    attenuation_model* attn = new attenuation_constant(0.0);
    profile_model* profile = new profile_linear(c0, attn);
    boundary_model* surface = new boundary_flat();
    boundary_model* bottom = new boundary_flat(bot_depth);
    ocean_model ocean(surface, bottom, profile);

    seq_log freq(10.0, 10.0, 4);
    cout << "frequencies: " << freq << endl;
    wposition1 pos(src_lat, src_lng, src_alt);
    seq_linear de(-10.7, 1.0, 10.0);
    seq_linear az(-10.5, 2.0, 10.0);

    // build a series of targets at different ranges

    wposition target(10, 1, src_lat, src_lng, src_alt);
    for (unsigned n = 0; n < target.size1(); ++n) {
        target.latitude(n, 0, src_lat + 0.01 * (n + 2.0));
    }
    proploss loss(&target);
    wave_queue wave( ocean, freq, pos, de, az, time_step, &loss, wave_queue::CLASSIC_RAY ) ;

    // propagate rays & record to log file

    cout << "propagate wavefronts" << endl;
    while (wave.time() < time_max) {
        wave.step();
    }
    loss.sum_eigenrays();

    // save results to spreadsheet and compare to analytic results

    std::ofstream os(csvname);
    os << "target,time,intensity,phase,src de,src az,trg de,trg az,surf,bot" << endl;
    os << std::setprecision(18);

    cout << "writing spreadsheets to " << csvname << endl;
    for (unsigned n = 0; n < target.size1(); ++n) {
        const eigenray_list *raylist = loss.eigenrays(n, 0);
        for (eigenray_list::const_iterator iter = raylist->begin();
                iter != raylist->end(); ++n, ++iter) {
            const eigenray &ray = *iter;
            os << n
               << "," << ray.time
               << "," << ray.intensity(0)
               << "," << ray.phase(0)
               << "," << ray.source_de
               << "," << ray.source_az
               << "," << ray.target_de
               << "," << ray.target_az
               << "," << ray.surface
               << "," << ray.bottom
               << endl;
            double range = c0 * ray.time;
            double pl = 20.0 * log10(range - 2.0);
            cout << "range=" << range
                 << " theory=" << pl
                 << " model=" << ray.intensity << endl;
            for (unsigned f = 0; f < freq.size(); ++f) {
                BOOST_CHECK(fabs(ray.intensity(f) - pl) < 0.2);
            }
        }
    }
}

/**
 * Compares modeled propagation loss as a function of range to the Lloyd's
 * mirror analytic expression for surface reflection in an isovelocity ocean.
 *
 *      - Source:       25 meters deep
 *      - Target:       200 meters deep, range is 200-10,000 m
 *      - Frequency:    2000 Hz
 *      - Sound Speed:  1500 m/s
 *      - Time Step:    50 msec
 *	- Source D/E:	-90 deg to 90 deg, 181 rays with tangent spacing
 *	- Source AZ:	-2 deg to 2 deg in 1 deg increments
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
 * - "detcoef" is the coefficient of determination and it measure of the
 *   fraction of the model that is predicts the analytic solution.
 *
 * An automatic error is thrown if bias > 0.5 dB, dev > 4 db, or
 * detcoef < 80%.
 *
 * @xref F.B. Jensen, W.A. Kuperman, M.B. Porter, H. Schmidt,
 * "Computational Ocean Acoustics", pp. 16-19.
 */
BOOST_AUTO_TEST_CASE(proploss_lloyds_range) {
    cout << "=== proploss_test: proploss_lloyds_range ===" << endl;
    const char* csvname = USML_TEST_DIR "/waveq3d/test/proploss_lloyds_range.csv";
    const char* ncname = USML_TEST_DIR "/waveq3d/test/proploss_lloyds_range.nc";
    const char* ncname_wave = USML_TEST_DIR "/waveq3d/test/proploss_lloyds_range_wave.nc";

    const double c0 = 1500.0;
    const double src_lat = 45.0;
    const double src_lng = -45.0;
    const double src_alt = -25.0;
    const double trg_alt = -200.0;
    const double time_max = 8.0;

    // initialize propagation model

    wposition::compute_earth_radius(src_lat);
    attenuation_model* attn = new attenuation_constant(0.0);
    profile_model* profile = new profile_linear(c0, attn);
    boundary_model* surface = new boundary_flat();
    boundary_model* bottom = new boundary_flat(bot_depth);
    ocean_model ocean(surface, bottom, profile);
    profile->flat_earth(true);

    seq_log freq(f0, 1.0, 1);
    const double wavenum = TWO_PI * freq(0) / c0;

    wposition1 pos(src_lat, src_lng, src_alt);
    seq_rayfan de ;
    seq_linear az( -4.0, 1.0, 4.0 );

    // build a series of targets at different ranges

    seq_linear range(200.0, 10.0, 10e3); // range in meters
    wposition target(range.size(), 1, src_lat, src_lng, trg_alt);
    for (unsigned n = 0; n < target.size1(); ++n) {
        double degrees = src_lat + range(n) / (1852.0 * 60.0); // range in latitude
        target.latitude(n, 0, degrees);
    }
    proploss loss(&target);
    wave_queue wave( ocean, freq, pos, de, az, time_step, &loss );

    // propagate rays & record to log file

    cout << "propagate wavefronts" << endl;
    cout << "writing wavefronts to " << ncname_wave << endl;
    wave.init_netcdf(ncname_wave);  // open a log file for wavefront data
    wave.save_netcdf();             // write ray data to log file
    while (wave.time() < time_max) {
        wave.step();
        wave.save_netcdf();         // write ray data to log file
    }
    wave.close_netcdf();            // close log file for wavefront data
    cout << "writing eigenrays to " << ncname << endl;
    loss.sum_eigenrays();
    loss.write_netcdf(ncname);

    // save results to spreadsheet and compare to analytic results

    cout << "writing spreadsheets to " << csvname << endl;
    std::ofstream os(csvname);
    os << "range,model,theory,m1amp,m1time,t1amp,t1time,m2amp,m2time,t2amp,t2time" << endl;
    os << std::setprecision(18);

    vector<double> tl_model(range.size());
    vector<double> tl_analytic(range.size());
    double mean_model = 0.0;
    double mean_analytic = 0.0;

    const double z1 = trg_alt - src_alt;
    const double z2 = trg_alt + src_alt;

    for (unsigned n = 0; n < range.size(); ++n) {
        tl_model[n] = -loss.total(n, 0)->intensity(0);

        // compute analytic solution

        const double R1 = sqrt(range(n) * range(n) + z1 * z1);
        const double R2 = sqrt(range(n) * range(n) + z2 * z2);
        complex<double> p1(0.0, wavenum * R1);
        complex<double> p2(0.0, wavenum * R2);
        p1 = exp(p1) / R1;
        p2 = -exp(p2) / R2;
        tl_analytic[n] = 10.0 * log10(norm(p1 + p2));

        // print to log file

        eigenray_list::const_iterator iter = loss.eigenrays(n, 0)->begin();
        if (iter != loss.eigenrays(n, 0)->end()) {
            os << range(n)
               << "," << tl_model[n]
               << "," << tl_analytic[n]
               << "," << -(*iter).intensity(0)
               << "," << (*iter).time
               << "," << 10.0 * log10(norm(p1))
               << "," << R1 / c0;
            ++iter;
            if (iter != loss.eigenrays(n, 0)->end()) {
                os << "," << -(*iter).intensity(0)
                   << "," << (*iter).time
                   << "," << 10.0 * log10(norm(p2))
                   << "," << R2 / c0;
            }
        }
        os << endl;
        mean_model += tl_model[n];
        mean_analytic += tl_analytic[n];
    }
    mean_model /= range.size();
    mean_analytic /= range.size();

    // compute statistics of difference between curves

    double bias = 0.0;
    double dev = 0.0;
    double Sxx = 0.0;
    double Syy = 0.0;
    double Sxy = 0.0;
    for (unsigned n = 0; n < range.size(); ++n) {
        const double diff = (tl_model[n] - tl_analytic[n]);
        bias += diff ;
        dev += (diff * diff);
        const double diff_analytic = (tl_analytic[n] - mean_analytic);
        Sxx += (diff_analytic * diff_analytic);
        const double diff_model = (tl_model[n] - mean_model);
        Syy += (diff_model * diff_model);
        Sxy += (diff_analytic * diff_model);
    }
    bias /= range.size() ;
    dev = sqrt( dev / range.size() - bias*bias ) ;
    double detcoef = Sxy * Sxy / (Sxx * Syy) * 100.0;

    cout << std::setprecision(4);
    cout << "bias = " << bias << " dB"
         << " dev = " << dev << " dB"
         << " detcoef = " << detcoef << "%" << endl ;

    BOOST_CHECK( bias <= 0.5 );
    BOOST_CHECK( dev <= 4.0 );
    BOOST_CHECK( detcoef >= 80.0 );
}

/**
 * Compares modeled propagation loss as a function of depth to the Lloyd's
 * mirror analytic expression for surface reflection in an isovelocity ocean.
 * This forces the model to deal with target points near the surface where the
 * up-going and down-going wavefronts must be extrapolated from two ray
 * families that have different numbers of surface bounces.
 *
 *      - Source:       25 meters deep
 *      - Target:       Range 2000 meters, depth is 1-50 meters
 *      - Frequency:    2000 Hz
 *      - Sound Speed:  1500 m/s
 *      - Time Step:    50 msec
 *	- Source D/E:	-90 deg to 90 deg, 181 rays with tangent spacing
 *	- Source AZ:	-2 deg to 2 deg in 1 deg increments
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
 *   the difference in flucuations of between the models.
 * - "detcoef" is the coefficient of determination and it measure of the
 *   fraction of the model that is predicts the analytic solution.
 *
 * An automatic error is thrown if bias > 0.5 dB, dev > 4 db, or
 * detcoef < 80%.
 *
 * @xref F.B. Jensen, W.A. Kuperman, M.B. Porter, H. Schmidt,
 * "Computational Ocean Acoustics", pp. 16-19.
 */
BOOST_AUTO_TEST_CASE(proploss_lloyds_depth) {
    cout << "=== proploss_test: proploss_lloyds_depth ===" << endl;
    const char* csvname = USML_TEST_DIR "/waveq3d/test/proploss_lloyds_depth.csv";
    const char* ncname = USML_TEST_DIR "/waveq3d/test/proploss_lloyds_depth.nc";
    const char* ncname_wave = USML_TEST_DIR "/waveq3d/test/proploss_lloyds_depth_wave.nc";
    const double c0 = 1500.0;
    const double src_lat = 45.0;
    const double src_lng = -45.0;
    const double src_alt = -25.0;
    const double range = 10e3;
    const double time_max = 8.0;

    // initialize propagation model

    wposition::compute_earth_radius(src_lat);
    attenuation_model* attn = new attenuation_constant(0.0);
    profile_model* profile = new profile_linear(c0, attn);
    boundary_model* surface = new boundary_flat();
    boundary_model* bottom = new boundary_flat(bot_depth);
    ocean_model ocean(surface, bottom, profile);
    profile->flat_earth(true);

    seq_log freq(f0, 1.0, 1);
    const double wavenum = TWO_PI * freq(0) / c0;

    wposition1 pos(src_lat, src_lng, src_alt);
    seq_rayfan de ;
    seq_linear az( -4.0, 1.0, 4.0 );

    // build a series of targets at different depths

    double degrees = src_lat + range / (1852.0 * 60.0); // range in latitude
    seq_linear depth(-40.1, 0.5, -0.1); // depth in meters
//    seq_linear depth(-0.1, 1.0, 1); // depth in meters
    wposition target(depth.size(), 1, degrees, src_lng, 0.0);
    for (unsigned n = 0; n < target.size1(); ++n) {
        target.altitude(n, 0, depth(n));
    }
    proploss loss(&target);
    wave_queue wave( ocean, freq, pos, de, az, time_step, &loss );

    // propagate rays & record to log file

    cout << "propagate wavefronts" << endl;
    cout << "writing wavefronts to " << ncname_wave << endl;
    wave.init_netcdf(ncname_wave); // open a log file for wavefront data
    wave.save_netcdf(); // write ray data to log file
    while (wave.time() < time_max) {
        wave.step();
        wave.save_netcdf(); // write ray data to log file
    }
    wave.close_netcdf(); // close log file for wavefront data
    cout << "writing eigenrays to " << ncname << endl;
    loss.sum_eigenrays();
    loss.write_netcdf(ncname);

    // save results to spreadsheet and compare to analytic results

    cout << "writing spreadsheets to " << csvname << endl;
    std::ofstream os(csvname);
    os << "depth,model,theory,m1amp,m1time,t1amp,t1time,m2amp,m2time,t2amp,t2time"
            << endl;
    os << std::setprecision(18);

    vector<double> tl_model(depth.size());
    vector<double> tl_analytic(depth.size());
    double mean_model = 0.0;
    double mean_analytic = 0.0;

    for (unsigned n = 0; n < depth.size(); ++n) {
        const double z1 = depth(n) - src_alt;
        const double z2 = depth(n) + src_alt;

        tl_model[n] = -loss.total(n, 0)->intensity(0);

        // compute analytic solution

        const double R1 = sqrt(range * range + z1 * z1);
        const double R2 = sqrt(range * range + z2 * z2);
        complex<double> p1(0.0, wavenum * R1);
        complex<double> p2(0.0, wavenum * R2);
        p1 = exp(p1) / R1;
        p2 = -exp(p2) / R2;
        tl_analytic[n] = 10.0 * log10(norm(p1 + p2));

        // print to log file

        eigenray_list::const_iterator iter = loss.eigenrays(n, 0)->begin();
        if (iter != loss.eigenrays(n, 0)->end()) {
            os << depth(n)
               << "," << tl_model[n]
               << "," << tl_analytic[n]
               << "," << -(*iter).intensity(0)
               << "," << (*iter).time
               << "," << 10.0 * log10(norm(p1))
               << "," << R1 / c0;
            ++iter;
            if (iter != loss.eigenrays(n, 0)->end()) {
                os << "," << -(*iter).intensity(0)
                   << "," << (*iter).time
                   << "," << 10.0 * log10(norm(p2))
                   << "," << R2 / c0;
            }
        }
        os << endl;
        mean_model += tl_model[n];
        mean_analytic += tl_analytic[n];
    }
    mean_model /= depth.size();
    mean_analytic /= depth.size();

    // compute statistics of difference between curves

    double bias = 0.0;
    double dev = 0.0;
    double Sxx = 0.0;
    double Syy = 0.0;
    double Sxy = 0.0;
    for (unsigned n = 0; n < depth.size(); ++n) {
        const double diff = (tl_model[n] - tl_analytic[n]);
        bias += diff ;
        dev += (diff * diff);
        const double diff_analytic = (tl_analytic[n] - mean_analytic);
        Sxx += (diff_analytic * diff_analytic);
        const double diff_model = (tl_model[n] - mean_model);
        Syy += (diff_model * diff_model);
        Sxy += (diff_analytic * diff_model);
    }
    bias /= depth.size() ;
    dev = sqrt( dev / depth.size() - bias*bias ) ;
    double detcoef = Sxy * Sxy / (Sxx * Syy) * 100.0;

    cout << std::setprecision(4);
    cout << "bias = " << bias << " dB"
         << " dev = " << dev << " dB"
         << " detcoef = " << detcoef << "%" << endl ;

    BOOST_CHECK( bias <= 0.5 );
    BOOST_CHECK( dev <= 4.0 );
    BOOST_CHECK( detcoef >= 80.0 );
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
