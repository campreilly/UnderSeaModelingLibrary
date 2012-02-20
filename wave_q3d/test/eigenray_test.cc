/**
 * @example wave_q3d/test/eigenray_test.cc
 */
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <usml/wave_q3d/wave_q3d.h>
#include <iostream>
#include <iomanip>
#include <fstream>

BOOST_AUTO_TEST_SUITE(eigenray_test)

using namespace boost::unit_test;
using namespace usml::wave_q3d;

/**
 * @ingroup wave_q3d_test
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
    const char* csvname = "eigenray_basic.csv";
    const char* ncname = "eigenray_basic.nc";
    const char* ncname_wave = "eigenray_basic_wave.nc";
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
    proploss loss( &target );
    wave_queue wave( ocean, freq, pos, de, az, time_step, &loss );

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
 * Bute note that, if the spacing between launch angles is too small,
 * Surface 3 occurs between the same two rays as the Direct Path.  There
 * is a fundamental limitation of the model's eigenray searching logic that
 * only allows one ray path to be found between any two launch angles.
 * In this test, a wider launch angle spacing would cause the model to fail
 * to find the Surface 3 path.
 */
BOOST_AUTO_TEST_CASE( eigenray_concave ) {
    cout << "=== eigenray_test: eigenray_concave ===" << endl;
    const char* ncname_wave = "eigenray_concave_wave.nc";
    const char* ncname = "eigenray_concave.nc";

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
    proploss loss( &target );
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
 * This test computes travel times and eigenray angles for for a combination
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
 *      \mu_{source} = arccos \left( \frac{L^2+D_1^2+D_2^2}{2 L D_1} \right) - 90
 * \f]\f[
 *      \mu_{target} = arccos \left( \frac{L^2+D_2^2+D_1^2}{2 L D_2} \right) - 90
 * \f]\f[
 *      \tau_{direct} = L / c_0
 * \f]
 * where:
 *  - \f$ L \f$ = length of direct-path (meters)
 *  - \f$ D_1 = R - d_1 \f$ = distance from earth center to source (meters)
 *  - \f$ D_2 = R - d_2 \f$ = distance from earth center to target (meters)
 *  - \f$ \mu_{source} \f$ = direct-path D/E angle at source (degrees)
 *  - \f$ \mu_{target} \f$ = direct-path D/E angle at target (degrees)
 *  - \f$ \tau_{direct} \f$ = direct-path travel time from source to target (sec)
 *
 * The surface-reflected path is very complicated in spherical coordinates.
 * One way to find it is to search for the roots to the transcendental equation:
 * \f[
 *      f( \xi_1 ) = D_1 sin( \xi_1 ) - D_2 sin( \xi - \xi_1 ) + \frac{D_1 D_2}{R} sin( \xi - 2 \xi_1 ) = 0
 * \f]
 * where
 *  - \f$ \xi_1 \f$ = latitude change from source to point of reflection
 *  - \f$ \xi_2 = \xi -\xi_1 \f$ = latitude change from reflection point to target
 *
 * This test uses the Newton-Raphson method to iterate over successive values of
 * \f$ \xi_1 \f$ until a solution \f$ f( \xi_1 ) \approx 0 \f$ is found.
 * \f[
 *      f'( \xi_1 ) = D_1 cos( \xi_1 ) + D_2 cos( \xi - \xi_1 ) - 2 \frac{D_1 D_2}{R} cos( \xi - 2 \xi_1 )
 * \f]\f[
 *      \xi_{1 new} = \xi_1 - \frac{ f( \xi_1 ) }{ f'( \xi_1 ) }
 * \f]
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
 *      \eta_{source} = arccos \left( \frac{a_1^2+D_1^2-R^2}{2 a_1 D_1} \right) - 90
 * \f]\f[
 *      \eta_{target} = arccos \left( \frac{a_2^2+D_2^2-R^2}{2 a_2 D_2} \right) - 90
 * \f]\f[
 *      \tau_{surface} = ( a_1 + a_2 ) / c_0
 * \f]
 * where:
 *  - \f$ a_1 \f$ = distance from source to point of reflection (meters)
 *  - \f$ a_2 \f$ = distance from point of reflection to target  (meters)
 *  - \f$ \eta_{source} \f$ = surface-reflected D/E angle at source (degrees)
 *  - \f$ \eta_{target} \f$ = surface-reflected D/E angle at target (degrees)
 *  - \f$ \tau_{surface} \f$ = surface-reflected travel time from source to target (sec)
 *
 * Errors are automatically generated if the modeled eigenrays
 * deviate from the analytic results by more than 0.5 millisecs in time or
 * 0.2 degrees in angle.
 *
 * @xref Weisstein, Eric W. "Newton's Method." From MathWorld--A Wolfram
 *       Web Resource. http://mathworld.wolfram.com/NewtonsMethod.html
 */
