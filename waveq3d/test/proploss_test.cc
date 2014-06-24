/**
 * @example waveq3d/test/proploss_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <usml/waveq3d/waveq3d.h>
#include <usml/netcdf/netcdf_files.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <time.h>

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
BOOST_AUTO_TEST_CASE(proploss_basic)
{
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
    for (unsigned n = 0; n < target.size1(); ++n)
    {
        target.latitude(n, 0, src_lat + 0.01 * (n + 2.0));
    }

    proploss loss(freq, pos, de, az, time_step, &target);
    wave_queue wave( ocean, freq, pos, de, az, time_step, &target, wave_queue::CLASSIC_RAY ) ;
    wave.addEigenrayListener(&loss);

    // propagate rays & record to log file

    cout << "propagate wavefronts" << endl;
    while (wave.time() < time_max)
    {
        wave.step();
    }
    loss.sum_eigenrays();

    // save results to spreadsheet and compare to analytic results

    std::ofstream os(csvname);
    os << "target,time,intensity,phase,src de,src az,trg de,trg az,surf,bot" << endl;
    os << std::setprecision(18);

    cout << "writing spreadsheets to " << csvname << endl;
    for (unsigned n = 0; n < target.size1(); ++n)
    {
        const eigenray_list *raylist = loss.eigenrays(n, 0);
        for (eigenray_list::const_iterator iter = raylist->begin();
                iter != raylist->end(); ++n, ++iter)
        {
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
            for (unsigned f = 0; f < freq.size(); ++f)
            {
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
 *	- Source D/E:	-90 deg to 90 deg, 181 rays with tangental spacing
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
 * - "detcoef" is the coefficient of determination and its measure of the
 *   fraction of the model that predicts the analytic solution.
 *
 * An automatic error is thrown if abs(bias) > 0.5 dB, dev > 4 db, or
 * detcoef < 80%.
 *
 * @xref F.B. Jensen, W.A. Kuperman, M.B. Porter, H. Schmidt,
 * "Computational Ocean Acoustics", pp. 16-19.
 */
