/**
 * @example waveq3d/test/eigenray_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <usml/waveq3d/waveq3d.h>
#include <iostream>
#include <iomanip>
#include <fstream>

BOOST_AUTO_TEST_SUITE(eigenray_test)

using namespace boost::unit_test;
using namespace usml::waveq3d;

/**
 * @ingroup waveq3d_test
 * @{
 */

static const double time_step = 0.100 ;
static const double f0 = 2000 ;
static const double src_lat = 45.0;        // location = mid-Atlantic
static const double src_lng = -45.0;
static const double c0 = 1500.0;           // constant sound speed
static const double bot_depth = 1e5 ;

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
 *   - Direct Path: 1.484018789 sec, -0.01 deg launch, 66.95 dB
 *   - Surface Bounce: 1.995102731 sec, 41.93623171 deg launch, 69.52 dB
 *   - Bottom Bounce: 3.051676949 sec, -60.91257162 deg launch, 73.21 dB
 *
 * With a time step of 100 msec and an angular spacing of 5.0 deg,
 * the interpolated results are expected to match the analytic values
 * within 2 msec and 0.05 deg.  The extrapolated bottom bounce path
 * is only accurate to within 20 msec and 1.0 deg because it is
 * outside of the ensonified ray fan.
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
BOOST_AUTO_TEST_CASE( eigenray_basic ) {
    cout << "=== eigenray_test: eigenray_basic ===" << endl;
    const char* csvname = USML_TEST_DIR "/waveq3d/test/eigenray_basic.csv";
    const char* ncname = USML_TEST_DIR "/waveq3d/test/eigenray_basic.nc";
    const char* ncname_wave = USML_TEST_DIR "/waveq3d/test/eigenray_basic_wave.nc";
    const double src_alt = -1000.0;
    const double trg_lat = 45.02;
    const double time_max = 3.5;
//    const double time_max = 1.7;

    // initialize propagation model

    wposition::compute_earth_radius( src_lat );
    attenuation_model* attn = new attenuation_constant(0.0);
    profile_model* profile = new profile_linear(c0,attn);
    boundary_model* surface = new boundary_flat();
    boundary_model* bottom = new boundary_flat(3000.0);
    ocean_model ocean( surface, bottom, profile );

    seq_log freq( 10e3, 1.0, 1 );
    wposition1 pos( src_lat, src_lng, src_alt );
    seq_linear de( -60.0, 1.0, 60.0 );
    seq_linear az( -4.0, 1.0, 4.0 );

    // build a single target

    wposition target( 1, 1, trg_lat, src_lng, src_alt );

    proploss loss(freq, pos, de, az, time_step, &target);
    wave_queue wave( ocean, freq, pos, de, az, time_step, &target) ;
    wave.addEigenrayListener(&loss);

    // propagate rays and record wavefronts to disk.

    cout << "propagate wavefronts for " << time_max << " seconds" << endl;
    cout << "writing wavefronts to " << ncname_wave << endl;

    wave.init_netcdf( ncname_wave );
    wave.save_netcdf();
    while ( wave.time() < time_max ) {
        wave.step();
        wave.save_netcdf();
    }
    wave.close_netcdf();

   // compute coherent propagation loss and write eigenrays to disk

    loss.sum_eigenrays();
    cout << "writing proploss to " << ncname << endl;
    loss.write_netcdf(ncname,"eigenray_basic test");

    // save results to spreadsheet and compare to analytic results

    cout << "writing tables to " << csvname << endl;
    std::ofstream os(csvname);
    os << "time,intensity,phase,s_de,s_az,t_de,t_az,srf,btm,cst"
    << endl;
    os << std::setprecision(18);
    cout << std::setprecision(18);

    const eigenray_list *raylist = loss.eigenrays(0,0);
    int n=0;
    BOOST_CHECK_EQUAL( raylist->size(), 3 ) ;
    for ( eigenray_list::const_iterator iter = raylist->begin();
            iter != raylist->end(); ++n, ++iter )
    {
        const eigenray &ray = *iter ;
        cout << "ray #" << n
             << " tl=" << ray.intensity(0)
             << " t=" << ray.time
             << " de=" << -ray.target_de
             << " error:" ;
        os << ray.time
           << "," << ray.intensity(0)
           << "," << ray.phase(0)
           << "," << ray.source_de
           << "," << ray.source_az
           << "," << ray.target_de
           << "," << ray.target_az
           << "," << ray.surface
           << "," << ray.bottom
           << "," << ray.caustic
           << endl;
        switch (n) {
            case 0 :
                cout << " tl=" << (ray.intensity(0)-66.9506)
                     << " t=" << (ray.time-1.484018789)
                     << " de=" << max( abs(ray.source_de+0.01),
                                       abs(ray.target_de-0.01) ) << endl ;
                BOOST_CHECK_SMALL( ray.intensity(0)-66.9506, 0.1 );
                BOOST_CHECK_SMALL( ray.time-1.484018789, 0.002 );
                BOOST_CHECK_SMALL( ray.phase(0)-0.0, 1e-6 );
                BOOST_CHECK_SMALL( ray.source_de+0.01, 0.01 );
                BOOST_CHECK_SMALL( ray.target_de-0.01, 0.01 );
                break;
            case 1 :
                cout << " tl=" << (ray.intensity(0)-69.5211)
                     << " t=" << (ray.time-1.995102731)
                     << " de=" << max( abs(ray.source_de-41.93623171),
                                       abs(ray.target_de+41.93623171) ) << endl ;
                BOOST_CHECK_SMALL( ray.intensity(0)-69.5211, 0.1 );
                BOOST_CHECK_SMALL( ray.time-1.995102731, 0.002 );
                BOOST_CHECK_SMALL( ray.phase(0)+M_PI, 1e-6 );
                BOOST_CHECK_SMALL( ray.source_de-41.93623171, 0.01 );
                BOOST_CHECK_SMALL( ray.target_de+41.93623171, 0.01 );
                break;
            case 2 : // note that extrapolation is less accurate
                cout << " tl=" << (ray.intensity(0)-73.2126)
                     << " t=" << (ray.time-3.051676949)
                     << " de=" << max( abs(ray.source_de+60.91257162),
                                       abs(ray.target_de-60.91257162) ) << endl ;
                // BOOST_CHECK_SMALL( ray.intensity(0)-73.2126, 0.05 ) ;
                BOOST_CHECK_SMALL( ray.time-3.051676949, 0.02 );
                BOOST_CHECK_SMALL( ray.phase(0)-0.0, 1e-6 );
                BOOST_CHECK_SMALL( ray.source_de+60.91257162, 1.0 );
                BOOST_CHECK_SMALL( ray.target_de-60.91257162, 1.0 );
                break;
            default :
                break;
        }
        BOOST_CHECK_SMALL( ray.source_az-0.0, 1e-6 );
        BOOST_CHECK_SMALL( ray.target_az-0.0, 1e-6 );
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
 *   - Direct Path: 89.05102557 sec, -0.578554378 deg launch, 0.621445622 deg target
 *   - Surface #1: 89.05369537 sec, 0.337347599 deg launch, 0.406539112 deg target
 *   - Surface #2: 89.05379297 sec, -0.053251329 deg launch, 0.233038477 deg target
 *   - Surface #3: 89.05320459 sec, -0.433973977 deg launch, -0.48969753 deg target
 *
 * When the model is run with these parameters, the travel times are accurate
 * to within 0.02 msec, and the angles are accurate to within 0.02 degrees.
 * But note that, if the spacing between launch angles is too small,
 * Surface 3 occurs between the same two rays as the Direct Path.  There
 * is a fundamental limitation of the model's eigenray searching logic that
 * only allows one ray path to be found between any two launch angles.
 * In this test, a wider launch angle spacing would cause the model to fail
 * to find the Surface 3 path.
 */
BOOST_AUTO_TEST_CASE( eigenray_concave ) {
    cout << "=== eigenray_test: eigenray_concave ===" << endl;
    const char* ncname_wave = USML_TEST_DIR "/waveq3d/test/eigenray_concave_wave.nc";
    const char* ncname = USML_TEST_DIR "/waveq3d/test/eigenray_concave.nc";

    const double src_alt = -200.0;      // source depth = 200 meters
    const double time_max = 120.0 ;     // let rays plots go into region w/ 2 roots
    const double trg_lat = 46.2;        // 1.2 degrees north of source
    const double trg_lng = src_lng ;
    const double trg_alt = -150.0;      // target depth = 150 meters

    // initialize propagation model

    wposition::compute_earth_radius( src_lat );             // init area of ops
    attenuation_model* attn = new attenuation_constant(0.0); // no absorption
    profile_model* profile = new profile_linear(c0,attn);   // iso-velocity
    boundary_model* surface = new boundary_flat();          // default surface
    boundary_model* bottom = new boundary_flat(bot_depth);  // flat bottom
    ocean_model ocean( surface, bottom, profile );

    seq_log freq( f0, 1.0, 1 );
    wposition1 pos( src_lat, src_lng, src_alt );    // build ray source
    seq_linear de( -1.0, 0.05, 1.0 );
    seq_linear az( -4.0, 1.0, 4.0 );

    // build a wavefront to ensonify a single target

    wposition target( 1, 1, trg_lat, trg_lng, trg_alt );

    proploss loss(freq, pos, de, az, time_step, &target);
    wave_queue wave( ocean, freq, pos, de, az, time_step, &target) ;
    wave.addEigenrayListener(&loss);

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

    // compare analytic results

    eigenray_list *raylist = loss.eigenrays(0,0);
    BOOST_CHECK_EQUAL( raylist->size(), 4 ) ;
    for ( eigenray_list::iterator iter=raylist->begin();
          iter != raylist->end(); ++iter )
    {
        eigenray ray = *iter ;
        double theory_t, theory_sde, theory_tde ;

        // direct path

        if ( ray.surface == 0 ) {
            cout << "direct: " ;
            theory_t = 89.05102557 ;
            theory_sde = -0.578554378 ;
            theory_tde = 0.621445622 ;

        // surface reflected paths

        } else {
            if ( ray.source_de > 0.0 ) {        // surface path #1
                cout << "surf1:  " ;
                theory_t = 89.05369537 ;
                theory_sde = 0.337347599 ;
                theory_tde = 0.406539112 ;
            } else {                            // surface path #2
                if ( ray.source_de > -0.1 ) {
                    cout << "surf2:  " ;
                    theory_t = 89.05379297 ;
                    theory_sde = -0.053251329 ;
                    theory_tde = 0.233038477 ;
                } else {                        // surface path #3
                    cout << "surf3:  " ;
                    theory_t = 89.05320459 ;
                    theory_sde = -0.433973977 ;
                    theory_tde = -0.48969753 ;
                }
            }
        }

        cout << "t = "<< ray.time
             << " sde = "<< ray.source_de
             << " tde = "<< ray.target_de
             << " error: t = "<< (ray.time-theory_t)
             << " sde = "<< (ray.source_de-theory_sde)
             << " tde = "<< (ray.target_de-theory_tde) << endl ;

        BOOST_CHECK_SMALL( ray.time - theory_t, 2e-5 );
        BOOST_CHECK_SMALL( ray.source_de - theory_sde, 0.02 );
        BOOST_CHECK_SMALL( ray.target_de - theory_tde, 0.02 );

    }
}

/**
 * Scenario is the exact same as eigenray_basic, except that the
 * number of targets is increased. This allows us to verify the
 * TL accuracy of the hybrid gaussian model.
 *
 * When using the gaussian form of TL calculations, we expect to
 * to see an oscillatory TL plot around the source in azimuthal
 * spread. This oscillation is inherint to the gaussians as most
 * most of the acoustic energy lies in between two rays.
 */
BOOST_AUTO_TEST_CASE( eigenray_tl_az ) {
    cout << "=== eigenray_test: eigenray_tl_az ===" << endl;
    const char* csvname = USML_TEST_DIR "/waveq3d/test/eigenray_tl_az.csv";
    const char* ncname = USML_TEST_DIR "/waveq3d/test/eigenray_tl_az.nc";
    const char* ncname_wave = USML_TEST_DIR "/waveq3d/test/eigenray_tl_az_wave.nc";
    const double src_alt = -1000.0 ;
    const double target_range = 2222.4 ;

    // Situational variables
    const double time_max = 1.8 ;
    const int num_targets = 100 ;
    const double angle_sprd = 16.0 ;
    const double az_start = -8.0 ;
    const double az_inc = 1.0 ;
    const double tar_ang_sprd = 6.0 ;
    const double tar_bearing = 0.0 ;

    // initialize propagation model

    wposition::compute_earth_radius( 0.0 );
    attenuation_model* attn = new attenuation_constant(0.0);
    profile_model* profile = new profile_linear(c0,attn);
    boundary_model* surface = new boundary_flat();
    boundary_model* bottom = new boundary_flat(3000.0);
    ocean_model ocean( surface, bottom, profile );

    seq_log freq( 1000.0, 1.0, 1 );
    wposition1 pos( 0.0, 0.0, src_alt );
    seq_linear de( -60.0, 1.0, 60.0 ) ;
    seq_linear az( az_start, az_inc, az_start+angle_sprd );

    // build a single target

    wposition target( num_targets, 1, 0.0, 0.0, src_alt ) ;
    // build a series of targets at target_range distance away
    double angle = (tar_ang_sprd*M_PI/180.0)/num_targets;
    double bearing_inc = (tar_bearing*M_PI/180.0) ;
    for (unsigned n = 0; n < num_targets; ++n) {
        wposition1 aTarget( pos, target_range, bearing_inc) ;
        target.latitude( n, 0, aTarget.latitude());
        target.longitude( n, 0, aTarget.longitude());
        target.altitude( n, 0, aTarget.altitude());
        bearing_inc = bearing_inc + angle;
    }

    proploss loss(freq, pos, de, az, time_step, &target);
    wave_queue wave( ocean, freq, pos, de, az, time_step, &target) ;
    wave.addEigenrayListener(&loss);

    // propagate rays and record wavefronts to disk.

    cout << "propagate wavefronts for " << time_max << " seconds" << endl;
    cout << "writing wavefronts to " << ncname_wave << endl;

    wave.init_netcdf( ncname_wave );
    wave.save_netcdf();
    while ( wave.time() < time_max ) {
        wave.step();
        wave.save_netcdf();
    }
    wave.close_netcdf();

   // compute coherent propagation loss and write eigenrays to disk

    loss.sum_eigenrays();
    cout << "writing proploss to " << ncname << endl;
    loss.write_netcdf(ncname,"eigenray_tl_az test");

    // save results to spreadsheet and compare to analytic results

    cout << "writing tables to " << csvname << endl;
    std::ofstream os(csvname);
    os << std::setprecision(18);
    cout << std::setprecision(18);

    for ( int i=0; i < num_targets; ++i ) {
        os << (tar_bearing+(tar_ang_sprd*i)/num_targets) << "," ;
        const eigenray_list *raylist = loss.eigenrays(i,0);
        int n=0;
        for ( eigenray_list::const_iterator iter = raylist->begin();
                iter != raylist->end(); ++n, ++iter )
        {
            const eigenray &ray = *iter ;
               os << ray.intensity(0) << "," ;
        }
        os << "," << (20*log10(target_range))
           << "," << (20*log10(2.0*sqrt((target_range*target_range)/4.0 + src_alt*src_alt)))
           << endl ;
    }
}

/**
 * Same as eigenray_tl_az except it tests a grid of
 * targets around the source.
 */

BOOST_AUTO_TEST_CASE( eigenray_tl_grid ) {
    cout << "=== eigenray_test: eigenray_tl_grid ===" << endl;
    const char* csvname = USML_TEST_DIR "/waveq3d/test/eigenray_tl_grid.csv";
    const char* ncname = USML_TEST_DIR "/waveq3d/test/eigenray_tl_grid.nc";
    const char* ncname_wave = USML_TEST_DIR "/waveq3d/test/eigenray_tl_grid_wave.nc";
    const double src_alt = -3000.0 ;
    const double target_range = 2222.4 ;

    // Situational variables
    const double time_max = 2.0 ;
    const int num_targets = 50 ;
    const int num_alts = 50 ;
    const double angle_sprd = 16.0 ;
    const double az_start = -8.0 ;
    const double az_inc = 1.0 ;
    const double tar_ang_sprd = 6.0 ;
    const double tar_bearing = -3.0 ;

    // initialize propagation model

    wposition::compute_earth_radius( 0.0 );
    attenuation_model* attn = new attenuation_constant(0.0);
    profile_model* profile = new profile_linear(c0,attn);
    boundary_model* surface = new boundary_flat();
    boundary_model* bottom = new boundary_flat(10000.0);
    ocean_model ocean( surface, bottom, profile );

    seq_log freq( 1000.0, 1.0, 1 );
    wposition1 pos( 0.0, 0.0, src_alt );
    seq_linear de( -60.0, 1.0, 60.0 ) ;
    seq_linear az( az_start, az_inc, az_start+angle_sprd );

    // build a single target

    wposition target( num_targets, num_alts, 0.0, 0.0, -1000.0 ) ;
    // build a series of targets at target_range distance away
    double angle = (tar_ang_sprd*M_PI/180.0)/num_targets;
    double bearing_inc = (tar_bearing*M_PI/180.0) ;
    for(int n = 0; n < num_targets; ++n) {
        double temp_alt ;
        wposition1 temp_pos( 0.0, 0.0, 0.0 ) ;
        for(unsigned m = 0; m < num_alts; ++m) {
            temp_alt = m * 4000.0 / num_alts - 5000.0 ;
            temp_pos.altitude( temp_alt ) ;
            wposition1 aTarget( temp_pos, target_range, bearing_inc) ;
            target.latitude( n, m, aTarget.latitude());
            target.longitude( n, m, aTarget.longitude());
            target.altitude( n, m, aTarget.altitude());
        }
        bearing_inc = bearing_inc + angle;
    }

    proploss loss(freq, pos, de, az, time_step, &target);
    wave_queue wave( ocean, freq, pos, de, az, time_step, &target) ;
    wave.addEigenrayListener(&loss);

    // propagate rays and record wavefronts to disk.

    cout << "propagate wavefronts for " << time_max << " seconds" << endl;
    cout << "writing wavefronts to " << ncname_wave << endl;

    wave.init_netcdf( ncname_wave );
    wave.save_netcdf();
    while ( wave.time() < time_max ) {
        wave.step();
        wave.save_netcdf();
    }
    wave.close_netcdf();

   // compute coherent propagation loss and write eigenrays to disk

    loss.sum_eigenrays();
    cout << "writing proploss to " << ncname << endl;
    loss.write_netcdf(ncname,"eigenray_tl_az test");

    // save results to spreadsheet and compare to analytic results

    cout << "writing tables to " << csvname << endl;
    std::ofstream os(csvname);
    os << std::setprecision(18);
    cout << std::setprecision(18);

    for ( int i=0; i < num_targets; ++i ) {
        os << (tar_bearing+(tar_ang_sprd*i)/num_targets) << "," ;
        const eigenray_list *raylist = loss.eigenrays(i,0);
        int n=0;
        for ( eigenray_list::const_iterator iter = raylist->begin();
                iter != raylist->end(); ++n, ++iter )
        {
            const eigenray &ray = *iter ;
               os << ray.intensity(0) << "," ;
        }
        os << "," << (20*log10(target_range))
           << "," << (20*log10(2.0*sqrt((target_range*target_range)/4.0 + src_alt*src_alt)))
           << endl ;
    }
}

/**
 * Scenario is the exact same as eigenray_basic, except that the
 * target lies on the azimuthal branch point between 0 and 360, and
 * the depression angle branch point of +90 and -90. Only one
 * eigenray should be produced at these branch point for any given
 * combination of time, bottom, and surface bounces.
 */
BOOST_AUTO_TEST_CASE( eigenray_branch_pt ) {
    cout << "=== eigenray_test: eigenray_branch_pt ===" << endl;
    const char* csvname = USML_TEST_DIR "/waveq3d/test/eigenray_branch_pt.csv";
    const char* ncname = USML_TEST_DIR "/waveq3d/test/eigenray_branch_pt.nc";
    const char* ncname_wave = USML_TEST_DIR "/waveq3d/test/eigenray_branch_pt_wave.nc";
    const double src_alt = -1000.0;
    const double target_range = 2226.0;
    const double time_max = 3.5;
    const int num_targets = 12 ;

    // initialize propagation model

    wposition::compute_earth_radius( 0.0 );
    attenuation_model* attn = new attenuation_constant(0.0);
    profile_model* profile = new profile_linear(c0,attn);
    boundary_model* surface = new boundary_flat();
    boundary_model* bottom = new boundary_flat(3000.0);
    ocean_model ocean( surface, bottom, profile );

    seq_log freq( 1000.0, 1.0, 1 );
    wposition1 pos( 0.0, 0.0, src_alt );
    seq_linear de( -90.0, 1.0, 90.0 );
    seq_linear az( 0.0, 15.0, 360.0 );

    // build a single target

    wposition target( num_targets+2, 1, 0.0, 0.0, src_alt ) ;
    target.altitude( 0, 0, src_alt-500 ) ;
    target.altitude( 1, 0, src_alt+500 ) ;
    // build a series of targets at 100 km
    double angle = TWO_PI/num_targets;
    double bearing_inc = 0 ;
    for (unsigned n = 2; n < num_targets+2; ++n) {
        wposition1 aTarget( pos, target_range, bearing_inc) ;
        target.latitude( n, 0, aTarget.latitude());
        target.longitude( n, 0, aTarget.longitude());
        target.altitude( n, 0, aTarget.altitude());
        bearing_inc = bearing_inc + angle;
    }

    proploss loss(freq, pos, de, az, time_step, &target);
    wave_queue wave( ocean, freq, pos, de, az, time_step, &target) ;
    wave.addEigenrayListener(&loss);

    // propagate rays and record wavefronts to disk.

    cout << "propagate wavefronts for " << time_max << " seconds" << endl;
    cout << "writing wavefronts to " << ncname_wave << endl;

    wave.init_netcdf( ncname_wave );
    wave.save_netcdf();
    while ( wave.time() < time_max ) {
        wave.step();
        wave.save_netcdf();
    }
    wave.close_netcdf();

   // compute coherent propagation loss and write eigenrays to disk

    loss.sum_eigenrays();
    cout << "writing proploss to " << ncname << endl;
    loss.write_netcdf(ncname,"eigenray_az_branch_pt test");

    // save results to spreadsheet and compare to analytic results

    cout << "writing tables to " << csvname << endl;
    std::ofstream os(csvname);
    os << "target,time,intensity,phase,s_de,s_az,t_de,t_az,srf,btm,cst"
    << endl;
    os << std::setprecision(18);
    cout << std::setprecision(18);

    for ( int i=0; i < num_targets; ++i ) {
        os << "#" << i ;
        cout << "===Target #" << i << "===" << endl;
        const eigenray_list *raylist = loss.eigenrays(i,0);
        int n=0;
        for ( eigenray_list::const_iterator iter = raylist->begin();
                iter != raylist->end(); ++n, ++iter )
        {
            const eigenray &ray = *iter ;
            cout << "ray #" << n
                 << " tl=" << ray.intensity(0)
                 << " t=" << ray.time
                 << " de=" << -ray.target_de
                 << endl;
            os << "," << ray.time
               << "," << ray.intensity(0)
               << "," << ray.phase(0)
               << "," << ray.source_de
               << "," << ray.source_az
               << "," << ray.target_de
               << "," << ray.target_az
               << "," << ray.surface
               << "," << ray.bottom
               << "," << ray.caustic
               << endl;
        }
    }
}
/// @}

BOOST_AUTO_TEST_SUITE_END()
