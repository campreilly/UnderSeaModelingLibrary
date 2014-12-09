/**
 * @file waveq3d_visual.cc
 *
 * Create a demo scenario to allow for the visualization of the
 * benefits and properties of WaveQ3D, a fully three dimensional
 * ray based model of sound propagation in water.
 *
 *      - Area 24N-26N, 56E-58E (Gulf of Oman)
 *      - Month: January
 *      - Source: 25.3N, 57.1E, 400 meters deep
 *      - Targets: 1km due North
 *      - Frequency: 6500 Hz
 *      - Travel Time: 25 seconds
 *
 */

#include <fstream>
#include <iomanip>

#include <usml/waveq3d/waveq3d.h>
#include <usml/netcdf/netcdf_files.h>

using namespace usml::waveq3d ;
using namespace usml::netcdf ;
using namespace usml::ocean ;

#define BALLARD_DATA "/home/david/usml-private/studies/florida_straits"

/**
 * Command line interface.
 */
int main() {
    cout << "=== WaveQ3D Visualization Demo ===" << endl ;

    // define scenario parameters
    int month = 9 ;
    double lat1 = 25.0 ;
    double lat2 = 27.0 ;
    double lng1 = -80.2 ;
    double lng2 = -78.0 ;
    wposition::compute_earth_radius( (lat1+lat2)/2.0 ) ;

    // create filenames to store data in
//	const char* bathy_name = BALLARD_DATA "/flstrts_bathymetry.asc" ;
//	const char* bathy_nc = BALLARD_DATA "/run1a_bathy.nc" ;
    const char* ncname_wave = USML_STUDIES_DIR "/waveq3d_visual/florida_straits_wave.nc";
    const char* ncname = USML_STUDIES_DIR "/waveq3d_visual/florida_straits_proploss.nc";

    // build sound velocity profile from World Ocean Atlas data
    cout << "loading temperature & salinity data from World Ocean Atlas" << endl ;
    netcdf_woa* temperature = new netcdf_woa(
                                USML_DATA_DIR "/woa09/temperature_seasonal_1deg.nc",
                                USML_DATA_DIR "/woa09/temperature_monthly_1deg.nc",
                                month, lat1, lat2, lng1, lng2 ) ;
    netcdf_woa* salinity = new netcdf_woa(
                            USML_DATA_DIR "/woa09/salinity_seasonal_1deg.nc",
                            USML_DATA_DIR "/woa09/salinity_monthly_1deg.nc",
                            month, lat1, lat2, lng1, lng2 ) ;
    data_grid<double,3>* ssp = data_grid_mackenzie::construct( temperature, salinity ) ;
    data_grid_svp* fast_ssp = new data_grid_svp(ssp, true) ;
    profile_model* profile = new profile_grid_fast( fast_ssp ) ;

    // load bathymetry from ETOPO1 database
    cout << "loading bathymetry from NetCDF database" << endl ;
//    ascii_arc_bathy* bathymetry = new ascii_arc_bathy( bathy_name ) ;
//    data_grid_bathy* fast_grid = new data_grid_bathy( new ascii_arc_bathy( bathy_name ) , true ) ;
//    reflect_loss_model* ballard = new reflect_loss_netcdf( bathy_nc ) ;
//    boundary_model* bottom = new boundary_grid_fast( fast_grid, ballard ) ;
    cout << "load bathymetry from ETOPO1 database" << endl ;
    boundary_model* bottom = new boundary_grid<double,2>( new netcdf_bathy(
    	USML_DATA_DIR "/bathymetry/ETOPO1_Ice_g_gmt4.grd",
	lat1, lat2, lng1, lng2 ) ) ;

    boundary_model* surface = new boundary_flat() ;

    wposition1 receiver( 26.0217, -79.99054, -20.0 ) ;
    double rho ;
    bottom->height( receiver, &rho ) ;
    receiver.rho(rho) ;

    // combine sound speed and bathymetry into ocean model
    ocean_model ocean( surface, bottom, profile ) ;

    // initialize proploss targets and wavefront
    cout << "initializing target" << endl ;
    seq_linear range( 3e3, 100.0, 80e3 ) ; // 3 to 80 km
    double bearing = to_radians(8.0) ;
    wposition source( range.size(), 1, 0.0, 0.0, -100.0 ) ;
    for ( unsigned n=0 ; n < range.size() ; ++n ) {
        double d = range(n) / wposition::earth_radius ;
        double lat = to_degrees( asin(
            cos( receiver.theta() ) * cos(d) +
            sin( receiver.theta() ) * sin(d) * cos(bearing) ) ) ;
        double lng = to_degrees( receiver.phi() + asin(
            sin(bearing) * sin(d) / sin(receiver.theta()) ) ) ;
        source.latitude(  n, 0, lat ) ;
        source.longitude( n, 0, lng ) ;
    }

    seq_linear freq( 415.0, 1.0, 1 ) ;
    seq_linear de( 0.0, 2.0, 60.0 ) ;
    seq_linear az( -40.0, 2.0, 20.0 ) ;
    const double time_max = 60.0 ;
    const double time_step = 0.1 ;

    proploss loss(freq, receiver, de, az, time_step, &source);
	wave_queue wave( ocean, freq, receiver, de, az, time_step, &source ) ;
	wave.addEigenrayListener(&loss);

    // setup netcdf files
    cout << "Generating scenario files..." << endl ;
    cout << "writing wavefronts to " << ncname_wave << endl;
    wave.init_netcdf( ncname_wave );
    wave.save_netcdf();

    // propagate wavefront
    while ( wave.time() < time_max ) {
        wave.step() ;
        wave.save_netcdf();
    }

    wave.close_netcdf();

    // compute coherent propagation loss and write eigenrays to disk
    loss.sum_eigenrays();
    cout << "writing proploss to " << ncname << endl;
    loss.write_netcdf(ncname,"WQ3D Visualization");

    cout << "*** Scenario files created ***" << endl;
}
