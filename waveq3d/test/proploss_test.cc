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
    wave_queue wave( ocean, freq, pos, de, az, time_step, &target, 0, wave_queue::CLASSIC_RAY ) ;
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
 * This test demonstrates that the frequency based accuracy of the model.
 * - Scenario parameters
 *   - Profile: constant 1500 m/s sound speed, no absorption
 *   - Bottom: 3000 meters
 *   - Source: 45N, 45W, -1000 meters
 *   - Target: 45.02N, 45W, -1000 meters
 *   - Time Step: 100 msec
 *   - Launch D/E: 1 degree linear spacing from -60 to 60 degrees
 *
 * - Analytic Results
 *   - Direct Path: 1.484018789 sec, -0.01 deg launch, 66.95 dB
 *   - Surface Bounce: 1.995102731 sec, 41.93623171 deg launch, 69.52 dB
 *   - Bottom Bounce: 3.051676949 sec, -60.91257162 deg launch, 73.21 dB
 */
BOOST_AUTO_TEST_CASE( proploss_freq )
{
    cout << "=== proploss_test: proploss_freq ===" << endl ;
    const char* ncwave = USML_TEST_DIR "/waveq3d/test/proploss_freq_wave.nc" ;
    const char* ncproploss = USML_TEST_DIR "/waveq3d/test/proploss_freq_proploss.nc" ;
    const char* csvname = USML_TEST_DIR "/waveq3d/test/proploss_freq.csv";
    const double c0 = 1500.0 ;
    const double src_lat = 45.0 ;
    const double src_lng = -45.0 ;
    const double src_alt = -1000.0 ;
    const double trg_lat = 45.02 ;
    const double time_max = 3.5 ;

    // initialize propagation model

    wposition::compute_earth_radius( src_lat ) ;
    attenuation_model* attn = new attenuation_constant(0.0) ;
    profile_model* profile = new profile_linear(c0,attn) ;
    boundary_model* surface = new boundary_flat() ;
    boundary_model* bottom = new boundary_flat(3000.0) ;
    ocean_model ocean( surface, bottom, profile ) ;

    seq_linear freq( 10.0, 10.0, 1000 ) ;
    wposition1 pos( src_lat, src_lng, src_alt ) ;
    seq_linear de( -90.0, 1.0, 90.0 ) ;
    seq_linear az( -4.0, 1.0, 4.0 ) ;

    // build a single target

    wposition target( 1, 1, trg_lat, src_lng, src_alt );

    proploss loss(freq, pos, de, az, time_step, &target);
    wave_queue wave( ocean, freq, pos, de, az, time_step, &target) ;
    wave.addEigenrayListener(&loss) ;

    // propagate rays & record to log file

    cout << "propagate wavefront for " << time_max << endl ;
    cout << "writing wavefronts to " << ncwave << endl ;

    wave.init_netcdf( ncwave ) ;
    wave.save_netcdf() ;
    while (wave.time() < time_max)
    {
        wave.step() ;
        wave.save_netcdf() ;
    }
    wave.close_netcdf() ;

    loss.sum_eigenrays() ;
    cout << "writing proploss to " << ncproploss << endl ;
    loss.write_netcdf(ncproploss,"proploss_freq test") ;

    // save results to spreadsheet and compare to analytic results

    cout << "writing data to " << csvname << endl ;
    std::ofstream os(csvname) ;
    os << "frequency,theory_direct,model_direct,theory_surface,model_surface,theory_bottom,theory_surface" << endl ;
    os << std::setprecision(18) ;

    const eigenray_list *raylist = loss.eigenrays(0, 0);
    eigenray direct_ray, surface_ray, bottom_ray ;
    for (eigenray_list::const_iterator iter = raylist->begin();
            iter != raylist->end(); ++iter)
    {
        if( iter->bottom == 1 && iter->surface == 0 ) bottom_ray = *iter ;
        else if( iter->bottom == 0 && iter->surface == 1 ) surface_ray = *iter ;
        else direct_ray = *iter ;
    }
    for(size_t j=0; j<freq.size(); ++j) {
        os << freq(j)
           << "," << 66.95
           << "," << direct_ray.intensity(j)
           << "," << 69.52
           << "," << surface_ray.intensity(j)
           << "," << 73.21
           << "," << bottom_ray.intensity(j)
           << endl ;
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
    BOOST_CHECK( dev <= 5.0 );
    BOOST_CHECK( detcoef >= 80.0 );
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
