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

/**
 * Command line interface.
 */
int main() {
    cout << "=== WaveQ3D Visualization Demo ===" << endl ;

    // define scenario parameters
    int month = 1 ;		        // January
    const double lat1 = 24.0 ;  // gulf of oman
    const double lat2 = 26.0 ;
    const double lng1 = 56.0 ;
    const double lng2 = 58.0 ;
    wposition::compute_earth_radius( (lat1+lat2)/2.0 ) ;

    wposition1 pos( 25.3, 57.1, -400.0 ) ;
//    seq_rayfan de( -34.0, 36.0, 21 ) ;
//    seq_linear az( 0.0, 15.0, 360.0 ) ;
//    seq_rayfan de ;
//    seq_linear az( 0.0, 15.0, 360.0 ) ;
    seq_linear de( -90.0, 2.0, 90.0 ) ;
    seq_linear az( 0.0, 2.0, 360.0 ) ;
    const double time_max = 25.0 ;
    const double time_step = 0.1 ;

    seq_log freq( 6500.0, 1.0, 1 ) ;

    // create filenames to store data in
    const char* csvname = USML_STUDIES_DIR "/waveq3d_visual/gulf_oman_eigenray.csv";
    const char* ncname = USML_STUDIES_DIR "/waveq3d_visual/gulf_oman_eigenray.nc";
    const char* ncname_wave = USML_STUDIES_DIR "/waveq3d_visual/gulf_oman_eigenray_wave.nc";

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

//    profile_model* profile = new profile_grid<double,3>(
//        data_grid_mackenzie::construct(temperature, salinity) ) ;

    data_grid<double,3>* ssp = data_grid_mackenzie::construct( temperature, salinity ) ;
    data_grid_svp* fast_ssp = new data_grid_svp(ssp, true) ;
    profile_model* profile = new profile_grid_fast( fast_ssp ) ;

    // load bathymetry from ETOPO1 database
    cout << "loading bathymetry from ETOPO1 database" << endl ;
//    data_grid<double,2>* grid = new netcdf_bathy( USML_DATA_DIR "/bathymetry/ETOPO1_Ice_g_gmt4.grd",
//        lat1, lat2, lng1, lng2 );
//    data_grid_bathy* fast_grid = new data_grid_bathy(grid, true) ;
//    boundary_model* bottom = new boundary_grid_fast( fast_grid ) ;
    boundary_model* bottom = new boundary_grid<double,2>( new netcdf_bathy(
        USML_DATA_DIR "/bathymetry/ETOPO1_Ice_g_gmt4.grd", lat1, lat2, lng1, lng2 ) );

    boundary_model* surface = new boundary_flat() ;

    // combine sound speed and bathymetry into ocean model
    ocean_model ocean( surface, bottom, profile ) ;

    // initialize proploss targets and wavefront
    cout << "initializing target" << endl ;
    const double tar_range = 1000.0 ;
    const double tar_alt = -10.0 ;
    wposition target( 1, 1, 0.0, 0.0, tar_alt ) ;
    wposition1 atarget( pos, tar_range, 0.0 ) ;
    target.latitude( 0, 0, atarget.latitude() ) ;
    target.longitude( 0, 0, atarget.longitude() ) ;

    proploss loss(freq, pos, de, az, time_step, &target);
	wave_queue wave( ocean, freq, pos, de, az, time_step, &target ) ;
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

    // save results to spreadsheet and compare to analytic results
    cout << "writing tables to " << csvname << endl;
    std::ofstream os(csvname);
    os << "time,intensity,phase,s_de,s_az,t_de,t_az,srf,btm,cst"
    << endl;
    os << std::setprecision(18);
    cout << std::setprecision(18);

    const eigenray_list *raylist = loss.eigenrays(0,0);
    int n=0;
    for ( eigenray_list::const_iterator iter = raylist->begin();
            iter != raylist->end(); ++n, ++iter )
    {
        const eigenray &ray = *iter ;
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

    cout << "*** Scenario files created ***" << endl;
}
