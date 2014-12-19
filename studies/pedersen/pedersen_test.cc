/**
 * @file pedersen_test.cc
 *
 * Analyze the performance of the model against the extreme downward refraction
 * N^2 linear test case developed by Pedersen and Gordon.  This is also the
 * test case that Weinberg and Keenan used to calibrate the minimum beam width
 * in GRAB. This test serves several important functions
 *
 *  - Compares the ray path locations, travel time, and D/E angles
 *    to analytic results.
 *  - Compares the total propagation loss to analytic results.
 *  - Analyzes the contribution of the phase change at the caustic.
 *
 * The Pedersen/Gordon paper focuses two test cases.
 *
 * - The shallow source portion analyzed an acoustic field for a downward
 *   refracting profile that had been fit to experimental velocity data.
 *   Propagation loss was studied in both the image interference zone
 *   and the shadow zone.
 * - The deep source test cases extended this analysis to a non-physical
 *   extreme that emphasizes the effects of the caustic.
 *
 * Pedersen/Gordon concludes that the pi/2 phase change should only occur
 * when the ray path touches the caustic and not when the ray passes through
 * a horizontal vertex.  This is consistent with the current implementation
 * of GRAB, but different than the original Weinberg/Keenan paper.
 *
 * Uses the MKS version of the parameters defined in Jensen et. al.
 * instead of the original yd/sec version defined by Pedersen.
 *
 *  - c0 = 1550 m/s, g0 = 1.2 1/s, f = 2000 Hz
 *  - shallow:
 *      - source: depth = 75 m, time inc=0.025, cs = 1467.149925 m/s
 *      - target: depth = 75 m, range = 500-1000 m
 *  - deep:
 *      - source: depth = 1000 m, time inc=0.1, cs = 970.954770 m/s
 *      - target: depth = 800 m, range = 3000-3100 m
 *
 * @xref M. A. Pedersen, D. F. Gordon, "Normal-Mode and Ray Theory Applied
 * to Underwater Acoustic conditions of Extreme Downward Refraction",
 * J. Acoust. Soc. Am. 51 (1B), 323-368 (June 1972).
 * @xref H. Weinberg, R. E. Keenan, “Gaussian ray bundles for modeling
 * high-frequency propagation loss under shallow-water conditions”,
 * J. Acoust. Soc. Am. 100 (3), 1421-1431, (Sept 1996).
 * @xref L. M. Brekhovskikh, Waves in Layered Media, 2nd Edition,
 * Academic Press Inc., 1980, Section 54.
 * @xref F. B. Jensen, W. A. Kuperman, M. B. Porter, and H. Schmidt,
 * Computational Ocean Acoustics (American Institute of Physics Press,
 * New York, 1994) pp. 162-166.
 */
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <usml/waveq3d/waveq3d.h>
#include <iostream>
#include <iomanip>
#include <fstream>

BOOST_AUTO_TEST_SUITE(pedersen_test)

using namespace boost::unit_test;
using namespace usml::waveq3d;

// Define analysis parameter constants.

static const double FREQ = 2000 ;   // Hz
static const double C0 = 1550.0 ;   // sound speed at surface (m/s)
static const double G0 = 1.2 ;      // speed gradient at surface (1/s)
static const double LAT_SOURCE = 45.0;  // source latitude (deg)
static const double LNG_SOURCE = -45.0; // source longitude (deg)

/**
 * Construct the ocean model from its constituant parts.
 *
 * - N^2 Linear Profile on a Flat Earth
 * - No Attenuation
 * - Default Sea Surface
 * - "Infinitely" deep bottom
 *
 * @return  Dynamically allocated ocean model.
 *          Calling routine responsible for cleanup.
 */
ocean_model* build_ocean() {
    wposition::compute_earth_radius(LAT_SOURCE);

    attenuation_model* attn = new attenuation_constant(0.0);
    profile_model* profile = new profile_n2( C0, G0, attn );
    profile->flat_earth(true);

    boundary_model* surface = new boundary_flat();
    boundary_model* bottom = new boundary_flat(30000.0);

    return new ocean_model(surface, bottom, profile);
}

