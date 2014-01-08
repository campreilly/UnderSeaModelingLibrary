/**
 * @file pekeris_waveguide.cc
 *
 * Compares the results of the analytic solution of
 * the Pekeris Waveguide to the WaveQ3D equivalent.
 *
 */
#include <usml/waveq3d/waveq3d.h>
#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <stdio.h>
#include <stdlib.h>

using namespace usml::waveq3d ;
using namespace usml::ocean ;

/**
 * The following test is to compare the results produced by
 * WaveQ3D to results obtained from the analytic solution of
 * Normal Mode Pekeris Waveguide from Jensen & Kuperman.
 */
int main() {
    cout << "=== pekeris_waveguide ===" << endl ;

   // define scenario parameters
    int num_targets = 200 ;
    double m_2_deg = 1.0 / ( 1852.0 * 60.0 ) ;            // conversion factor from meters to degree lat
    const double depth = 200.0 ;
    wposition::compute_earth_radius( 0.0 ) ;
    wposition1 pos ;
    pos.altitude(-100.0) ;

    // setup fan parameters
    seq_rayfan de( -90.0, 90.0, 181 ) ;
    seq_linear az( 0.0, 15.0, 360.0 ) ;
    seq_log freq( 1000.0, 1000.0, 1 ) ;
    const double time_max = 5.0 ;
    const double time_step = 0.05 ;

    // setup files to output all data to

    const char* csvname = USML_STUDIES_DIR "/pekeris/pekeris_waveguide_eigenray.csv";
    const char* ncname = USML_STUDIES_DIR "/pekeris/pekeris_waveguide_proploss.nc";
    const char* ncname_wave = USML_STUDIES_DIR "/pekeris/pekeris_waveguide_eigenray_wave.nc";

    // build sound velocity profile

    const double c0 = 1500.0 ;
    attenuation_model* att_mod = new attenuation_constant(0.0);
    profile_model*  profile = new profile_linear(c0, att_mod) ;
//    profile->flat_earth(true);
    boundary_model* surface = new boundary_flat() ;

    reflect_loss_model* bot_loss =
        new reflect_loss_rayleigh( 1.5, 1700.0/c0, 0.5 ) ;
//    reflect_loss_model* bot_loss = new reflect_loss_constant(0.0);
    boundary_model* bottom = new boundary_flat( depth , bot_loss ) ;

    ocean_model ocean( surface, bottom, profile ) ;

    // initialize proploss targets and wavefront

    wposition target( num_targets, 1, pos.latitude(), pos.longitude(), -30.0 ) ;
    double inc = ( 6000.0 * m_2_deg ) / num_targets ;
    for( int n=1; n < target.size1(); ++n ) {
        target.longitude( n, 0, pos.longitude() + (inc * n) ) ;
    }

    proploss loss( freq, pos, de, az, time_step, &target ) ;
    wave_queue wave( ocean, freq, pos, de, az, time_step, &target ) ;
    wave.addProplossListener( &loss ) ;

    // propagate wavefront
    cout << "writing wavefronts to " << ncname_wave << endl;

    wave.init_netcdf( ncname_wave );
    wave.save_netcdf();

    while ( wave.time() < time_max ) {
        wave.step() ;
        wave.save_netcdf();
    }

    wave.close_netcdf();

    //compute coherent propagation loss and write eigenrays to disk

    loss.sum_eigenrays();
    cout << "writing proploss to " << ncname << endl;
    loss.write_netcdf(ncname,"Pekeris Waveguide");

    //save results to spreadsheet and compare to analytic results

    cout << "writing tables to " << csvname << endl;
    std::ofstream os(csvname);
    os << "target,depth,range,intensity" << endl;
    os << std::setprecision(18);
    cout << std::setprecision(18);

    for(int n = 0; n < target.size1() ; ++n)
    {
        os << n << ","
           << target.altitude(n,0) << ","
           << target.latitude(n,0) / m_2_deg << ","
           << -loss.total(n,0)->intensity(0)
           << endl;
    }
}
