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
#include <usml/wave_q3d/wave_q3d.h>
#include <usml/netcdf/netcdf_files.h>
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace usml::wave_q3d ;
using namespace usml::netcdf ;
using namespace usml::ocean ;

/**
 * Command line interface.
 */
int main( int argc, char* argv[] ) {
    char csvname[80] ;
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
    netcdf_woa temperature(
   		USML_DATA_TEMP_SEASON, USML_DATA_TEMP_MONTH,
        month, lat1, lat2, lng1, lng2 ) ;
    netcdf_woa salinity(
		USML_DATA_SALT_SEASON, USML_DATA_SALT_MONTH,
        month, lat1, lat2, lng1, lng2 ) ;

    // compute sound speed

    cout << "compute sound speed" << endl ;
    profile_model* profile =
    	new profile_mackenzie<float,3>( temperature, salinity ) ;

    // load bathymetry from ETOPO1 database

    cout << "load bathymetry from ETOPO1 database" << endl ;
    boundary_model* bottom = new boundary_grid<float,2>( new netcdf_bathy(
    		USML_DATA_BATHYMETRY, lat1, lat2, lng1, lng2 ) ) ;
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
    	    sprintf( csvname, "malta_rays_%02.0f_%02.0f.csv",
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

            wvector1 ndir( wave.current()->ndirection, d, a ) ;
            double de, az ;
            ndir.direction( &de, &az ) ;

            *os[d][a] << wave.time() << ','
                << wave.current()->position.latitude(d,a) << ','
                << wave.current()->position.longitude(d,a) << ','
                << wave.current()->position.altitude(d,a) << ','
                << de << "," << az << ","
                << wave.current()->surface(d,a) << ','
                << wave.current()->bottom(d,a) << ','
                << wave.current()->caustic(d,a) << ','
                << wave.current()->position.rho(d,a) << ','
                << wave.current()->position.theta(d,a) << ','
                << wave.current()->position.phi(d,a) << ','
                << wave.current()->pos_gradient.rho(d,a) << ','
                << wave.current()->pos_gradient.theta(d,a) << ','
                << wave.current()->pos_gradient.phi(d,a) << ','
                << wave.current()->ndirection.rho(d,a) << ','
                << wave.current()->ndirection.theta(d,a) << ','
                << wave.current()->ndirection.phi(d,a) << ','
                << wave.current()->ndir_gradient.rho(d,a) << ','
                << wave.current()->ndir_gradient.theta(d,a) << ','
                << wave.current()->ndir_gradient.phi(d,a) << ','
                << wave.current()->sound_speed(d,a) << ','
                << wave.current()->sound_gradient.rho(d,a) ;
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
                wvector1 ndir( wave.current()->ndirection, d, a ) ;
                double de, az ;
                ndir.direction( &de, &az ) ;

                *os[d][a] << wave.time() << ','
                    << wave.current()->position.latitude(d,a) << ','
                    << wave.current()->position.longitude(d,a) << ','
                    << wave.current()->position.altitude(d,a) << ','
                    << de << "," << az << ","
                    << wave.current()->surface(d,a) << ','
                    << wave.current()->bottom(d,a) << ','
                    << wave.current()->position.rho(d,a) << ','
                    << wave.current()->position.theta(d,a) << ','
                    << wave.current()->position.phi(d,a) << ','
                    << wave.current()->pos_gradient.rho(d,a) << ','
                    << wave.current()->pos_gradient.theta(d,a) << ','
                    << wave.current()->pos_gradient.phi(d,a) << ','
                    << wave.current()->ndirection.rho(d,a) << ','
                    << wave.current()->ndirection.theta(d,a) << ','
                    << wave.current()->ndirection.phi(d,a) << ','
                    << wave.current()->ndir_gradient.rho(d,a) << ','
                    << wave.current()->ndir_gradient.theta(d,a) << ','
                    << wave.current()->ndir_gradient.phi(d,a) << ','
                    << wave.current()->sound_speed(d,a) << ','
                    << wave.current()->sound_gradient.rho(d,a) ;
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
