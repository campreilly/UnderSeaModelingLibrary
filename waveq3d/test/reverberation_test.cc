/**
 * @example waveq3d/test/reverberation_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <usml/waveq3d/waveq3d_reverb.h>
#include <usml/utilities/SharedPointerManager.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdio>
#include <sys/time.h>

BOOST_AUTO_TEST_SUITE(reverberation_test)

using namespace boost::unit_test ;
using namespace usml::waveq3d ;
using namespace usml::utilities ;

/**
 * @ingroup waveq3d_test
 * @{
 */

/**
 * Produce a simple scenario where waveq3d eigenverb_monostatic can
 * produce a reverberation curve that can then be compared to the
 * classic results.
 */
BOOST_AUTO_TEST_CASE( monostatic ) {
    cout << "=== reverberation_test: monostatic ===" << endl;
    typedef SharedPointerManager<reverberation_model>  Manager ;
    const char* csvname = USML_TEST_DIR "/waveq3d/test/monostatic.csv" ;
    const char* nc_wave = USML_TEST_DIR "/waveq3d/test/monostatic_wave.nc" ;
    double time_max = 7.5 ;
    double time_step = 0.1 ;
    double resolution = 0.1 ;
    double T0 = 0.25 ;                 // Pulse length
    const double f0 = 1000.0 ;
    const double lat = 0.0 ;
    const double lng = 0.0 ;
    const double alt = 0.0 ;
    const double c0 = 1500.0 ;              // constant sound speed
    const double depth = 200.0 ;
    unsigned bins = time_max / resolution ;
    const double SL = 200.0 ;

    // initialize propagation model

    attenuation_model* attn = new attenuation_constant( 0.0 ) ;
    profile_model* profile = new profile_linear( c0, attn ) ;

    boundary_model* surface = new boundary_flat() ;
    surface->setScattering_Model( new scattering_lambert() ) ;

    double btm_spd = 1.10 ;
    double btm_dsty = 1.9 ;
    double btm_atn = 0.8 ;
    reflect_loss_model* btm_rflt =
        new reflect_loss_rayleigh( btm_dsty, btm_spd, btm_atn ) ;
    boundary_model* bottom = new boundary_flat( depth, btm_rflt ) ;
    bottom->setScattering_Model( new scattering_lambert() ) ;

    // create a simple volume layer
//    boundary_model* v1 = new boundary_flat( 100.0 ) ;
//    v1->setScattering_Model( new scattering_lambert() ) ;
//    vector<boundary_model*> v(1) ;
//    v[0] = v1 ;
//    volume_layer* volume = new volume_layer( v ) ;
//
//    ocean_model ocean( surface, bottom, profile, volume ) ;
    ocean_model ocean( surface, bottom, profile ) ;

    seq_log freq( f0, 1.0, 1 );
    wposition1 pos( lat, lng, alt ) ;
//    seq_rayfan de( -90.0, 0.0, 41, -15.0 ) ;
    seq_linear de( -90.0, -1.0, 45, true ) ;
    seq_linear az( 0.0, 45.0, 360.0 ) ;

    wave_queue_reverb wave( ocean, freq, pos, de, az, time_step ) ;
    wave.setID( SOURCE_ID ) ;

        // Set the monostatic cache up
    Manager monostatic( new eigenverb_monostatic( ocean, wave, T0, bins, time_max ) ) ;
    wave.setReverberation_Model( monostatic ) ;

    cout << "Saving wavefront to " << nc_wave << endl ;
    wave.init_netcdf(nc_wave) ;
    wave.save_netcdf() ;
    // propagate rays and record wavefronts to disk.

    cout << "propagate wavefront for " << time_max << " seconds" << endl ;
    while ( wave.time() < time_max ) {
        wave.step() ;
        wave.save_netcdf() ;
    }
    wave.close_netcdf() ;

   // compute coherent propagation loss and write eigenrays to disk
    eigenverb_monostatic* verb_model = (eigenverb_monostatic*)monostatic.getPointer() ;
    const char* eigenverb_file = "eigenverb_data.txt" ;
    cout << "writing eigenverb data to " << eigenverb_file << endl ;
    verb_model->save_eigenverbs(eigenverb_file) ;

    reverberation_model* reverb = monostatic.getPointer() ;
    cout << "computing reverberation levels" << endl ;
    struct timeval time ;
    struct timezone zone ;
    gettimeofday( &time, &zone ) ;
    double start = time.tv_sec + time.tv_usec * 1e-6 ;
    reverb->compute_reverberation() ;
    gettimeofday( &time, &zone ) ;
    double complete = time.tv_sec + time.tv_usec * 1e-6 ;
    cout << "Computation of reverberation curve took "
         << (complete-start) << " sec."
         << endl ;

    cout << "writing reverberation curve to " << csvname << endl;
    std::ofstream os(csvname);
    os << "time,intensity" << endl ;
    os << std::setprecision(18);
    cout << std::setprecision(18);

    const vector<double> reverb_tl = reverb->getReverberation_curve() ;
    vector<double> r = SL + 10.0*log10(reverb_tl) ;
    for ( unsigned i=0; i < bins; ++i ) {
        if( i % 10 == 0 ) {
            cout << "reverb_level(" << i << "): " << r(i) << endl ;
        }
        os << ( i * time_max / bins )
           << "," << r(i)
           << endl ;
    }
}

/**
 * Produce a simple scenario where waveq3d eigenverb_bistatic can
 * produce a reverberation curve that can then be compared to the
 * classic results.
 */
