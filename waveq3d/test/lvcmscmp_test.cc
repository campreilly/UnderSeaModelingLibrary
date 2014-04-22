/**
 * @example ocean/test/lvcmscmp_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <usml/ocean/reflect_loss_srflos_grid.h>
#include <usml/waveq3d/waveq3d.h>
#include <iostream>
#include <iomanip>
#include <fstream>

BOOST_AUTO_TEST_SUITE(lvcmscmp_test)

using namespace boost::unit_test;
using namespace usml::waveq3d;

/**
 * @ingroup lvcmscmp_test
 * @{
 */

static const double time_step = 0.05 ;
static const double f0 = 6500 ;
static const double src_lat = 26.0;        // location = Gulf of Oman
static const double src_lng = 57.75;
static const double c0 = 1532.0;           // constant sound speed (m/s)
static const double bot_depth = 100 ;      // (meters)

/**
 * Tests the basic features of the eigenray model for a simple target.
 *
 */
BOOST_AUTO_TEST_CASE( eigenray_basic ) {
    cout << "=== lvcmscmp_test: eigenray_basic ===" << endl;
    const char* csvname = USML_TEST_DIR "/waveq3d/test/lvcms_eigenray_basic.csv";
    const char* ncname = USML_TEST_DIR "/waveq3d/test/lvcms_eigenray_basic.nc";
    const char* ncname_wave = USML_TEST_DIR "/waveq3d/test/lvcms_eigenray_basic_wave.nc";
    //const char* profile_txt = USML_TEST_DIR "/waveq3d/test/GO_Jan_svp_data.txt";
    const double src_alt = -27.0;
    const double time_max = 0.7;

    // initialize propagation model

    wposition::compute_earth_radius( src_lat );
    attenuation_model* attn = new attenuation_constant(0.0); // No Attenuation loss
    profile_model* profile = new profile_linear(c0, attn);
    // profile_model* profile = new profile_linear(c0);

//    data_grid<double, 1>* grid = new ascii_profile(profile_txt);
//    profile_model* profile = new profile_grid<double,1>(grid, attn);

    boundary_model* surface = new boundary_flat();           // No Surface loss
    //surface->reflect_loss(new reflect_loss_srflos(10.29, c0)); // 10.29m/s = 20 knots
    boundary_model* bottom = new boundary_flat(bot_depth);
    //bottom->reflect_loss(new reflect_loss_rayleigh(reflect_loss_rayleigh::SILT));
    bottom->reflect_loss(new reflect_loss_constant(0.0));

    ocean_model ocean( surface, bottom, profile );

    seq_log freq( f0, 1.0, 1 );
    wposition1 pos( src_lat, src_lng, src_alt );
    seq_linear de( -35.0, 1.0, 35.0 );
    seq_linear az( -180.0, 15.0, 180.0 );

    // build a single target at depth of 50 meters due north
    double target_range = 1000.0; // meters
    wposition target( 1, 1, 0.0, 0.0, -50.0 ) ;
    wposition1 aTarget( pos, target_range, 0.0 ) ;
    target.latitude( 0, 0, aTarget.latitude() ) ;
    target.longitude( 0, 0, aTarget.longitude() ) ;

    proploss loss(freq, pos, de, az, time_step, &target);
    wave_queue wave( ocean, freq, pos, de, az, time_step, &target) ;
    wave.addProplossListener(&loss);

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
    os << std::setprecision(10);
    cout << std::setprecision(10);

    const eigenray_list *raylist = loss.eigenrays(0,0);
    int n=0;
    for ( eigenray_list::const_iterator iter = raylist->begin();
            iter != raylist->end(); ++n, ++iter )
    {
        const eigenray &ray = *iter ;
        cout << "ray #" << n
             << " t=" << ray.time
             << " tl=" << ray.intensity(0)
             << " de=" << -ray.target_de << endl;
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
    }
}
/// @}

BOOST_AUTO_TEST_SUITE_END()
