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
#include <iomanip>
#include <boost/timer/timer.hpp>

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

    #ifdef USML_DEBUG
        const char* csvname = USML_STUDIES_DIR "/ray_speed/ray_speed_eigenray.csv";
        const char* ncname = USML_STUDIES_DIR "/ray_speed/ray_speed_eigenray.nc";
        const char* ncname_wave = USML_STUDIES_DIR "/ray_speed/ray_speed_eigenray_wave.nc";
    #endif

    // build sound velocity profile from World Ocean Atlas data

    cout << "load temperature & salinity data from World Ocean Atlas" << endl ;
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
//    attenuation_model* attn = new attenuation_constant(0.0);
//    profile->attenuation(attn);

    // load bathymetry from ETOPO1 database

    data_grid<double,2>* grid = new netcdf_bathy( USML_DATA_DIR "/bathymetry/ETOPO1_Ice_g_gmt4.grd",
        lat1, lat2, lng1, lng2 );
    data_grid_bathy* fast_grid = new data_grid_bathy(grid, true) ;
    cout << "load bathymetry from ETOPO1 database" << endl ;
    boundary_model* bottom = new boundary_grid_fast( fast_grid ) ;

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

    proploss loss(freq, pos, de, az, time_step, &target);
	wave_queue wave( ocean, freq, pos, de, az, time_step, &target ) ;
	wave.addEigenrayListener(&loss) ;

    // propagate wavefront

    #ifdef USML_DEBUG
        cout << "writing wavefronts to " << ncname_wave << endl;

        wave.init_netcdf( ncname_wave );
        wave.save_netcdf();
    #endif

    cout << "propagate wavefronts for " << time_max << " secs" << endl ;
    {
        boost::timer::auto_cpu_timer timer ;
        while ( wave.time() < time_max ) {
            wave.step() ;
        }
    }

    #ifdef USML_DEBUG
        wave.close_netcdf();

        // compute coherent propagation loss and write eigenrays to disk

        loss.sum_eigenrays();
        cout << "writing proploss to " << ncname << endl;
        loss.write_netcdf(ncname,"WQ3Deigenray test");

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
    #endif
}
