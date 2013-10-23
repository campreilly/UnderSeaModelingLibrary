/**
 * @file malta_rays.cc
 *
 * Reflect rays off of the ETOPO1 bottom near the Malta escarpment.
 * Use December sound velocity profile from World Ocean Atlas.
 * Ray path starts from 36N 16E, D/E=-90->0 deg and AZ set from command line.
 *
 * At an azimuth of 270, the run for 90 seconds should propagate up the
 * slope, turn around and head back down the slope.  Because of slop
 * features in the latitude direction, the ray does not stay in
 * the east-west plane.
 *
 * At an azimuth of 315, the shallowest rays run into dry land.  This
 * stimulates the model's ability to turn around rays in very shallow water.
 *
 * Results are stored to speadsheet files for later plotting.
 */
#include <usml/waveq3d/waveq3d.h>
#include <usml/netcdf/netcdf_files.h>
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace usml::waveq3d ;
using namespace usml::netcdf ;
using namespace usml::ocean ;

/**
 * Command line interface.
 */
int main( int argc, char* argv[] ) {
    cout << "=== malta_rays_test ===" << endl ;

    // define scenario parameters

    int month = 12 ;			// december
    const double lat1 = 30.0 ;  // entire Mediterranean Sea
    const double lat2 = 46.0 ;
    const double lng1 = -8.0 ;
    const double lng2 = 37.0 ;
    wposition::compute_earth_radius( (lat1+lat2)/2.0 ) ;

    wposition1 pos( 35.983333333, 16.0, -10.0 ) ;
    seq_linear de( -45.0, 5.0, 0.0 ) ;

    double angle = (argc <= 1) ? 270.0 : atof(argv[1]) ;
    cout << "azimuth=" << angle << endl ;
    seq_linear az( angle, 1.0, 1 ) ;
    const double time_max = 90.0 ;
    const double time_step = 0.1 ;

    seq_log freq( 3000.0, 1.0, 1 ) ;

    // load temperature & salinity data from World Ocean Atlas

    cout << "load temperature & salinity data from World Ocean Atlas" << endl ;
    netcdf_woa* temperature = new netcdf_woa(
	USML_DATA_DIR "/woa09/temperature_seasonal_1deg.nc",
	USML_DATA_DIR "/woa09/temperature_monthly_1deg.nc",
        month, lat1, lat2, lng1, lng2 ) ;
    netcdf_woa* salinity = new netcdf_woa(
	USML_DATA_DIR "/woa09/salinity_seasonal_1deg.nc",
	USML_DATA_DIR "/woa09/salinity_monthly_1deg.nc",
        month, lat1, lat2, lng1, lng2 ) ;

    // compute sound speed

    cout << "compute sound speed" << endl ;
    profile_model* profile = new profile_grid<double,3>(
        data_grid_mackenzie::construct(temperature, salinity) ) ;

    // load bathymetry from ETOPO1 database

    cout << "load bathymetry from ETOPO1 database" << endl ;
    boundary_model* bottom = new boundary_grid<double,2>( new netcdf_bathy(
    	USML_DATA_DIR "/bathymetry/ETOPO1_Ice_g_gmt4.grd",
	lat1, lat2, lng1, lng2 ) ) ;

    double height ;
    wvector1 normal ;
    bottom->height( pos, &height, &normal ) ;

    // combine sound speed and bathymetry into ocean model

    boundary_model* surface = new boundary_flat() ;
    ocean_model ocean( surface, bottom, profile ) ;

    // initialize wavefront

    cout << "propagate rays" << endl ;
    wave_queue wave( ocean, freq, pos, de, az, time_step ) ;

    // initialize output to spreadsheet files

    std::ofstream*** os = new std::ofstream**[de.size()] ;
    for ( unsigned d=0 ; d < de.size() ; ++d ) {
    	os[d] = new std::ofstream*[az.size()] ;
    	for ( unsigned a=0 ; a < az.size() ; ++a ) {
            static char csvname[256] ;
    	    sprintf( csvname, "%s/malta_rays_%02.0f_%02.0f.csv",
    		USML_STUDIES_DIR "/malta_rays",
    	        fabs(de(d)), fabs(az(a)) ) ;
    	    os[d][a] = new std::ofstream(csvname) ;
            *os[d][a] << "t,"
                << "lat,lng,alt,"
                << "de,az,srf,bot,cst,"
                << "r,theta,phi,"
                << "rd,thd,phid,"
                << "mu,eta,nu,"
                << "mud,etad,nud,"
                << "c,dcdz"
                << endl ;

            wvector1 ndir( wave.curr()->ndirection, d, a ) ;
            double de, az ;
            ndir.direction( &de, &az ) ;

            *os[d][a] << wave.time() << ','
                << wave.curr()->position.latitude(d,a) << ','
                << wave.curr()->position.longitude(d,a) << ','
                << wave.curr()->position.altitude(d,a) << ','
                << de << "," << az << ","
                << wave.curr()->surface(d,a) << ','
                << wave.curr()->bottom(d,a) << ','
                << wave.curr()->caustic(d,a) << ','
                << wave.curr()->position.rho(d,a) << ','
                << wave.curr()->position.theta(d,a) << ','
                << wave.curr()->position.phi(d,a) << ','
                << wave.curr()->pos_gradient.rho(d,a) << ','
                << wave.curr()->pos_gradient.theta(d,a) << ','
                << wave.curr()->pos_gradient.phi(d,a) << ','
                << wave.curr()->ndirection.rho(d,a) << ','
                << wave.curr()->ndirection.theta(d,a) << ','
                << wave.curr()->ndirection.phi(d,a) << ','
                << wave.curr()->ndir_gradient.rho(d,a) << ','
                << wave.curr()->ndir_gradient.theta(d,a) << ','
                << wave.curr()->ndir_gradient.phi(d,a) << ','
                << wave.curr()->sound_speed(d,a) << ','
                << wave.curr()->sound_gradient.rho(d,a) ;
        	*os[d][a] << endl ;
        	(*os[d][a]).flush() ;
        }
    }

    // propagate wavefront

    while ( wave.time() < time_max ) {

        // move wavefront to next time step

        wave.step() ;

        // write to spreadsheet file

    	for ( unsigned d=0 ; d < de.size() ; ++d ) {
    	    for ( unsigned a=0 ; a < az.size() ; ++a ) {
                wvector1 ndir( wave.curr()->ndirection, d, a ) ;
                double de, az ;
                ndir.direction( &de, &az ) ;

                *os[d][a] << wave.time() << ','
                    << wave.curr()->position.latitude(d,a) << ','
                    << wave.curr()->position.longitude(d,a) << ','
                    << wave.curr()->position.altitude(d,a) << ','
                    << de << "," << az << ","
                    << wave.curr()->surface(d,a) << ','
                    << wave.curr()->bottom(d,a) << ','
                    << wave.curr()->position.rho(d,a) << ','
                    << wave.curr()->position.theta(d,a) << ','
                    << wave.curr()->position.phi(d,a) << ','
                    << wave.curr()->pos_gradient.rho(d,a) << ','
                    << wave.curr()->pos_gradient.theta(d,a) << ','
                    << wave.curr()->pos_gradient.phi(d,a) << ','
                    << wave.curr()->ndirection.rho(d,a) << ','
                    << wave.curr()->ndirection.theta(d,a) << ','
                    << wave.curr()->ndirection.phi(d,a) << ','
                    << wave.curr()->ndir_gradient.rho(d,a) << ','
                    << wave.curr()->ndir_gradient.theta(d,a) << ','
                    << wave.curr()->ndir_gradient.phi(d,a) << ','
                    << wave.curr()->sound_speed(d,a) << ','
                    << wave.curr()->sound_gradient.rho(d,a) ;
            	*os[d][a] << endl ;
            	(*os[d][a]).flush() ;
            }
        }

    }
    cout << "wave propagated for " << wave.time() << " secs" << endl ;

    for ( unsigned d=0 ; d < de.size() ; ++d ) {
	for ( unsigned a=0 ; a < az.size() ; ++a ) {
    	    delete os[d][a] ;
	}
	delete[] os[d] ;
    }
    delete[] os ;
}