/**
 * Tests the accuracy of the raytrace model against the analytic
 * solutions for N^2 linear test case developed by Pedersen/Gordon.
 * Limits this comparison to the portion of the propagation after
 * all the rays are heading down.
 *
 * @param   source_depth    Source depth (m)
 * @param   de              Source D/E angles (deg)
 * @param   time_threshold  Error tolerance for travel time (%)
 * @param   range_threshold Error tolerance for target range (%)
 * @param   de_threshold    Error tolerance for target D/E angle (%)
 * @param   ncfile          File used to store wavefront history
 * @param   csvfile         File used to store model/theory comparisons
 */
void analyze_raytrace(
    double source_depth, const seq_vector& de,
    double time_min, double time_inc, double time_max,
    double time_threshold, double range_threshold, double de_threshold,
    const char* ncfile, const char* csvfile )
{
    // initialize source information

    ocean_model* ocean = build_ocean() ;
    seq_log freq( FREQ, 1.0, 1 ) ;
    wposition1 pos( LAT_SOURCE, LNG_SOURCE, source_depth );
    seq_linear az(0.0,1.0,1);       // only 1 ray needed for ray trace

    // compute the ray trace and store wavefronts to disk

    wave_queue wave( *ocean, freq, pos, de, az, time_inc );
    cout << "writing wavefronts to " << ncfile << endl;
    wave.init_netcdf(ncfile); // open a log file for wavefront data
    wave.save_netcdf();     // write initial ray data to log file

    cout << "writing spreadsheets to " << csvfile << endl;
    std::ofstream os(csvfile);
    os << "DElaunch,Tmodel,Ttheory,Rmodel,Rtheory,DEmodel,DEtheory" << endl;
    os << std::setprecision(18);
    const double Cs = wave.curr()->sound_speed(0,0) ; // sound speed at source
    const double b = G0 * 2 / (C0*C0*C0) ;

    double time_max_error = 0.0 ;
    double range_max_error = 0.0 ;
    double de_max_error = 0.0 ;

    while ( wave.time() < time_max ) {
        wave.step();        // propagate by one time step
        wave.save_netcdf(); // write ray data to log file

        // compare to the analytic results once all the rays are heading down

        if ( wave.time() >= time_min  ) {
            for ( size_t d=0 ; d < wave.num_de() ; ++d ) {

                // find Cm = speed at which each ray becomes horizonal

                const double As = -to_radians(wave.source_de(d)); // angle at source
                const double Cm = Cs / cos(As) ;    // Pedersen's ray parameter

                // find A0 = the angle at which this ray will hit the surface
                // Pedersen eq. 9: cos(A0) = C0/Cm

                const int surface = wave.curr()->surface(d,0) ;
                const double A0 = (surface==0) ? NAN : acos(min(1.0,C0/Cm)) ;

                // find Ah = the angle at the target location
                // Pedersen eq. 9: cos(Ah) = Ch/Cm

                const double Ch = wave.curr()->sound_speed(d,0) ;
                const double Ah = acos(Ch/Cm) ;

                wvector1 ndir( wave.curr()->ndirection, d, 0 ) ;
                double DEmodel, AZmodel ;
                ndir.direction( &DEmodel, &AZmodel ) ;

                // find R = horizontal range to the ray at the target location
                // Pedersen eq. 11: R=2/(b Cm^2)[tan(Ah)-tan(As)-2 tan(A0)]

                const double tanAh = tan(Ah);
                const double tanAs = tan(As);
                const double tanA0 = (surface==0) ? 0.0 : tan(A0);

                const double R = (2/(b*Cm*Cm)) * ( tanAh - tanAs - 2*tanA0 ) ;

                const double Rmodel = wposition::earth_radius * to_radians(
                    wave.curr()->position.latitude(d,0) - LAT_SOURCE ) ;

                // find T = travel time to the ray at the target location
                // Pedersen eq. 12:
                // R = 2/(3b Cm^3)[tan^3(Ah)-tan^3(As)-2tan^3(A0)] + R/Cm

                const double T = (2/(3*b*Cm*Cm*Cm))
                    * ( tanAh*tanAh*tanAh - tanAs*tanAs*tanAs
                      - 2*tanA0*tanA0*tanA0 ) + R / Cm ;

                // write the results to a spreadsheet

                os << -to_degrees(As)
                   << "," << wave.time() << "," << T
                   << "," << Rmodel << "," << R
                   << "," << DEmodel << "," << -to_degrees(Ah)
                   << endl ;

                const double time_error = wave.time()-T ;
                const double range_error = Rmodel-R ;
                const double de_error = DEmodel+to_degrees(Ah) ;
                time_max_error = max( time_max_error, abs(time_error) ) ;
                range_max_error = max( range_max_error, abs(range_error) ) ;
                de_max_error = max( de_max_error, abs(de_error) ) ;
                BOOST_CHECK_SMALL( time_error, time_threshold ) ;
                BOOST_CHECK_SMALL( range_error, range_threshold ) ;
                BOOST_CHECK_SMALL( de_error, de_threshold ) ;
            }
        }
    }

    // clean up and exit

    wave.close_netcdf(); // close log file for wavefront data
    delete ocean ;
    cout << "max errors: time=" << time_max_error
         << " range=" << range_max_error
         << " de=" << de_max_error << endl ;
}