BOOST_AUTO_TEST_CASE( eigenray_lloyds ) {
    cout << "=== eigenray_test: eigenray_lloyds ===" << endl;
    const char* ncname_wave = "eigenray_lloyds_wave.nc";
    const char* ncname = "eigenray_lloyds.nc";
    const char* analytic_name = "eigenray_lloyds_analytic.nc";

    const double src_alt = -200.0;      // source depth = 200 meters
    const double time_max = 120.0 ;     // let rays plots go into region w/ 2 roots

    const double rmax = 45.0/60.0 ;     // limit targets to area where N/R converges
    const double rmin = 1.0/60.0 ;      // 1 nmi min range
    const double rinc = 1.0/60.0 ;      // 1 nmi range inc
    const seq_linear range( rmin, rinc, rmax );   // range in latitude

    static double depth[] = { 0, 10, 100, 1000 } ;
    unsigned num_depths = sizeof(depth) / sizeof(double) ;

    //*********************************************************************
    // compute eigenrays for this ocean

    wposition::compute_earth_radius( src_lat );			// init area of ops
    attenuation_model* attn = new attenuation_constant(0.0);    // no absorption
    profile_model* profile = new profile_linear(c0,attn);	// iso-velocity
    boundary_model* surface = new boundary_flat();		// default surface
    boundary_model* bottom = new boundary_flat(bot_depth);	// flat bottom
    ocean_model ocean( surface, bottom, profile );

    seq_log freq( f0, 1.0, 1 );
    wposition1 pos( src_lat, src_lng, src_alt );		// build ray source
    seq_rayfan de ;
    seq_linear az( -4.0, 1.0, 4.0 );

    // build a grid of targets at different ranges and depths

    wposition target( range.size(), num_depths, src_lat, src_lng, src_alt );
    for ( unsigned t1=0; t1 < range.size(); ++t1 ) {
        for ( unsigned t2=0; t2 < num_depths; ++t2 ) {
            target.latitude( t1, t2, src_lat + range(t1) );
            target.altitude( t1, t2, -depth[t2] );
        }
    }
    proploss loss( &target );
    wave_queue wave( ocean, freq, pos, de, az, time_step, &loss );

    // propagate rays & record to log files

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

    //*********************************************************************
    // compare each target location to analytic results

    cout << "testing eigenrays" << endl;
    for ( unsigned t1=0; t1 < range.size(); ++t1 ) {
        for ( unsigned t2=0; t2 < num_depths; ++t2 ) {
            double time, sde, tde, phase ;

            // setup analytic equations for this target

            const double R = wposition::earth_radius;
            const double xi = to_radians( target.latitude(t1,t2) - src_lat ) ;
            const double d1 = - src_alt;
            const double d2 = - target.altitude(t1,t2) ;
            const double D1 = R - d1 ;
            const double D2 = R - d2 ;

            eigenray_list *raylist = loss.eigenrays(t1,t2);
            for ( eigenray_list::iterator iter=raylist->begin();
                  iter != raylist->end(); ++iter )
            {
                eigenray ray = *iter ;

                //*************************************************************
                // compare direct-path model to analytic results

                if ( ray.surface == 0 || depth[t2] < 1e-3 ) {

                    // compute analytic results

                    const double L = sqrt( D1*D1 + D2*D2 - 2.0*D1*D2*cos(xi) );
                    time = L/c0 ;
                    sde = to_degrees( -asin( (L*L+D1*D1-D2*D2) / (2.0*L*D1) ) ) ;
                    tde = to_degrees( asin( (L*L+D2*D2-D1*D1) / (2.0*L*D2) ) ) ;
                    phase = 0.0 ;
                    if ( ray.surface == 1 ) {
                        tde *= -1.0 ;
                        phase = -M_PI ;
                    }

                //*************************************************************
                // compare surface-reflected model to analytic results

                } else {

                    // find reflection point using root of transindental equation
                    // warning: xi2 = 0 for depths < 1e-3, and this makes solution unstable

                    double xi1 = xi ;
                    double xi2 = xi - xi1 ;
                    if ( abs(d2) > 0.5 ) {
                        xi1 = xi / 2.0 ;
                        xi2 = xi - xi1 ;
                        double f,g,delta ;
                        do {
                            f = D1*sin(xi1) - D2*sin(xi2) + D1*D2/R*sin(xi2-xi1) ;
                            g = D1*cos(xi1) + D2*cos(xi2) - 2.0*D1*D2/R*cos(xi2-xi1) ;
                            delta = - f / g ;
                            xi1 += delta ;
                            xi2 = xi - xi1 ;
                        } while ( abs(delta) > 1e-6 ) ;
                    }

                    // compute analytic results

                    const double a1 = sqrt( R*R + D1*D1 - 2.0*R*D1*cos(xi1) );
                    const double a2 = sqrt( R*R + D2*D2 - 2.0*R*D2*cos(xi2) );
                    time = (a1+a2)/c0 ;
                    sde = to_degrees( -asin( (a1*a1+D1*D1-R*R) / (2.0*a1*D1) ) ) ;
                    tde = to_degrees( asin( (a2*a2+D2*D2-R*R) / (2.0*a2*D2) ) ) ;
                    phase = -M_PI ;
                }

                //*************************************************************
                // test the accuracy of the model
                // acknowledge that there will be bigger errors at short range

                if ( range(t1) >= 0.1 ) {
                    BOOST_CHECK_SMALL( ray.time - time, 0.0005 );
                    BOOST_CHECK_SMALL( ray.phase(0)-phase, 1e-6 );
                    BOOST_CHECK_SMALL( ray.source_de - sde, 0.2 );
                    BOOST_CHECK_SMALL( ray.source_az, 1e-6 );
                    BOOST_CHECK_SMALL( ray.target_de - tde, 0.2 );
                    BOOST_CHECK_SMALL( ray.target_az, 1e-6 );
                }

                //*************************************************************
                // replace modeled values with analytic results

                (*iter).time = time ;
                (*iter).source_de = sde ;
                (*iter).source_az = 0.0 ;
                (*iter).target_de = tde ;
                (*iter).target_az = 0.0 ;

//                cout << "lat=" << range[t1] << " depth=" << depth[t2] << " path=" << ray.surface ;
//                cout << " time=" << time << " sd=" << sde << " tde=" << tde << endl ;
            }   // loop through eigenrays for each target
        }   // loop through target depths
    }   // loop through target ranges

    loss.write_netcdf(analytic_name);

}

/// @}

BOOST_AUTO_TEST_SUITE_END()
