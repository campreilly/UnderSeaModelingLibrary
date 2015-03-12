/**
 * @example studies/reverberation/reverberation_bistatic.cc
 */
#include <boost/progress.hpp>
#include <usml/ocean/ocean.h>
#include <usml/waveq3d/waveq3d.h>
#include <usml/eigenverb/eigenverb_collection.h>
#include <usml/eigenverb/envelope_monostatic.h>
#include <fstream>

using namespace usml::waveq3d ;
using namespace usml::eigenverb ;

//#define MONOSTATIC_DEBUG

/**
 * Produce a simple scenario where waveq3d eigenverb_monostatic can
 * produce a reverberation curve that can then be compared to the
 * classic results.
 */
int main() {
    cout << "=== reverberation_test: monostatic ===" << endl;
    const char* csvname = USML_STUDIES_DIR "/reverberation/monostatic.csv" ;
	#ifdef MONOSTATIC_DEBUG
		const char* nc_wave = USML_STUDIES_DIR "/reverberation/monostatic_wave.nc" ;
	#endif
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
    size_t bins = time_max / resolution ;
    const double SL = 200.0 ;

    // initialize propagation model

    attenuation_model* attn = new attenuation_constant( 0.0 ) ;
    profile_model* profile = new profile_linear( c0, attn ) ;

    boundary_model* surface = new boundary_flat() ;
    surface->scattering( new usml::ocean::scattering_lambert() ) ;

    double btm_spd = 1.10 ;
    double btm_dsty = 1.9 ;
    double btm_atn = 0.8 ;
    reflect_loss_model* btm_rflt =
        new reflect_loss_rayleigh( btm_dsty, btm_spd, btm_atn ) ;
    boundary_model* bottom = new boundary_flat( depth, btm_rflt ) ;
    bottom->scattering( new scattering_lambert() ) ;

    // create a simple volume layer
//    boundary_model* v1 = new boundary_flat( 100.0 ) ;
//    v1->scattering( new scattering_lambert() ) ;
//    vector<boundary_model*> v(1) ;
//    v[0] = v1 ;
//    volume_layer* volume = new volume_layer( v ) ;
//
//    ocean_model ocean( surface, bottom, profile, volume ) ;
    ocean_model ocean( surface, bottom, profile ) ;

    seq_log freq( f0, 1.0, 1 );
    wposition1 pos( lat, lng, alt ) ;
    seq_rayfan de ;
//    seq_rayfan de( -90.0, 0.0, 91 ) ;
//    seq_linear de( -89.5, 0.5, 1.0 ) ;
    seq_linear az( 0.0, 360.0, 360.0 ) ;

    wave_queue wave( ocean, freq, pos, de, az, time_step ) ;
    eigenverb_collection monostatic( ocean.num_volume() ) ;
//    eigenverb_collection monostatic( lng, lat, M_PI/10.0, M_PI/10.0, ocean.num_volume() ) ;
    wave.add_eigenverb_listener( &monostatic ) ;

	#ifdef MONOSTATIC_DEBUG
		cout << "Saving wavefront to " << nc_wave << endl ;
		wave.init_netcdf(nc_wave) ;
		wave.save_netcdf() ;
	#endif
    // propagate rays and record wavefronts to disk.

    cout << "propagate wavefront for " << time_max << " seconds" << endl ;
    while ( wave.time() < time_max ) {
        wave.step() ;
		#ifdef MONOSTATIC_DEBUG
        	wave.save_netcdf() ;
		#endif
    }
	#ifdef MONOSTATIC_DEBUG
    	wave.close_netcdf() ;
	#endif

   // write eigenverbs to disk
    #ifdef MONOSTATIC_DEBUG
        const char* bottom_eigenverbs = USML_STUDIES_DIR "/reverberation/monostatic_eigenverbs_bottom.nc" ;
        const char* surface_eigenverbs = USML_STUDIES_DIR "/reverberation/monostatic_eigenverbs_surface.nc" ;
        cout << "writing bottom eigenverbs to " << bottom_eigenverbs << endl ;
        monostatic.write_netcdf(bottom_eigenverbs, usml::eigenverb::BOTTOM) ;
        cout << "writing surface eigenverbs to " << surface_eigenverbs << endl ;
        monostatic.write_netcdf(surface_eigenverbs, usml::eigenverb::SURFACE) ;
    #endif

    envelope_collection levels( resolution, bins, az.size() ) ;
    envelope_monostatic reverb( ocean, T0, time_max ) ;
    cout << "computing reverberation levels" << endl ;
    {
        boost::progress_timer timer ;
    	reverb.generate_envelopes( monostatic, monostatic, &levels ) ;
    }

    const char* reverb_file = USML_STUDIES_DIR "/reverberation/monostatic_envelopes.nc" ;
    cout << "writing reverberation curves to " << reverb_file << endl ;
    levels.write_netcdf( reverb_file ) ;

//    cout << "writing reverberation curve to " << csvname << endl;
//    std::ofstream os(csvname);
//    os << "time,intensity" << endl ;
//    os << std::setprecision(18);
//    cout << std::setprecision(18);

    vector<double> r = levels.envelopes(0) ;
//    for(size_t i=1; i<az.size(); ++i) {
//        r += levels.envelopes(i) ;
//    }
    r = SL + 10.0*log10( r ) ;
    for ( size_t i=0; i < bins; ++i ) {
        if( i % 10 == 0 ) {
            cout << "reverb_level(" << i << "): " << r(i) << endl ;
        }
//        os << ( i * time_max / bins )
//           << "," << r(i)
//           << endl ;
    }
}