/**
 * Tests the accuracy of the raytrace model against the analytic
 * solution for N^2 linear, shallow source, test case used by Pedersen/Gordon.
 * Generates linearly spaced beams in a 0:0.5:25 degree fan
 * to match Figure 3.6 in Jensen et. al.  The critical ray for a source
 * at 75.0 meters is 18.82 degrees.
 *
 * Errors are automatically generated if the ray paths deviate from the
 * analytic results by more than 3 millisecs in time, 5.0 meters in range,
 * or 0.2 degrees in angle.
 */
BOOST_AUTO_TEST_CASE( pedersen_shallow_raytrace ) {
    cout << "=== pedersen_shallow_raytrace ===" << endl ;
    seq_linear de( 0.0, 0.02, 25.00 ) ;
    analyze_raytrace( -75.0, de,
        0.30, 0.01, 0.85,
        0.003, 5.0, 0.2,
        USML_STUDIES_DIR "/pedersen/pedersen_shallow_raytrace.nc",
        USML_STUDIES_DIR "/pedersen/pedersen_shallow_raytrace.csv" ) ;
}

/**
 * Tests the accuracy of the raytrace model against the analytic
 * solution for N^2 linear test case used by Pedersen/Gordon.
 * Generates linearly spaced beams in a 20:1:60 degrees fan
 * to match Figure 3.8 in Jensen et. al.  The critical ray for a source
 * at 1000.0 meters is 51.21 degrees.
 *
 * Errors are automatically generated if the ray paths deviate from the
 * analytic results by more than 8 millisecs in time, 10 meters in range,
 * or 0.02 degrees in angle.
 */
BOOST_AUTO_TEST_CASE( pedersen_deep_raytrace ) {
    cout << "=== pedersen_deep_raytrace ===" << endl ;
    seq_linear de( 20.00, 0.2, 60.00 ) ;
    analyze_raytrace( -1000.0, de,
        2.0, 0.02, 3.5,
        0.008, 10.0, 0.03,
        USML_STUDIES_DIR "/pedersen/pedersen_deep_raytrace.nc",
        USML_STUDIES_DIR "/pedersen/pedersen_deep_raytrace.csv" ) ;
}

/**
 * Tests the accuracy of the proploss model against the analytic
 * solution for N^2 linear test case developed by Pedersen/Gordon.
 *
 * @param   time_step       Size of the time step to used in model (sec)
 * @param   de              Source D/E angles (deg)
 * @param   frequency       Source frequency (Hz)
 * @param   source_depth    Source depth (m)
 * @param   target_depth    Target depth (m)
 * @param   target_range    List of target ranges (m)
 * @param   ncfile          File used to store eigenrays
 * @param   csvfile         File used to store model/theory comparisons
 */
