/**
 * @file malta_movie.cc
 *
 * Demonstrates the model's ability to visualize the wavefront in 3-D.
 * Uses the World Ocean Atlas and the ETOPO1 databases to construct a
 * real world environment near the Malta escarpment south-east of Sicily.
 *
 *      - Area: 34.5 to 37.0 North, 14,5 to 17.0 East
 *      - Month: December
 *      - Source: 36N, 16.0E, 10 meters deep
 *      - Frequency: 3000 Hz
 *      - Travel Time: 80 seconds
 *      - Time Step: 50 msec
 *      - D/E: [-45,0] in 1.00 deg steps
 *      - AZ: [180,360] in 10.0 deg steps
 *
 * The resulting wavefronts are stored in the "malta_movie.nc" netCDF
 * file for later plotting by Matlab, Octave, or other analysis routines.
 */
#include <usml/waveq3d/waveq3d.h>
#include <usml/netcdf/netcdf_files.h>
#include <usml/ocean/ocean.h>
#include <fstream>

using namespace usml::waveq3d ;
using namespace usml::netcdf ;
using namespace usml::ocean ;

/**
 * Command line interface.
 */
int main( int argc, char* argv[] ) {
    cout << "=== malta_movie ===" << endl ;

    // define scenario parameters

    int month = 12 ;		// december
    const double lat1 = 30.0 ;  // entire Mediterranean Sea
    const double lat2 = 46.0 ;
    const double lng1 = -8.0 ;
    const double lng2 = 37.0 ;
    wposition::compute_earth_radius( (lat1+lat2)/2.0 ) ;

    wposition1 pos( 35.983333333, 16.0, -10.0 ) ;
    seq_rayfan de( 0.0, 45.0, 181 ) ;
    seq_linear az( 225.0, 5.0, 315.0 ) ;
    const double time_max = 60.0 ;
    const double time_step = 0.050 ;

    seq_log freq( 3000.0, 1.0, 1 ) ;

    // load temperature & salinity data from World Ocean Atlas

    cout << "load temperature & salinity data from World Ocean Atlas" << endl ;
    netcdf_woa temperature(
	USML_DATA_DIR "/woa09/temperature_seasonal_1deg.nc",
	USML_DATA_DIR "/woa09/temperature_monthly_1deg.nc",
        month, lat1, lat2, lng1, lng2 ) ;
    netcdf_woa salinity(
	USML_DATA_DIR "/woa09/salinity_seasonal_1deg.nc",
	USML_DATA_DIR "/woa09/salinity_monthly_1deg.nc",
        month, lat1, lat2, lng1, lng2 ) ;

    // compute sound speed

    cout << "compute sound speed" << endl ;
    profile_model* profile =
    	new profile_mackenzie<float,3>( temperature, salinity ) ;

    // load bathymetry from ETOPO1 database

    cout << "load bathymetry from ETOPO1 database" << endl ;
    boundary_model* bottom = new boundary_grid<float,2>( new netcdf_bathy(
    	USML_DATA_DIR "/bathymetry/ETOPO1_Ice_g_gmt4.grd", 
	lat1, lat2, lng1, lng2 ) ) ;
    double height ;
    wvector1 normal ;
    bottom->height( pos, &height, &normal ) ;

    // combine sound speed and bathymetry into ocean model

    boundary_model* surface = new boundary_flat() ;
    ocean_model ocean( surface, bottom, profile ) ;

    // initialize wavefront

    const char* ncname = USML_STUDIES_DIR "/malta_movie/malta_movie.nc" ;
    cout << "propagate rays & record to " << ncname << endl ;
    wave_queue wave( ocean, freq, pos, de, az, time_step ) ;
    wave.init_netcdf( ncname ) ;
    wave.save_netcdf() ;

    // propagate wavefront

    while ( wave.time() < time_max ) {
        // cout << "time=" << wave.time() << endl ;
        wave.step() ;
        wave.save_netcdf() ;
    }
    wave.close_netcdf() ;
    cout << "wave propagated for " << wave.time() << " secs" << endl ;
}
