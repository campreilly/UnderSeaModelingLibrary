/**
 * @file cmp_speed.cc
 *
 * Compare the speed of the model to the STD14 scenario from CASS.
 * In this scenario, CASS specifies a grid of sound speeds and
 * bottom depths in latitude and longitude coordinates for an
 * area near Hawaii.
 *
 *      - Area 16.2N-24.6N, 164.7-155.4W
 *      - Source: 19.52N 160.5W, 200 meters deep (SW of Hawaii)
 *      - Targets: ring of receivers at 100 km from source
 *      - Frequency: 250 Hz
 *      - Travel Time: 80 seconds
 *      - Time Step: 100 msec
 *      - D/E: [-90,90] as 181 tangent spaced rays
 *      - AZ: [0,360] in 15.0 deg steps
 *
 */
#include <usml/waveq3d/waveq3d.h>
#include <usml/netcdf/netcdf_files.h>
#include <fstream>

using namespace usml::waveq3d ;
using namespace usml::netcdf ;

/**
 * Command line interface.
 */
int main( int argc, char* argv[] ) {
    cout << "=== ray_speed ===" << endl ;

    int num_targets = 100 ;
    if ( argc > 1 ) {
        num_targets = atoi( argv[1] ) ;
    }

    // define scenario parameters

    seq_linear freq( 250.0, 1.0, 1 ) ;
    wposition::compute_earth_radius( 19.52 ) ;
    wposition1 pos( 19.52, -160.5, -200.0 ) ;
    seq_rayfan de( -90.0, 90.0, 181 ) ;
    seq_linear az( 0.0, 15.0, 360.0 ) ;
    const double time_max = 80.0 ;
    const double time_step = 0.100 ;

    // load STD14 environmental data from netCDF files

    cout << "load STD14 environmental data" << endl ;
    const double lat1 = 16.2 ;
    const double lat2 = 24.6 ;
    const double lng1 = -164.7 ;
    const double lng2 = -155.4 ;
    profile_model* profile = new profile_grid<double,3>( new netcdf_profile(
            USML_STUDIES_DIR "/std14profile.nc", 0.0, lat1, lat2, lng1, lng2,
            wpostion::earth_radius ) ) ;
    boundary_model* bottom = new boundary_grid<double,2>( new netcdf_bathy(
            USML_STUDIES_DIR "/std14bathy.nc", lat1, lat2, lng1, lng2,
            wpostion::earth_radius ) ) ;
    boundary_model* surface = new boundary_flat() ;
    ocean_model ocean( surface, bottom, profile ) ;

    // initialize proploss targets and wavefront

    cout << "initialize " << num_targets << " targets" << endl ;
    randgen::seed(0) ;  // fix the initial seed
    wposition target( num_targets, 1, pos.latitude(), pos.longitude(), pos.altitude() ) ;
    for ( unsigned n=0 ; n < target.size1() ; ++n ) {
        target.latitude(  n, 0, pos.latitude() + randgen::uniform() - 0.5 ) ;
        target.longitude( n, 0, pos.longitude() + randgen::uniform() - 0.5 ) ;
    }
    proploss loss( &target ) ;
    wave_queue wave( ocean, freq, pos, de, az, time_step, &loss ) ;
//    wave_queue wave( ocean, freq, pos, de, az, time_step ) ;

    // propagate wavefront

    cout << "propagate wavefronts for " << time_max << " secs" << endl ;
    time_t start = time(NULL) ;
    while ( wave.time() < time_max ) {
        wave.step() ;
    }
    time_t complete = time(NULL) ;
    cout << "Progating for " << time_max << " sec with "
         << ( target.size1() * target.size2() ) << " targets took "
         << (difftime(complete,start)) << " sec."
         << endl ;
}
