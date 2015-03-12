/**
 * @example studies/reverberation/reverberation_bistatic.cc
 */
#include <boost/progress.hpp>
#include <usml/ocean/ocean.h>
#include <usml/waveq3d/waveq3d.h>
#include <usml/eigenverb/eigenverb_collection.h>
#include <usml/eigenverb/envelope_generator.h>
#include <fstream>

using namespace usml::waveq3d ;
using namespace usml::utilities ;

//#define BISTATIC_DEBUG

/**
 * Produce a simple scenario where waveq3d eigenverb_bistatic can
 * produce a reverberation curve that can then be compared to the
 * classic results.
 */
int main() {
    cout << "=== reverberation_bistatic ===" << endl ;

    const char* csvname = USML_STUDIES_DIR "/reverberation/bistatic.csv" ;
    const char* ssp_file = USML_STUDIES_DIR "/reverberation/bistatic_sound_speed.txt" ;
	#ifdef BISTATIC_DEBUG
		const char* nc_source = USML_STUDIES_DIR "/reverberation/bistatic_wave_source.nc" ;
		const char* nc_receiver = USML_STUDIES_DIR "/reverberation/bistatic_wave_receiver.nc" ;
	#endif
    double time_max = 10.0 ;
    double time_step = 0.1 ;
    double resolution = 0.1 ;
    double T0 = 1.0 ;                 // Pulse length
    const double f0 = 1000.0 ;
    const double src_lat = 0.0 ;
    const double src_lng = 0.0 ;
    const double src_alt = -50.0 ;
    const double rcvr_lat = 0.018 ;             // 2 km north of the source
    const double rcvr_lng = 0.0 ;
    const double rcvr_alt = -50.0 ;
    const double depth = 1000.0 ;
    size_t bins = time_max / resolution ;
    const double SL = 250.0 ;

    // initialize propagation model

    attenuation_model* attn = new attenuation_constant( 0.0 ) ;
    profile_model* profile = new profile_grid<double,1>( new ascii_profile( ssp_file ) ) ;
    profile->attenuation( attn ) ;

    boundary_model* surface = new boundary_flat() ;
    surface->scattering( new scattering_lambert() ) ;

    double btm_speed = 0.9860893 ;
    double btm_density = 1.1480675 ;
    double btm_atten = 0.0192162 ;
    boundary_model* bottom = new boundary_flat( depth ) ;
    bottom->reflect_loss( new reflect_loss_rayleigh( btm_density, btm_speed, btm_atten ) ) ;
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
    wposition1 source( src_lat, src_lng, src_alt ) ;
    wposition1 receiver( rcvr_lat, rcvr_lng, rcvr_alt ) ;
    seq_linear de( -90.0, 1.0, 90.0 ) ;
    seq_linear az( 0.0, 45.0, 360.0 ) ;
    wave_queue wave_source( ocean, freq, source, de, az, time_step ) ;
    wave_queue wave_receiver( ocean, freq, receiver, de, az, time_step ) ;

        // Set the reverberation model to a bistatic common cache
    eigenverb_collection source_bistatic( ocea.num_volume() ) ;
    eigenverb_collection receiver_bistatic( ocea.num_volume() ) ;
    wave_source.add_eigenverb_listener( &source_bistatic ) ;
    wave_receiver.add_eigenverb_listener( &receiver_bistatic ) ;
    cout << "Bistatic reverberation source and receiver wave have been set." << endl ;

	#ifdef BISTATIC_DEBUG
		cout << "Saving source wavefront to " << nc_source  << endl ;
		cout << "Saving receiver wavefront to " << nc_receiver << endl ;
		wave_source.init_netcdf( nc_source ) ;
		wave_source.save_netcdf() ;
		wave_receiver.init_netcdf( nc_receiver ) ;
		wave_receiver.save_netcdf() ;
	#endif
    // propagate rays and record wavefronts to disk.

    cout << "propagate wavefront for " << time_max << " seconds" << endl ;
    while ( wave_source.time() < time_max && wave_receiver.time() < time_max ) {
        wave_source.step() ;
        wave_receiver.step() ;
		#ifdef BISTATIC_DEBUG
			wave_source.save_netcdf() ;
			wave_receiver.save_netcdf() ;
		#endif
    }
	#ifdef BISTATIC_DEBUG
		wave_source.close_netcdf() ;
		wave_receiver.close_netcdf() ;
	#endif

    #ifdef BISTATIC_DEBUG
        const char* src_bottom_eigenverbs = USML_STUDIES_DIR "/reverberation/source_bistatic_eigenverbs_bottom.nc" ;
        const char* src_surface_eigenverbs = USML_STUDIES_DIR "/reverberation/source_bistatic_eigenverbs_surface.nc" ;
        cout << "writing source bottom eigenverbs to " << src_bottom_eigenverbs << endl ;
        source_bistatic.write_netcdf(src_bottom_eigenverbs, usml::eigenverb::BOTTOM) ;
        cout << "writing source surface eigenverbs to " << src_surface_eigenverbs << endl ;
        source_bistatic.write_netcdf(src_surface_eigenverbs, usml::eigenverb::SURFACE) ;
        const char* rcv_bottom_eigenverbs = USML_STUDIES_DIR "/reverberation/receiver_bistatic_eigenverbs_bottom.nc" ;
        const char* rcv_surface_eigenverbs = USML_STUDIES_DIR "/reverberation/receiver_bistatic_eigenverbs_surface.nc" ;
        cout << "writing receiver bottom eigenverbs to " << rcv_bottom_eigenverbs << endl ;
        receiver_bistatic.write_netcdf(rcv_bottom_eigenverbs, usml::eigenverb::BOTTOM) ;
        cout << "writing receiver surface eigenverbs to " << rcv_surface_eigenverbs << endl ;
        receiver_bistatic.write_netcdf(rcv_surface_eigenverbs, usml::eigenverb::SURFACE) ;
    #endif

   // compute coherent propagation loss and write eigenrays to disk
    envelope_collection levels( resolution, bins, az.size() ) ;
    envelope_monostatic reverb( ocean, T0, time_max ) ;
    cout << "computing reverberation levels" << endl ;
    {
        boost::progress_timer timer ;
        reverb.generate_envelopes( source_bistatic, receiver_bistatic, &levels ) ;
    }

    const char* reverb_file = USML_STUDIES_DIR "/reverberation/bistatic_envelopes.nc" ;
    cout << "writing reverberation curves to " << reverb_file << endl ;
    levels.write_netcdf( reverb_file ) ;

    vector<double> r = levels.envelopes(0) ;
    r = SL + 10.0*log10( r ) ;
    for ( size_t i=0; i < bins; ++i ) {
        if( i % 10 == 0 ) {
            cout << "reverb_level(" << i << "): " << r(i) << endl ;
        }
        os << ( i * time_max / bins )
           << "," << r(i)
           << endl ;
    }
}
