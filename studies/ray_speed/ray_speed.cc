/**
 * @file ray_speed.cc
 *
 * Measure the speed of the model in a realistic scenario.
 * Uses the World Ocean Atlas and the ETOPO1 databases to construct a
 * real world environment near the Malta escarpment south-east of Sicily.
 *
 *      - Area 30N-46N, 8W-37E (entire Mediterranean Sea)
 *      - Month: December
 *      - Source: 36N, 16.0E, 100 meters deep
 *      - Targets: 6x6 grid +/- 2.5 degrees around source
 *      - Frequency: 3000 Hz
 *      - Travel Time: 60 seconds
 *      - Time Step: 50 msec
 *      - D/E: [-90,90] as 181 tangent spaced rays
 *      - AZ: [0,360] in 5.0 deg steps
 *
 */
#include <usml/waveq3d/waveq3d.h>
#include <usml/netcdf/netcdf_files.h>
#include <fstream>

using namespace usml::waveq3d ;
using namespace usml::netcdf ;
using namespace usml::ocean ;

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

    int month = 12 ;		// december
    const double lat1 = 30.0 ;  // entire Mediterranean Sea
    const double lat2 = 46.0 ;
    const double lng1 = -8.0 ;
    const double lng2 = 37.0 ;
    wposition::compute_earth_radius( (lat1+lat2)/2.0 ) ;

    wposition1 pos( 36.0, 16.0, -10.0 ) ;
    seq_rayfan de( -90.0, 90.0, 181 ) ;
    seq_linear az( 0.0, 15.0, 360.0 ) ;
    const double time_max = 60.0 ;
    const double time_step = 0.100 ;

    seq_log freq( 3000.0, 1.0, 1 ) ;

    // build sound velocity profile from World Ocean Atlas data

    cout << "load temperature & salinity data from World Ocean Atlas" << endl ;
    netcdf_woa temperature(
	USML_DATA_DIR "/woa09/temperature_seasonal_1deg.nc",
	USML_DATA_DIR "/woa09/temperature_monthly_1deg.nc",
        month, lat1, lat2, lng1, lng2 ) ;
    netcdf_woa salinity(
	USML_DATA_DIR "/woa09/salinity_seasonal_1deg.nc",
	USML_DATA_DIR "/woa09/salinity_monthly_1deg.nc",
        month, lat1, lat2, lng1, lng2 ) ;
    profile_model* profile =
    	new profile_mackenzie<double,3>( temperature, salinity ) ;
//    attenuation_model* attn = new attenuation_constant(0.0);
//    profile_model* profile = new profile_linear(1500.0,attn);

    // load bathymetry from ETOPO1 database

    cout << "load bathymetry from ETOPO1 database" << endl ;
    boundary_model* bottom = new boundary_grid<double,2>( new netcdf_bathy(
    	USML_DATA_DIR "/bathymetry/ETOPO1_Ice_g_gmt4.grd", 
	lat1, lat2, lng1, lng2 ) ) ;
//    boundary_model* bottom = new boundary_flat(3000.0);
    double height ;
    wvector1 normal ;
    bottom->height( pos, &height, &normal ) ;

    // combine sound speed and bathymetry into ocean model

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