BOOST_AUTO_TEST_CASE( bistatic ) {
    cout << "=== reverberation_test: bistatic ===" << endl;
    typedef SharedPointerManager<reverberation_model>  Manager ;
    const char* csvname = USML_TEST_DIR "/waveq3d/test/bistatic.csv" ;
    const char* nc_source = USML_TEST_DIR "/waveq3d/test/bistatic_wave_source.nc" ;
    const char* nc_receiver = USML_TEST_DIR "/waveq3d/test/bistatic_wave_receiver.nc" ;
    const char* ssp_file = USML_TEST_DIR "/waveq3d/test/bistatic_sound_speed.txt" ;
    double time_max = 10.0 ;
    double time_step = 0.1 ;
    double resolution = 0.1 ;
    double T0 = 1.0 ;                 // Pulse length
    const double f0 = 13500.0 ;
    const double src_lat = 0.0 ;
    const double src_lng = 0.0 ;
    const double src_alt = -8.0 ;
    const double rcvr_lat = 0.018 ;             // 2 km north of the source
    const double rcvr_lng = 0.0 ;
    const double rcvr_alt = -30.0 ;
    const double depth = 1000.0 ;
    unsigned bins = time_max / resolution ;
    const double SL = 250.0 ;

    // initialize propagation model

    attenuation_model* attn = new attenuation_constant( 0.0 ) ;
    profile_model* profile = new profile_grid<double,1>( new ascii_profile( ssp_file ), attn ) ;

    boundary_model* surface = new boundary_flat() ;
    surface->setScattering_Model( new scattering_lambert() ) ;

    double btm_spd = 0.9860893 ;
    double btm_dsty = 1.1480675 ;
    double btm_atn = 0.0192162 ;
    reflect_loss_model* btm_rflt =
        new reflect_loss_rayleigh( btm_dsty, btm_spd, btm_atn ) ;
    boundary_model* bottom = new boundary_flat( depth, btm_rflt ) ;
    bottom->setScattering_Model( new scattering_lambert() ) ;

    // create a simple volume layer
//    boundary_model* v1 = new boundary_flat( 100.0 ) ;
//    v1->setScattering_Model( new scattering_lambert() ) ;
//    vector<boundary_model*> v(1) ;
//    v[0] = v1 ;
//    volume_layer* volume = new volume_layer( v ) ;
//
//    ocean_model ocean( surface, bottom, profile, volume ) ;
    ocean_model ocean( surface, bottom, profile ) ;

    seq_log freq( f0, 1.0, 1 );
    wposition1 source( src_lat, src_lng, src_alt ) ;
    wposition1 receiver( rcvr_lat, rcvr_lng, rcvr_alt ) ;
//    seq_rayfan de( -90.0, 0.0, 41, -15.0 ) ;
    seq_linear de( -45.0, 1.0, 45.0 ) ;
    seq_linear az( 0.0, 45.0, 360.0 ) ;

    wave_queue_reverb wave_source( ocean, freq, source, de, az, time_step ) ;
    wave_queue_reverb wave_receiver( ocean, freq, receiver, de, az, time_step ) ;
    wave_source.setID( SOURCE_ID ) ;
    wave_receiver.setID( RECEIVER_ID ) ;

        // Set the reverberation model to a bistatic common cache
    Manager bistatic( new eigenverb_bistatic( ocean, wave_source, wave_receiver, T0, bins, time_max ) ) ;
    wave_source.setReverberation_Model( bistatic ) ;
    wave_receiver.setReverberation_Model( bistatic ) ;

    cout << "Saving source wavefront to " << nc_source  << endl ;
    cout << "Saving receiver wavefront to " << nc_receiver << endl ;
    wave_source.init_netcdf( nc_source ) ;
    wave_source.save_netcdf() ;
    wave_receiver.init_netcdf( nc_receiver ) ;
    wave_receiver.save_netcdf() ;
    // propagate rays and record wavefronts to disk.

    cout << "propagate wavefront for " << time_max << " seconds" << endl ;
    while ( wave_source.time() < time_max && wave_receiver.time() < time_max ) {
        wave_source.step() ;
        wave_receiver.step() ;
        wave_source.save_netcdf() ;
        wave_receiver.save_netcdf() ;
    }
    wave_source.close_netcdf() ;
    wave_receiver.close_netcdf() ;

   // compute coherent propagation loss and write eigenrays to disk
    reverberation_model* reverb = bistatic.getPointer() ;
    cout << "computing reverberation levels" << endl ;
    struct timeval time ;
    struct timezone zone ;
    gettimeofday( &time, &zone ) ;
    double start = time.tv_sec + time.tv_usec * 1e-6 ;
    reverb->compute_reverberation() ;
    gettimeofday( &time, &zone ) ;
    double complete = time.tv_sec + time.tv_usec * 1e-6 ;
    cout << "Computation of reverberation curve took "
         << (complete-start) << " sec."
         << endl ;

    cout << "writing reverberation curve to " << csvname << endl;
    std::ofstream os(csvname);
    os << "time,intensity" << endl ;
    os << std::setprecision(18);
    cout << std::setprecision(18);

    const vector<double> reverb_tl = reverb->getReverberation_curve() ;
    vector<double> r = SL + 10.0*log10(reverb_tl) ;
    for ( unsigned i=0; i < bins; ++i ) {
        if( i % 10 == 0 ) {
            cout << "reverb_level(" << i << "): " << r(i) << endl ;
        }
        os << ( i * time_max / bins )
           << "," << r(i)
           << endl ;
    }
}
/// @}

BOOST_AUTO_TEST_SUITE_END()

