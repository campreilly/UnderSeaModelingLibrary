/**
 * @example waveq3d/test/reverberation_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <usml/waveq3d/waveq3d_reverb.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdio>

BOOST_AUTO_TEST_SUITE(reverberation_test)

using namespace boost::unit_test ;
using namespace usml::waveq3d ;

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
    const char* csvname = USML_TEST_DIR "/waveq3d/test/monostatic.csv" ;
    double time_max = 7.0 ;
    double time_step = 0.1 ;
    double T0 = 0.1 ;                 // Pulse length
    const double f0 = 2000 ;
    const double lat = 0.0 ;
    const double lng = 0.0 ;
    const double alt = 0.0 ;
    const double c0 = 1500.0 ;              // constant sound speed
    const double depth = 200.0 ;
    unsigned bins = time_max / time_step ;
    const double SL = pow( 10.0, 20.0 ) ;

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
    boundary_model* v1 = new boundary_flat( 100.0 ) ;
    v1->setScattering_Model( new scattering_lambert() ) ;
    vector<boundary_model*> v(1) ;
    v[0] = v1 ;
    volume_layer* volume = new volume_layer( v ) ;

    ocean_model ocean( surface, bottom, profile, volume ) ;

    seq_log freq( f0, 1.0, 1 );
    wposition1 pos( lat, lng, alt ) ;
    seq_rayfan de( -90.0, 0.0, 361 ) ;
    seq_linear az( -0.01, 0.01, 0.01 ) ;

    wave_queue_reverb wave( ocean, freq, pos, de, az, time_step, T0, bins, time_max ) ;

    // propagate rays and record wavefronts to disk.

    cout << "propagate wavefront for " << time_max << " seconds" << endl ;
    while ( wave.time() < time_max ) {
        wave.step() ;
    }

   // compute coherent propagation loss and write eigenrays to disk

    reverberation_model* reverb = wave.getReverberation_Model() ;
    cout << "computing reverberation levels" << endl ;
    reverb->compute_reverberation() ;
    cout << "writing reverberation curve to " << csvname << endl;
    std::ofstream os(csvname);
    os << "time,intensity" << endl ;
    os << std::setprecision(18);
    cout << std::setprecision(18);

    const vector<double> reverb_tl = reverb->getReverberation_curve() ;
    vector<double> r = 10.0*log10(SL*2.0*reverb_tl) ;
    for ( unsigned i=0; i < bins; ++i ) {
//        double r = 10*log10(2.0*reverb_tl(i)) ;
        cout << "reverb_level(" << i << "): " << r(i) << endl ;
        os << ( i * time_max / bins )
           << "," << r(i)
           << endl ;
    }
}
/// @}

BOOST_AUTO_TEST_SUITE_END()

