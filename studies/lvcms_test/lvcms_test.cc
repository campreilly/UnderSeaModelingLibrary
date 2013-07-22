/**
 * @file lvcms_test.cc
 *
 * Test the lvcms source location, and targets scenario.
 * Uses the World Ocean Atlas and the ETOPO1 databases to construct a
 * real world environment off the coast of Jacksonville FL.
 *
 *      - Area 31.6N to 26.4S, -77.5E to -82.5W
 *      - Month: May
 *      - Source:
 *      - Targets:
 *      - Frequency: 6500 Hz
 *      - Travel Time: 5 seconds
 *      - Time Step: 10 msec
 *      - D/E: [-90,90] as 181 tangent spaced rays
 *      - AZ: [0,360] in 15.0 deg steps
 *
 */
#include <usml/waveq3d/waveq3d.h>
#include <usml/netcdf/netcdf_files.h>
#include <fstream>
#include <iomanip>

using namespace usml::waveq3d ;
using namespace usml::netcdf ;
using namespace usml::ocean ;

#define DEBUG
/**
 * Command line interface.
 */
int main( int argc, char* argv[] ) {

    cout << "=== lvcms_test ===" << endl ;

    // define scenario parameters

    int month = 6 ;		// May
    const double lat1 = 26.4 ; //South
    const double lat2 = 31.6 ; //North
    const double lng1 = -82.5 ; // West
    const double lng2 = -77.5 ; // East

    wposition::compute_earth_radius( (lat1+lat2)/2.0 ) ;

    wposition1 src_pos( 29.0, -80.0, -90.0 ) ;

    seq_rayfan de( -90.0, 90.0, 181 ) ;
    seq_linear az( -180.0, 15.0, 180.0 ) ;

    const double time_max = 6.0 ;
    const double time_step = 0.010 ;

    seq_log freq( 6500.0, 1.0, 1 ) ;

    #ifdef DEBUG
        const char* csvname = "lvcms_test_eigenray.csv";
        const char* ncname = "lvcms_test_eigenrays.nc";
        const char* ncname_wave = "lvcms_test_wave_front.nc";
        const char* ncname_ssp = "lvcms_test_ssp.csv";
    #endif

    // build sound velocity profile from World Ocean Atlas data

    cout << "load temperature & salinity data from World Ocean Atlas" << endl ;
    netcdf_woa temperature(
    		USML_DATA_DIR "/woa09/temperature_seasonal_1deg.nc",
    		USML_DATA_DIR "/woa09/temperature_monthly_1deg.nc", month, lat1, lat2, lng1, lng2 ) ;
    netcdf_woa salinity(
    		USML_DATA_DIR "/woa09/salinity_seasonal_1deg.nc",
    		USML_DATA_DIR "/woa09/salinity_monthly_1deg.nc", month, lat1, lat2, lng1, lng2 ) ;

    profile_model* profile = new profile_mackenzie<double,3>( temperature, salinity ) ;


    cout << "load bathymetry from ETOPO1 database" << endl ;
    usml::types::wposition::compute_earth_radius(29.0);

    usml::netcdf::netcdf_bathy* pBathymetry = new usml::netcdf::netcdf_bathy(
    		USML_DATA_DIR "/bathymetry/ETOPO1_Ice_g_gmt4.grd",
    		lat1, lat2, lng1, lng2, usml::types::wposition::earth_radius);

    boundary_model* bottom = new boundary_grid<double,2>( pBathymetry ) ;

    // combine sound speed and bathymetry into ocean model
    boundary_model* surface = new boundary_flat() ;

#ifdef DEBUG

    // print ssp results for source lat/long entry
	std::ofstream ssp_output(ncname_ssp);
	cout << "writing tables to " << ncname_ssp << endl;

	unsigned index[3];
	index[1] = 0;
	index[2] = 0;

	matrix<double> speed(1, 1);
	wposition location(1, 1);
	location.latitude(0, 0, 29.0);
	location.longitude(0, 0, -80.0);
	wvector gradient(1, 1);

	ssp_output << "Depth,Temp,Sal,Speed,Gradient" << endl;

	for (unsigned d = 0; d < temperature.axis(0)->size(); ++d) {
		index[0] = d;
		location.rho(0, 0, (*temperature.axis(0))(d));
		profile->sound_speed(location, &speed, &gradient);
		ssp_output << -location.altitude(0, 0) << "," << temperature.data(index)
		<< "," << salinity.data(index) << "," << speed(0, 0) << ","
		<< -gradient.rho(0, 0) << std::endl;
	}

	ssp_output.close();

#endif

    ocean_model ocean( surface, bottom, profile ) ;

    cout << "initialize targets" << endl ;

    wposition target( 3, 1, src_pos.latitude(), src_pos.longitude(), src_pos.altitude() ) ;

    target.latitude ( 0, 0,  29.01) ;
    target.longitude( 0, 0, -80.0 ) ;
    target.altitude ( 0, 0, -10.0 ) ;

    target.latitude ( 1, 0,  29.05 ) ;
    target.longitude( 1, 0, -79.95 ) ;
    target.altitude ( 1, 0, -100.0 ) ;

    target.latitude ( 2, 0,  28.95 ) ;
    target.longitude( 2, 0, -80.05 ) ;
    target.altitude ( 2, 0, -100.0 ) ;

    proploss loss( &target ) ;
    wave_queue wave( ocean, freq, src_pos, de, az, time_step, &loss ) ;

    // propagate wavefront
	#ifdef DEBUG
        cout << "writing wavefronts to " << ncname_wave << endl;

        wave.init_netcdf( ncname_wave );
        wave.save_netcdf();
    #endif
    cout << "propagate wavefronts for " << time_max << " secs" << endl ;
    time_t start = time(NULL) ;
    while ( wave.time() < time_max ) {
        wave.step() ;
        #ifdef DEBUG
            wave.save_netcdf();
        #endif
    }

	#ifdef DEBUG
        wave.close_netcdf();

        // compute coherent propagation loss and write eigenrays to disk

        loss.sum_eigenrays();
        cout << "writing proploss to " << ncname << endl;
        loss.write_netcdf(ncname,"lvcms test eigenrays");

        // save results to spreadsheet and compare to analytic results

        cout << "writing tables to " << csvname << endl;
        std::ofstream os(csvname);
        os << "target, ray, time, intensity, phase, s_de, s_az, t_de, t_az, srf, btm, cst"
        << endl;
        os << std::setprecision(18);
        cout << std::setprecision(18);

        for (unsigned int m = 0; m < 3 /* num targets */; ++m) {
			const eigenray_list *raylist = loss.eigenrays(m,0);
			int n=0;
			for ( eigenray_list::const_iterator iter = raylist->begin();
					iter != raylist->end(); ++n, ++iter )
			{
				const eigenray &ray = *iter ;
				os  <<  m
					<< "," << n
					<< "," << ray.time
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
        }

        os.close();

	#endif


    time_t complete = time(NULL) ;
    cout << "Propagating for " << time_max << " sec with "
         << ( target.size1() * target.size2() ) << " targets took "
         << (difftime(complete,start)) << " sec."
         << endl ;
}