void analyze_proploss(
    const seq_vector& de, double source_depth, double target_depth,
    const seq_vector& target_range, double time_inc, double time_max,

    const char* ncfile, const char* csvfile )
{
    // initialize source information

    ocean_model* ocean = build_ocean() ;
    seq_log freq( FREQ, 1.0, 1 ) ;
    wposition1 pos( LAT_SOURCE, LNG_SOURCE, source_depth );
    seq_linear az(-4.0,1.0,4.0);

    // build a series of targets at different ranges

    wposition target(target_range.size(),1,LAT_SOURCE,LNG_SOURCE,target_depth);
    for (size_t n = 0; n < target.size1(); ++n) {
        const double angle = target_range[n] / wposition::earth_radius ;
        target.latitude( n, 0, LAT_SOURCE + to_degrees(angle) );
    }

    proploss loss(freq, pos, de, az, time_inc, &target);

    wave_queue wave( *ocean, freq, pos, de, az, time_inc, &target ) ;

    if (!wave.addEigenrayListener(&loss)) {
    	cout << "Error adding proploss listener! " << endl ;
    	exit(1);
    }

    // compute the proploss and store eigenrays to disk

    cout << "propagate wavefronts" << endl;
    while ( wave.time() < time_max ) {
        wave.step();        // propagate by one time step
    }
    loss.sum_eigenrays();
    cout << "writing eigenrays to " << ncfile << endl;
    loss.write_netcdf(ncfile);

    // save results to spreadsheet for post-test analysis

    cout << "writing spreadsheets to " << csvfile << endl;
    std::ofstream os(csvfile);
    os << "range,model,first,second" << endl;
    os << std::setprecision(18);

    vector<double> tl_model(target.size1());
    for (size_t n = 0; n < target.size1(); ++n) {
        double tl_model = -loss.total(n,0)->intensity(0);
        os << target_range[n] << "," << tl_model ;

        const eigenray_list* raylist = loss.eigenrays(n, 0);
        for (eigenray_list::const_iterator iter = raylist->begin();
                iter != raylist->end(); ++iter) {
            const eigenray &ray = *iter;
            os << "," << (-ray.intensity(0));
        }
        os << endl;
    }

    // clean up and exit

    delete ocean ;
}

/**
 * Tests the accuracy of the proploss model against the analytic solution for
 * the shallow source, N^2 linear test case developed by Pedersen and Gordon.
 * The source is located at a depth of 75 meters. Receivers have a depth of
 * 75 meters and ranges from 500 to 1000 yds. Uses rays from 0 to 30 degrees at
 * a very a tight 0.025 deg spacing.  Such tight spacing is needed to capture
 * the portion of the surface reflected path just inside of the critical ray.
 *
 * This configuration tests the model's ability to produce accurate propagation
 * loss, phase, travel times, and eigneray angles at the edge of a shadow zone.
 * Eigenrays are written out in both netCDF and CSV format so that they can be
 * compared to other models off-line.
 */
BOOST_AUTO_TEST_CASE( pedersen_shallow_proploss ) {
    cout << "=== pedersen_shallow_proploss ===" << endl ;
    seq_linear ranges(500.0,1.0,1000.0) ;
    seq_linear de( 0.0, 0.02, 25.0 ) ;
    analyze_proploss( de, -75.0, -75.0, ranges,
        0.01, 0.85,
        USML_STUDIES_DIR "/pedersen/pedersen_shallow_proploss.nc",
        USML_STUDIES_DIR "/pedersen/pedersen_shallow_proploss.csv" ) ;
}