BOOST_AUTO_TEST_CASE(proploss_lloyds_range)
{
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

    seq_log freq(f0, 1.0, 1);  // 2000 Hz
    const double wavenum = TWO_PI * freq(0) / c0;

    wposition1 pos(src_lat, src_lng, src_alt);
    seq_rayfan de ;
    seq_linear az( -4.0, 1.0, 4.0 );

    // build a series of targets at different ranges

    seq_linear range(200.0, 10.0, 10e3); // range in meters
    wposition target(range.size(), 1, src_lat, src_lng, trg_alt);
    for (unsigned n = 0; n < target.size1(); ++n)
    {
        double degrees = src_lat + range(n) / (1852.0 * 60.0); // range in latitude
        target.latitude(n, 0, degrees);
    }

    proploss loss(freq, pos, de, az, time_step, &target);
    wave_queue wave( ocean, freq, pos, de, az, time_step, &target) ;
    wave.addEigenrayListener(&loss);

    // propagate rays & record to log file

    cout << "propagate wavefronts" << endl;
    cout << "writing wavefronts to " << ncname_wave << endl;
    wave.init_netcdf(ncname_wave);  // open a log file for wavefront data
    wave.save_netcdf();             // write ray data to log file
    while (wave.time() < time_max)
    {
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

    for (unsigned n = 0; n < range.size(); ++n)
    {
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
        if (iter != loss.eigenrays(n, 0)->end())
        {
            os << range(n)
            << "," << tl_model[n]
            << "," << tl_analytic[n]
            << "," << -(*iter).intensity(0)
            << "," << (*iter).time
            << "," << 10.0 * log10(norm(p1))
            << "," << R1 / c0;
            ++iter;
            if (iter != loss.eigenrays(n, 0)->end())
            {
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
    for (unsigned n = 0; n < range.size(); ++n)
    {
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

    BOOST_CHECK( abs(bias) <= 0.5 );
    BOOST_CHECK( dev <= 4.0 );
    BOOST_CHECK( detcoef >= 80.0 );
}

/**
 * Compares modeled propagation loss as a function of range and frequency to the Lloyd's
 * mirror analytic expression for surface reflection in an isovelocity ocean.
 *
 *      - Source:       25 meters deep
 *      - Target:       200 meters deep, range is 200-10,000 m
 *      - Frequency:    10-10000 Hz
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
 *   the difference in flucuations between the models.
 * - "detcoef" is the coefficient of determination and it measure of the
 *   fraction of the model that predicts the analytic solution.
 *
 * An automatic error is thrown if abs(bias) > 0.5 dB, detcoef < 80% or dev > 4 db.
 * Special case for dev on 10 Hz and 10000 Hz; abs(dev) > 5 db.
 *
 *
 * @xref F.B. Jensen, W.A. Kuperman, M.B. Porter, H. Schmidt,
 * "Computational Ocean Acoustics", pp. 16-19.
 */
BOOST_AUTO_TEST_CASE(proploss_lloyds_range_freq)
{
    cout << "=== proploss_test: proploss_lloyds_range_freq ===" << endl;

    const char* csvname = USML_TEST_DIR "/waveq3d/test/proploss_lloyds_range_freq.csv";
    const char* ncname = USML_TEST_DIR "/waveq3d/test/proploss_lloyds_range_freq.nc";
    const char* ncname_wave = USML_TEST_DIR "/waveq3d/test/proploss_lloyds_range_freq_wave.nc";

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

    seq_log freq(10.0, 10.0, 4);
    cout << "frequencies: " << freq << endl;

    wposition1 pos(src_lat, src_lng, src_alt);
    seq_rayfan de ;
    seq_linear az( -4.0, 1.0, 4.0 );

    // build a series of targets at different ranges

    seq_linear range(200.0, 10.0, 10e3); // range in meters
    wposition target(range.size(), 1, src_lat, src_lng, trg_alt);
    for (unsigned n = 0; n < target.size1(); ++n)
    {
        double degrees = src_lat + range(n) / (1852.0 * 60.0); // range in latitude
        target.latitude(n, 0, degrees);
    }

    proploss loss(freq, pos, de, az, time_step, &target);
    wave_queue wave( ocean, freq, pos, de, az, time_step, &target) ;
    wave.addEigenrayListener(&loss);

    // propagate rays & record to log file

    cout << "propagate wavefronts" << endl;
    cout << "writing wavefronts to " << ncname_wave << endl;
    wave.init_netcdf(ncname_wave);  // open a log file for wavefront data
    wave.save_netcdf();             // write ray data to log file
    while (wave.time() < time_max)
    {
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


    for (unsigned f=0; f < freq.size(); ++f)
    {
        const double wavenum = TWO_PI * freq(f) / c0 ;
        os << "freq: " << freq(f) << endl;

        for (unsigned n = 0; n < range.size(); ++n)
        {
            tl_model[n] = -loss.total(n, 0)->intensity(f);

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
            if (iter != loss.eigenrays(n, 0)->end())
            {
                os << range(n)                     // range
                << "," << tl_model[n]              // model
                << "," << tl_analytic[n]           // theory
                << "," << -(*iter).intensity(f)    // m1amp
                << "," << (*iter).time             // m1time
                << "," << 10.0 * log10(norm(p1))   // t1amp
                << "," << R1 / c0;                 // t1time
                ++iter;
                if (iter != loss.eigenrays(n, 0)->end())
                {
                    os << "," << -(*iter).intensity(f)  // m2amp
                    << "," << (*iter).time              // m2time
                    << "," << 10.0 * log10(norm(p2))    // t2amp
                    << "," << R2 / c0;                  // t2time
                }
            }
            os << endl;
            mean_model += tl_model[n];
            mean_analytic += tl_analytic[n];
        }
        mean_model /= range.size();
        mean_analytic /= range.size();

        // compute statistics of difference between curves for each freq

        double bias = 0.0;
        double dev = 0.0;
        double Sxx = 0.0;
        double Syy = 0.0;
        double Sxy = 0.0;
        for (unsigned n = 0; n < range.size(); ++n)
        {
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
        cout << " freq =" << freq(f) << "Hz: bias = " << bias << " dB"
         << " dev = " << dev << " dB"
         << " detcoef = " << detcoef << "%" << endl ;

        BOOST_CHECK( detcoef >= 80.0 );

        // Extreme low and high freq can have a greater dev
        int iFreq = (int)freq(f);
        switch (iFreq) {
            case 10:
                BOOST_CHECK( abs(bias) <= 10.0 );
                BOOST_CHECK( dev <= 5.0 );
                break;
            case 100:
                BOOST_CHECK( abs(bias) <= 1.0 );
                BOOST_CHECK( dev <= 4.0 );
                break;
            case 10000:
                BOOST_CHECK( dev <= 5.0 );
                break;
            default:
                BOOST_CHECK( abs(bias) <= 0.5 );
                BOOST_CHECK( dev <= 4.0 );
        }

    } // end Freq
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
BOOST_AUTO_TEST_CASE(proploss_lloyds_depth)
{
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
//    seq_rayfan de( -17.0, 17.0, 363 ) ;
//    seq_linear de( -5.0, 5.0, 720 );
    seq_rayfan de ;
    seq_linear az( -4.0, 1.0, 4.0 );

    // build a series of targets at different depths

    double degrees = src_lat + to_degrees(range / (wposition::earth_radius+src_alt)); // range in latitude
    seq_linear depth(-0.1, -0.5, -40.1); // depth in meters
//    seq_linear depth(-0.1, 1.0, 1); // depth in meters
    wposition target(depth.size(), 1, degrees, src_lng, 0.0);
    for (unsigned n = 0; n < target.size1(); ++n)
    {
        target.altitude(n, 0, depth(n));
    }
//    wposition target(1,1,degrees,src_lng,-25.0);

    proploss loss(freq, pos, de, az, time_step, &target);
    wave_queue wave( ocean, freq, pos, de, az, time_step, &target) ;
    wave.addEigenrayListener(&loss);

    // propagate rays & record to log file

    cout << "propagate wavefronts" << endl;
    cout << "writing wavefronts to " << ncname_wave << endl;
    wave.init_netcdf(ncname_wave); // open a log file for wavefront data
    wave.save_netcdf(); // write ray data to log file
    while (wave.time() < time_max)
    {
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
    os << "depth,model,theory,m1amp,t1amp,m1time,t1time,time1-diff,m2amp,t2amp,m2time,t2time,time2-diff,phase"
    << endl;
    os << std::setprecision(18);

    vector<double> tl_model(depth.size());
    vector<double> tl_analytic(depth.size());
    double mean_model = 0.0;
    double mean_analytic = 0.0;

    for (unsigned n = 0; n < depth.size(); ++n)
    {
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
        if (iter != loss.eigenrays(n, 0)->end())
        {
            os << depth(n)
            << "," << tl_model[n]
            << "," << tl_analytic[n]
            << "," << -(*iter).intensity(0)
            << "," << 10.0 * log10(norm(p1))
            << "," << (*iter).time
            << "," << R1 / c0
            << "," << (*iter).time-R1 / c0;
            ++iter;
            if (iter != loss.eigenrays(n, 0)->end())
            {
                os << "," << -(*iter).intensity(0)
                << "," << 10.0 * log10(norm(p2))
                << "," << (*iter).time
                << "," << R2 / c0
                << "," << (*iter).time-R2 / c0;
            }
            os << "," << (*iter).phase(0);
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
    for (unsigned n = 0; n < depth.size(); ++n)
    {
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

    BOOST_CHECK( abs(bias) <= 0.7 );
    BOOST_CHECK( dev <= 4.0 );
    BOOST_CHECK( detcoef >= 80.0 );
}

/** Test in progress */
BOOST_AUTO_TEST_CASE( bottom_type_effects ) {
    cout << "=== profile_model: bottom_type_effects ===" << endl;
    int month = 1 ;		        // January
    const double lat1 = 24.0 ;  // gulf of oman
    const double lat2 = 26.0 ;
    const double lng1 = 56.0 ;
    const double lng2 = 58.0 ;
    wposition::compute_earth_radius( (lat1+lat2)/2.0 ) ;

    const double src_lat = 26.0 ;
    const double src_lng = 56.75 ;
    const double src_alt = -27.0 ;
    const double tar_range = 1000.0 ;
    const double tar_alt = -50.0 ;
    const double time_max = 4.0 ;
    const double dt = 0.05 ;
    seq_log freq( 6500.0, 0.0, 1 ) ;

    wposition1 pos( src_lat, src_lng, src_alt ) ;
    seq_rayfan de( -34.0, 36.0, 21 ) ;
    seq_linear az( 0.0, 15.0, 360.0 ) ;

    // build sound velocity profile from World Ocean Atlas data
    cout << "loading temperature & salinity data from World Ocean Atlas" << endl ;
    usml::netcdf::netcdf_woa* temperature = new usml::netcdf::netcdf_woa(
                                USML_DATA_DIR "/woa09/temperature_seasonal_1deg.nc",
                                USML_DATA_DIR "/woa09/temperature_monthly_1deg.nc",
                                month, lat1, lat2, lng1, lng2 ) ;
    usml::netcdf::netcdf_woa* salinity = new usml::netcdf::netcdf_woa(
                            USML_DATA_DIR "/woa09/salinity_seasonal_1deg.nc",
                            USML_DATA_DIR "/woa09/salinity_monthly_1deg.nc",
                            month, lat1, lat2, lng1, lng2 ) ;
    profile_model* profile = new profile_grid<double,3>(
        data_grid_mackenzie::construct(temperature, salinity) ) ;

    // load bathymetry from ETOPO1 database
    cout << "loading bathymetry from ETOPO1 database" << endl ;
    boundary_model* bottom = new boundary_grid<double,2>( new usml::netcdf::netcdf_bathy(
        USML_DATA_DIR "/bathymetry/ETOPO1_Ice_g_gmt4.grd", lat1, lat2, lng1, lng2 ) );
    bottom->reflect_loss( new reflect_loss_rayleigh( reflect_loss_rayleigh::SILT ) ) ;

    boundary_model* surface = new boundary_flat() ;

    // combine sound speed and bathymetry into ocean model
    ocean_model ocean( surface, bottom, profile ) ;

    // create a target due north tar_range away
    wposition target( 1, 1, 0.0, 0.0, tar_alt ) ;
    wposition1 atarget( pos, tar_range, 0.0 ) ;
    target.latitude( 0, 0, atarget.latitude() ) ;
    target.longitude( 0, 0, atarget.longitude() ) ;

    proploss* pLoss = new proploss(freq, pos, de, az, dt, &target) ;
    wave_queue* pWave = new wave_queue( ocean, freq, pos, de, az, dt, &target ) ;
    pWave->addEigenrayListener(pLoss);

    while(pWave->time() < time_max) {
        pWave->step();
    }

    pLoss->sum_eigenrays();
    cout << "\n\n=====Eigenrays --> Rayleigh::SILT=====" << endl ;
    cout << "time (s)    bounces(s,b,c)    launch angle    arrival angle        TL            phase" << endl ;
    cout << std::setprecision(5) ;
    for (eigenray_list::const_iterator iter = pLoss->eigenrays(0, 0)->begin(); iter != pLoss->eigenrays(0, 0)->end(); ++iter)
    {
        cout << (*iter).time
        << "       (" << (*iter).surface << ", " << (*iter).bottom << ", " << (*iter).caustic << ")"
        << "\t\t" << (*iter).source_de
        << "\t\t" << (*iter).target_de
        << "\t\t  " << (*iter).intensity(0)
        << "\t " << (*iter).phase(0)
        << endl;
    }

    bottom->reflect_loss( new reflect_loss_rayleigh( reflect_loss_rayleigh::SAND ) ) ;
    delete pLoss ;
    delete pWave ;
    pLoss = new proploss(freq, pos, de, az, dt, &target) ;
    pWave = new wave_queue( ocean, freq, pos, de, az, dt, &target ) ;
    pWave->addEigenrayListener(pLoss) ;

    while(pWave->time() < time_max) {
        pWave->step();
    }

    pLoss->sum_eigenrays();
    cout << "=====Eigenrays --> Rayleigh::SAND=====" << endl ;
    cout << "time (s)    bounces(s,b,c)    launch angle    arrival angle        TL            phase" << endl ;
    cout << std::setprecision(5) ;
    for (eigenray_list::const_iterator iter = pLoss->eigenrays(0, 0)->begin(); iter != pLoss->eigenrays(0, 0)->end(); ++iter)
    {
        cout << (*iter).time
        << "       (" << (*iter).surface << ", " << (*iter).bottom << ", " << (*iter).caustic << ")"
        << "\t\t" << (*iter).source_de
        << "\t\t" << (*iter).target_de
        << "\t\t  " << (*iter).intensity(0)
        << "\t " << (*iter).phase(0)
        << endl;
    }

    bottom->reflect_loss( new reflect_loss_rayleigh( reflect_loss_rayleigh::CLAY ) ) ;
    delete pLoss ;
    delete pWave ;
    pLoss = new proploss(freq, pos, de, az, dt, &target) ;
    pWave = new wave_queue( ocean, freq, pos, de, az, dt, &target ) ;
    pWave->addEigenrayListener(pLoss) ;

    while(pWave->time() < time_max) {
        pWave->step();
    }

    pLoss->sum_eigenrays();
    cout << "=====Eigenrays --> Rayleigh::CLAY=====" << endl ;
    cout << "time (s)    bounces(s,b,c)    launch angle    arrival angle        TL            phase" << endl ;
    cout << std::setprecision(5) ;
    for (eigenray_list::const_iterator iter = pLoss->eigenrays(0, 0)->begin(); iter != pLoss->eigenrays(0, 0)->end(); ++iter)
    {
        cout << (*iter).time
        << "       (" << (*iter).surface << ", " << (*iter).bottom << ", " << (*iter).caustic << ")"
        << "\t\t" << (*iter).source_de
        << "\t\t" << (*iter).target_de
        << "\t\t  " << (*iter).intensity(0)
        << "\t " << (*iter).phase(0)
        << endl;
    }

    bottom->reflect_loss( new reflect_loss_rayleigh( reflect_loss_rayleigh::LIMESTONE ) ) ;
    delete pLoss ;
    delete pWave ;
    pLoss = new proploss(freq, pos, de, az, dt, &target) ;
    pWave = new wave_queue( ocean, freq, pos, de, az, dt, &target ) ;
    pWave->addEigenrayListener(pLoss) ;

    while(pWave->time() < time_max) {
        pWave->step();
    }

    pLoss->sum_eigenrays();
    cout << "=====Eigenrays --> Rayleigh::LIMESTONE=====" << endl ;
    cout << "time (s)    bounces(s,b,c)    launch angle    arrival angle        TL            phase" << endl ;
    cout << std::setprecision(5) ;
    for (eigenray_list::const_iterator iter = pLoss->eigenrays(0, 0)->begin(); iter != pLoss->eigenrays(0, 0)->end(); ++iter)
    {
        cout << (*iter).time
        << "       (" << (*iter).surface << ", " << (*iter).bottom << ", " << (*iter).caustic << ")"
        << "\t\t" << (*iter).source_de
        << "\t\t" << (*iter).target_de
        << "\t\t  " << (*iter).intensity(0)
        << "\t " << (*iter).phase(0)
        << endl;
    }

    bottom->reflect_loss( new reflect_loss_rayleigh( reflect_loss_rayleigh::BASALT ) ) ;
    delete pLoss ;
    delete pWave ;
    pLoss = new proploss(freq, pos, de, az, dt, &target) ;
    pWave = new wave_queue( ocean, freq, pos, de, az, dt, &target ) ;
    pWave->addEigenrayListener(pLoss) ;

    while(pWave->time() < time_max) {
        pWave->step();
    }

    pLoss->sum_eigenrays();
    cout << "=====Eigenrays --> Rayleigh::BASALT=====" << endl ;
    cout << "time (s)    bounces(s,b,c)    launch angle    arrival angle        TL            phase" << endl ;
    cout << std::setprecision(5) ;
    for (eigenray_list::const_iterator iter = pLoss->eigenrays(0, 0)->begin(); iter != pLoss->eigenrays(0, 0)->end(); ++iter)
    {
        cout << (*iter).time
        << "       (" << (*iter).surface << ", " << (*iter).bottom << ", " << (*iter).caustic << ")"
        << "\t\t" << (*iter).source_de
        << "\t\t" << (*iter).target_de
        << "\t\t  " << (*iter).intensity(0)
        << "\t " << (*iter).phase(0)
        << endl;
    }

    bottom->reflect_loss( new reflect_loss_rayleigh( reflect_loss_rayleigh::GRAVEL ) ) ;
    delete pLoss ;
    delete pWave ;
    pLoss = new proploss(freq, pos, de, az, dt, &target) ;
    pWave = new wave_queue( ocean, freq, pos, de, az, dt, &target ) ;
    pWave->addEigenrayListener(pLoss) ;

    while(pWave->time() < time_max) {
        pWave->step();
    }

    pLoss->sum_eigenrays();
    cout << "=====Eigenrays --> Rayleigh::GRAVEL=====" << endl ;
    cout << "time (s)    bounces(s,b,c)    launch angle    arrival angle        TL            phase" << endl ;
    cout << std::setprecision(5) ;
    for (eigenray_list::const_iterator iter = pLoss->eigenrays(0, 0)->begin(); iter != pLoss->eigenrays(0, 0)->end(); ++iter)
    {
        cout << (*iter).time
        << "       (" << (*iter).surface << ", " << (*iter).bottom << ", " << (*iter).caustic << ")"
        << "\t\t" << (*iter).source_de
        << "\t\t" << (*iter).target_de
        << "\t\t  " << (*iter).intensity(0)
        << "\t " << (*iter).phase(0)
        << endl;
    }

    bottom->reflect_loss( new reflect_loss_rayleigh( reflect_loss_rayleigh::MORAINE ) ) ;
    delete pLoss ;
    delete pWave ;
    pLoss = new proploss(freq, pos, de, az, dt, &target) ;
    pWave = new wave_queue( ocean, freq, pos, de, az, dt, &target ) ;
    pWave->addEigenrayListener(pLoss) ;

    while(pWave->time() < time_max) {
        pWave->step();
    }

    pLoss->sum_eigenrays();
    cout << "=====Eigenrays --> Rayleigh::MORAINE=====" << endl ;
    cout << "time (s)    bounces(s,b,c)    launch angle    arrival angle        TL            phase" << endl ;
    cout << std::setprecision(5) ;
    for (eigenray_list::const_iterator iter = pLoss->eigenrays(0, 0)->begin(); iter != pLoss->eigenrays(0, 0)->end(); ++iter)
    {
        cout << (*iter).time
        << "       (" << (*iter).surface << ", " << (*iter).bottom << ", " << (*iter).caustic << ")"
        << "\t\t" << (*iter).source_de
        << "\t\t" << (*iter).target_de
        << "\t\t  " << (*iter).intensity(0)
        << "\t " << (*iter).phase(0)
        << endl;
    }

    bottom->reflect_loss( new reflect_loss_rayleigh( reflect_loss_rayleigh::CHALK ) ) ;
    delete pLoss ;
    delete pWave ;
    pLoss = new proploss(freq, pos, de, az, dt, &target) ;
    pWave = new wave_queue( ocean, freq, pos, de, az, dt, &target ) ;
    pWave->addEigenrayListener(pLoss) ;

    while(pWave->time() < time_max) {
        pWave->step();
    }

    pLoss->sum_eigenrays();
    cout << "=====Eigenrays --> Rayleigh::CHALK=====" << endl ;
    cout << "time (s)    bounces(s,b,c)    launch angle    arrival angle        TL            phase" << endl ;
    cout << std::setprecision(5) ;
    for (eigenray_list::const_iterator iter = pLoss->eigenrays(0, 0)->begin(); iter != pLoss->eigenrays(0, 0)->end(); ++iter)
    {
        cout << (*iter).time
        << "       (" << (*iter).surface << ", " << (*iter).bottom << ", " << (*iter).caustic << ")"
        << "\t\t" << (*iter).source_de
        << "\t\t" << (*iter).target_de
        << "\t\t  " << (*iter).intensity(0)
        << "\t " << (*iter).phase(0)
        << endl;
    }
}

/**
 * Compare results produced by BELLHOP for a Munk SSP
 * Transmission loss plot to a WaveQ3D equivalent.
 */

BOOST_AUTO_TEST_CASE( munk_tl_plot ) {
    cout << "==== proploss_test: munk_tl_plot ====" << endl;

    // Setup scenario parameters
    int num_range_tar = 10 ;
    int num_alt_tar = 10 ;
    double c0 = 1500.0 ;
    wposition1 pos( 45.0, -45.0, -1000.0 ) ;
    double range = 101000.0 ;
    double time_step = 0.07 ;
    double time_max = range/c0 * 1.2 ;
    double depth = 5000.0 ;
    double m_2_deg = 1.0 / ( 1852.0 * 60.0 ) ;            // conversion factor from meters to degree lat

    // initialize propagation model

    wposition::compute_earth_radius(pos.latitude()) ;
    attenuation_model* attn = new attenuation_constant(0.0) ;
    profile_model* profile = new profile_munk(1300.0, 1300.0, 1500.0, 7.37e-3, attn) ;
    boundary_model* surface = new boundary_flat() ;
//    reflect_loss_model* loss_mod = new reflect_loss_rayleigh(1.8, 1600, 0.8) ;
    boundary_model* bottom = new boundary_flat(depth);
    ocean_model ocean(surface, bottom, profile) ;
    profile->flat_earth(true);

    seq_log freq( 50.0, 50.0, 1 ) ;
    seq_rayfan de( -13.0, 13.0, 70 ) ;
    seq_linear az( -4.0, 1.0, 4.0 );

    // setup files to output all data to

//    const char* csvname = USML_TEST_DIR "/waveq3d/test/munk_tl_eigenray.csv";
    const char* ncname = USML_TEST_DIR "/waveq3d/test/munk_tl_proploss.nc";
    const char* ncname_wave = USML_TEST_DIR "/waveq3d/test/munk_tl_eigenray_wave.nc";

    // build a series of targets at different depths
    double inc_range = (range * m_2_deg) / num_range_tar ;
    double inc_alt = depth / num_alt_tar ;
    wposition target( num_range_tar, num_alt_tar, pos.latitude() , pos.longitude(), 0.0 ) ;
    for(int i=0; i<target.size1(); ++i) {
        for(int j=0; j<target.size2(); ++j) {
            target.latitude( i, j, pos.latitude() + inc_range * i ) ;
            target.altitude( i, j, -inc_alt * j ) ;
        }
    }
    proploss loss( freq, pos, de, az, time_step, &target ) ;
    wave_queue wave( ocean, freq, pos, de, az, time_step, &target ) ;
    wave.addEigenrayListener( &loss ) ;

    cout << "propagate wavefronts for " << time_max << endl;
    cout << "writing wavefronts to " << ncname_wave << endl;
    wave.init_netcdf(ncname_wave); // open a log file for wavefront data
    wave.save_netcdf(); // write ray data to log file
    while(wave.time() < time_max) {
        wave.step();
        wave.save_netcdf(); // write ray data to log file
    }
    wave.close_netcdf(); // close log file for wavefront data
    cout << "writing eigenrays to " << ncname << endl;
    loss.sum_eigenrays();
    loss.write_netcdf(ncname);

    cout << "***TL at Target 101km away and 1000 deep***" << endl;
    cout << std::setprecision(8);
    int rng_inx = num_range_tar - 1 ;
    int dpt_inx = 1000.0 / inc_alt ;
    cout << "TL: " << -loss.total(rng_inx, dpt_inx)->intensity(0) << endl;
//    cout << "TL: " << -loss.total(0,0)->intensity(0) << endl;
}

BOOST_AUTO_TEST_CASE( surface_duct_tl_plot ) {
    cout << "==== proploss_test: surface_duct_tl_plot ====" << endl;

    // Setup scenario parameters
    int num_range_tar = 10 ;
    int num_alt_tar = 10 ;
    double c0 = 1476.0 ;
    wposition1 pos( 45.0, -45.0, -25.0 ) ;
    double range = 151000.0 ;
    double time_step = 0.1 ;
    double time_max = range/c0 * 1.05 ;
    double depth = 4000.0 ;
    wposition::compute_earth_radius(pos.latitude()) ;
    double m_2_deg = 1.0 / ( 1852.0 * 60.0 ) ;            // conversion factor from meters to degree lat

    // Construct a 2-D Sound Speed Grid
    seq_vector* _axis[1] ;
    double depths [] = {0.0, -250.0, -300.0, -375.0, -425.0, -500.0, -600.0,
                       -700.0, -810.0, -900.0, -1000.0, -1100.0, -1180.0,
                       -1340.0, -1600.0, -1800.0, -2500.0, -3000.0, -4000.0} ;
    for(int i=0; i<sizeof(depths)/sizeof(double); ++i) {
        depths[i] += wposition::earth_radius ;
    }
    const double ssp [] = {1497.0, 1502.0, 1485.0, 1478.0, 1477.0, 1476.0, 1476.5,
                           1477.0, 1478.0, 1479.0, 1480.0, 1481.0, 1482.0, 1484.0,
                           1487.0, 1490.0, 1498.7, 1506.8, 1523.9} ;
    _axis[0] = new seq_data(depths, sizeof(depths)/sizeof(double)) ;
    data_grid<double,1>* duct_ssp = new data_grid<double,1>(_axis) ;
    for(unsigned int i=0; i<19; ++i) {
        unsigned int index [1] = {i};
        duct_ssp->data(index, ssp[i]) ;
    }

    // initialize propagation model
    wposition::compute_earth_radius(pos.latitude()) ;
    attenuation_model* attn = new attenuation_constant(0.0) ;
    profile_model* profile = new profile_grid<double,1>(duct_ssp, attn) ;
    boundary_model* surface = new boundary_flat() ;
//    reflect_loss_model* loss_mod = new reflect_loss_rayleigh(1.8, 1600, 0.8) ;
    boundary_model* bottom = new boundary_flat(depth);
    ocean_model ocean(surface, bottom, profile) ;
    profile->flat_earth(true);

//    cout << "Depth\tSpeed" << endl;
//    for(int i=0; i<19; ++i) {
//        matrix<double> ssp(1,1) ;
//        wposition pos(1,1, 45.0, -45.0, depths[i]-wposition::earth_radius) ;
//        ocean.profile().sound_speed(pos, &ssp) ;
//        cout << depths[i]-wposition::earth_radius << "\t" << ssp(0,0) << endl;
//    }

    seq_log freq( 200.0, 200.0, 1 ) ;
    seq_rayfan de( -10.0, 10.0, 70 ) ;
    seq_linear az( -4.0, 1.0, 4.0 );

    // setup files to output all data to

    const char* ncname = USML_TEST_DIR "/waveq3d/test/surface_duct_tl_proploss.nc";
    const char* ncname_wave = USML_TEST_DIR "/waveq3d/test/surface_duct_tl_eigenray_wave.nc";
    const char* csvname_ssp = USML_TEST_DIR "/waveq3d/test/surface_duct_tl_ssp.csv";

    // build a series of targets at different depths
    double inc_range = (range * m_2_deg) / num_range_tar ;
    double inc_alt = depth / num_alt_tar ;
    wposition target( num_range_tar, num_alt_tar, pos.latitude() , pos.longitude(), 0.0 ) ;
    for(int i=0; i<target.size1(); ++i) {
        for(int j=0; j<target.size2(); ++j) {
            target.latitude( i, j, pos.latitude() + inc_range * i ) ;
            target.altitude( i, j, -inc_alt * j ) ;
        }
    }
    proploss loss( freq, pos, de, az, time_step, &target );
    wave_queue wave( ocean, freq, pos, de, az, time_step, &target ) ;
    wave.addEigenrayListener(&loss);

    cout << "propagate wavefronts for " << time_max << endl;
    cout << "writing wavefronts to " << ncname_wave << endl;
    wave.init_netcdf(ncname_wave); // open a log file for wavefront data
    wave.save_netcdf(); // write ray data to log file
    while(wave.time() < time_max) {
        wave.step();
        wave.save_netcdf(); // write ray data to log file
    }
    wave.close_netcdf(); // close log file for wavefront data
    cout << "writing eigenrays to " << ncname << endl;
    loss.sum_eigenrays();
    loss.write_netcdf(ncname);

    cout << "writing ssp table to " << csvname_ssp << endl;
    std::ofstream os(csvname_ssp);
    os << "depth,ssp" << endl;
    os << std::setprecision(18);
    cout << std::setprecision(18);

    seq_linear dpt(0.0, 5.0, 4000.0) ;

    for(int n = 0; n < dpt.size() ; ++n)
    {
        matrix<double> ssp(1,1) ;
        wposition pos(1, 1, 45.0, -45.0, -dpt(n) ) ;
        ocean.profile().sound_speed(pos, &ssp) ;
        os << -dpt(n) << ","
           << ssp(0,0)
           << endl;
    }
}

BOOST_AUTO_TEST_CASE( lloyds_mirror_tl_plot ) {
    cout << "==== proploss_test: lloyds_mirror_tl_plot ====" << endl;
        //Display start time of program
    struct tm finish_time ;
    time_t rawtime ;
    time( &rawtime ) ;
    localtime_r( &rawtime, &finish_time ) ;
    cout << asctime(&finish_time) ;

    // Setup scenario parameters
    int num_range_tar = 10 ;
    int num_alt_tar = 10 ;
    double c0 = 1500.0 ;
    wposition1 pos( 45.0, -45.0, -25.0 ) ;
    double range = 1e4 ;
    double time_step = 0.1 ;
    double time_max = range/c0 * 1.05 ;
    double depth = 1e7 ;
    wposition::compute_earth_radius(pos.latitude()) ;
    double m_2_deg = 1.0 / ( 1852.0 * 60.0 ) ;            // conversion factor from meters to degree lat

    // initialize propagation model
    wposition::compute_earth_radius(pos.latitude()) ;
    attenuation_model* attn = new attenuation_constant(0.0) ;
    profile_model* profile = new profile_linear(c0, attn) ;
    boundary_model* surface = new boundary_flat() ;
    boundary_model* bottom = new boundary_flat(depth);
    ocean_model ocean(surface, bottom, profile) ;
    profile->flat_earth(true);

    seq_log freq( 150.0, 150.0, 1 ) ;
    seq_rayfan de ;
    seq_linear az( -4.0, 1.0, 4.0 );

    // setup files to output all data to

    const char* ncname = USML_TEST_DIR "/waveq3d/test/lloyds_mirror_tl_proploss.nc";
    const char* ncname_wave = USML_TEST_DIR "/waveq3d/test/lloyds_mirror_tl_eigenray_wave.nc";

    // build a series of targets at different depths
    double inc_range = (range * m_2_deg) / num_range_tar ;
    double inc_alt = 200.0 / num_alt_tar ;
    wposition target( num_range_tar, num_alt_tar, pos.latitude() , pos.longitude(), 0.0 ) ;
    for(int i=0; i<target.size1(); ++i) {
        for(int j=0; j<target.size2(); ++j) {
            target.latitude( i, j, pos.latitude() + inc_range * i ) ;
            target.altitude( i, j, -inc_alt * j ) ;
        }
    }
    proploss loss( freq, pos, de, az, time_step, &target ) ;
    wave_queue wave( ocean, freq, pos, de, az, time_step, &target ) ;
    wave.addEigenrayListener( &loss ) ;

    cout << "propagate wavefronts for " << time_max << endl;
    cout << "writing wavefronts to " << ncname_wave << endl;
    wave.init_netcdf(ncname_wave); // open a log file for wavefront data
    wave.save_netcdf(); // write ray data to log file
    while(wave.time() < time_max) {
        wave.step();
        wave.save_netcdf(); // write ray data to log file
    }
    wave.close_netcdf(); // close log file for wavefront data
    cout << "writing eigenrays to " << ncname << endl;
    loss.sum_eigenrays();
    loss.write_netcdf(ncname);

        //Display finish time of program
    time( &rawtime ) ;
    localtime_r( &rawtime, &finish_time ) ;
    cout << asctime(&finish_time) ;

}

/// @}

BOOST_AUTO_TEST_SUITE_END()
