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
#include <fstream>
#include <iomanip>

#include "lvcms_waveq3d.h"
#include <usml/waveq3d/waveq3d.h>
#include <usml/netcdf/netcdf_files.h>
#include <usml/ocean/profile_mt.h>
#include <usml/ocean/boundary_mt.h>

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

    try {

		cout << "loading temperature & salinity data from World Ocean Atlas" << endl ;
		netcdf_woa temperature(
				USML_DATA_DIR  "/woa09/temperature_seasonal_1deg.nc",
				USML_DATA_DIR  "/woa09/temperature_monthly_1deg.nc", month, lat1, lat2, lng1, lng2 ) ;
		netcdf_woa salinity(
				USML_DATA_DIR  "/woa09/salinity_seasonal_1deg.nc",
				USML_DATA_DIR  "/woa09/salinity_monthly_1deg.nc", month, lat1, lat2, lng1, lng2 ) ;

		// build sound velocity profile from World Ocean Atlas data
		profile_model* profile = new usml::ocean::profile_mackenzie<double,3>( temperature, salinity );
		profile->attenuation(new attenuation_constant(0.0));
		profile_mt* mt_profile = new profile_mt(profile);


		usml::types::wposition::compute_earth_radius(29.0);

		cout << "loading bathymetry from ETOPO1 database" << endl ;
		netcdf_bathy* pBathymetry = new usml::netcdf::netcdf_bathy(
				USML_DATA_DIR  "/bathymetry/ETOPO1_Ice_g_gmt4.grd",
				lat1, lat2, lng1, lng2, usml::types::wposition::earth_radius);

		boundary_model* bottom = new boundary_grid<double,2>( pBathymetry ) ;
		boundary_mt* mt_bottom = new boundary_mt( bottom );

		// combine sound speed and bathymetry into ocean model
		boundary_model* surface = new boundary_flat() ;
		boundary_mt* mt_surface = new boundary_mt( surface );

		cout << "initialize targets" << endl ;
		wposition1 src_pos( 29.0, -80.0, -90.0 ) ;

		//wposition targets = new wposition( 3, 1, src_pos.latitude(), src_pos.longitude(), src_pos.altitude() ) ;
		wposition targets( 3, 1, src_pos.latitude(), src_pos.longitude(), src_pos.altitude() ) ;


		targets.latitude ( 0, 0,  29.01) ;
		targets.longitude( 0, 0, -80.0 ) ;
		targets.altitude ( 0, 0, -10.0 ) ;

		targets.latitude ( 1, 0,  29.05 ) ;
		targets.longitude( 1, 0, -79.95 ) ;
		targets.altitude ( 1, 0, -100.0 ) ;

		targets.latitude ( 2, 0,  28.95 ) ;
		targets.longitude( 2, 0, -80.05 ) ;
		targets.altitude ( 2, 0, -100.0 ) ;

		ocean_model* ocean = new ocean_model( mt_surface, mt_bottom, mt_profile ) ;


		std::vector<LvcmsWaveQ3D*> lvcmsWaveq3dThreads;

		int totalThreads = 2;

		for (int i=0; i < totalThreads; ++i){

			LvcmsWaveQ3D* waveq3dThread = new LvcmsWaveQ3D();

			//waveq3dThread->PrintSPPToCSV(temperature, salinity, mt_profile, i);

			waveq3dThread->setOcean(ocean);

			waveq3dThread->setThreadNum(i);

			waveq3dThread->setTargets(&targets);

			proploss* loss = new proploss(waveq3dThread->getTargets());

			waveq3dThread->setProploss(loss);

			lvcmsWaveq3dThreads.push_back(waveq3dThread);
		}

		std::vector<LvcmsWaveQ3D*>::iterator iter;
		for ( iter = lvcmsWaveq3dThreads.begin(); iter != lvcmsWaveq3dThreads.end(); ++iter )
		{
			LvcmsWaveQ3D* aThread = *iter;
			aThread->Process();
		}

		while (lvcmsWaveq3dThreads.size() > 0) {
			for ( iter = lvcmsWaveq3dThreads.begin(); iter != lvcmsWaveq3dThreads.end(); ++iter )
			{
				LvcmsWaveQ3D* aThread = *iter;
				if (!aThread->ThreadRunning()) {
					lvcmsWaveq3dThreads.erase(iter);
					usleep(100000);
					break;
				}
			}
			usleep(100000);
		}

    } catch (std::exception& e) {

    	cout << "lvcms_test: " <<  __func__ << ": Error - " << e.what() << endl;
    	return 1;
    }
	return 0;
}