/**
 * Tests the accuracy of the proploss model against the analytic solution for
 * the deep source, N^2 linear test case developed by Pedersen and Gordon.
 * The source is located at a depth of 1000 meters. Receivers have a depth of
 * 800 meters and ranges from 3000 to 3120 meters. Uses rays from 20
 * to 60 degrees at a 0.25 deg spacing.
 *
 * We found that tangent spaced beams did not work very well for this
 * scenario.  Several combinations of parameters lead to artifacts in the
 * shadow zone from an imperfect cancellation of the direct and caustic paths.
 * We believe that this is caused the accumulation of small cell width
 * and target range errors over hundreds of contributing beams.
 *
 * This configuration tests the model's ability to produce accurate propagation
 * loss, phase, travel times, and eigneray angles in an area where a caustic
 * causes rays to cross at the edge of a shadow zone.  Eigenrays are written
 * out in both netCDF and CSV format so that they can be  compared to other
 * models off-line.
 */
BOOST_AUTO_TEST_CASE( pedersen_deep_proploss ) {
    cout << "=== pedersen_deep_proploss ===" << endl ;
    seq_linear ranges(3000.0,0.25,3120.0) ;
    seq_linear de( 20.0, 0.2, 60.0 ) ;
    analyze_proploss( de, -1000.0, -800.0, ranges,
        0.01, 3.5,
        USML_STUDIES_DIR "/pedersen/pedersen_deep_proploss.nc",
        USML_STUDIES_DIR "/pedersen/pedersen_deep_proploss.csv" ) ;
}

///**
// * Tests the sensitivity of the proploss model the D/E angular resolution
// * near the caustic. The source is located at a depth of 1000 yds. Receivers
// * have a depth of 800 yds and ranges from 3100 to 3180 yds. Uses a ray fan
// * from +40 to +51 degrees with increments of 0.025, 0.05, 0.10, and 0.20
// * degrees. This configuration test the models sensitivity to ray spacing
// * near the caustic.
// *
// * The N^2 linear test case developed by Pedersen and Gordon was specifically
// * chosen because we expect it to be very sensitive to the ray geometry near
// * the caustic.  This sensitivity is especially true for the deep source
// * varient, because the profile below 200 meters is more extreme than those
// * found in the real world.
// */
//BOOST_AUTO_TEST_CASE( pedersen_deep_sensitivity ) {
//    cout << "=== pedersen_deep_sensitivity ===" << endl ;
//    seq_linear ranges(3000.0,0.25,3120.0) ;
//
//    seq_rayfan de( -90.0, 90.0, 181, 51.21 ) ;
//    analyze_proploss( de, -1000, -800.0, ranges, 0.01, 3.5,
//        "pedersen_deep_sensitivity_tan.nc",
//        "pedersen_deep_sensitivity_tan.csv" ) ;
//
//    seq_linear de1000( 40.00, 0.100, 60.00 ) ;
//    analyze_proploss( de1000, -1000, -800.0, ranges, 0.01, 3.5,
//        "pedersen_deep_sensitivity_1000.nc",
//        "pedersen_deep_sensitivity_1000.csv" ) ;
//
//    seq_linear de0500( 40.00, 0.05000, 60.00 ) ;
//    analyze_proploss( de0500, -1000, -800.0, ranges, 0.01, 3.5,
//        "pedersen_deep_sensitivity_0500.nc",
//        "pedersen_deep_sensitivity_0500.csv" ) ;
//
//    seq_linear de0250( 40.00, 0.0250, 60.00 ) ;
//    analyze_proploss( de0250, -1000, -800.0, ranges, 0.01, 3.5,
//        "pedersen_deep_sensitivity_0250.nc",
//        "pedersen_deep_sensitivity_0250.csv" ) ;
//
//    seq_linear de0125( 40.00, 0.0125, 60.00 ) ;
//    analyze_proploss( de0125, -1000, -800.0, ranges, 0.01, 3.5,
//        "pedersen_deep_sensitivity_0125.nc",
//        "pedersen_deep_sensitivity_0125.csv" ) ;
//}

/// @}

BOOST_AUTO_TEST_SUITE_END()
