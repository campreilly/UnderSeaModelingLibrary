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
#include <sys/time.h>

using namespace usml::waveq3d ;
using namespace usml::netcdf ;

/**
 * Command line interface.
 */
int main( int argc, char* argv[] ) {
    cout << "=== cmp_speed ===" << endl ;

    int num_targets = 100;
    if ( argc > 1 ) {
        num_targets = atoi( argv[1] ) ;
    }

    // define scenario parameters

    seq_linear freq( 250.0, 1.0, 1 ) ;
    wposition::compute_earth_radius( 19.52 ) ;
    wposition1 src_pos( 19.52, -160.5, -200.0 ) ;
    seq_rayfan de( -90.0, 90.0, 181 ) ;
    seq_linear az( 0.0, 15.0, 360.0 ) ;
//    seq_linear de( -90.0, 1.0, -90.0 ) ;
//    seq_linear az( 0.0, 1.0, 0.0 ) ;
    const double target_depth = 100.0; // Meters
    const double target_range = 100000.0; // Meters
    const double time_max = 80.0 ;
//    const double time_max = 0.0 ;
    const double time_step = 0.100 ;

    // load STD14 environmental data from netCDF files

    const double lat1 = 16.2 ;
    const double lat2 = 24.6 ;
    const double lng1 = -164.4;
    const double lng2 = -155.5 ;

    cout << "load STD14 environmental profile data" << endl ;
//    profile_model* profile = new profile_grid<double,3>( new netcdf_profile(
//            USML_STUDIES_DIR "/cmp_speed/std14profile.nc", 0.0, lat1, lat2, lng1, lng2,
//            wposition::earth_radius ) ) ;
        //fast_grid_3d
    data_grid<double,3>* ssp = new netcdf_profile( USML_STUDIES_DIR "/cmp_speed/std14profile.nc",
            0.0, lat1, lat2, lng1, lng2, wposition::earth_radius ) ;
    data_grid_svp* fast_ssp = new data_grid_svp(*ssp,true) ;
    delete ssp ;
    profile_model* profile = new profile_grid_fast( fast_ssp ) ;
//  attenuation_model* attn = new attenuation_constant(0.0);
//  profile_model* profile = new profile_linear(1500.0,attn);

    cout << "load STD14 environmental bathy data" << endl ;
//    boundary_model* bottom = new boundary_grid<double,2>( new netcdf_bathy(
//            USML_STUDIES_DIR "/cmp_speed/std14bathy.nc", lat1, lat2, lng1, lng2,
//            wposition::earth_radius ) ) ;
        //fast_grid_2d
    data_grid<double,2>* grid = new netcdf_bathy( USML_STUDIES_DIR "/cmp_speed/std14bathy.nc",
        lat1, lat2, lng1, lng2, wposition::earth_radius );
    data_grid_bathy* fast_grid = new data_grid_bathy(*grid, true) ;
    delete grid ;
    boundary_model* bottom = new boundary_grid_fast( fast_grid ) ;
//    boundary_model* bottom = new boundary_flat(4000.0) ;
//    bottom->reflect_loss(new reflect_loss_constant(0.0)) ;

//    bottom->reflect_loss(new reflect_loss_rayleigh(reflect_loss_rayleigh::MUD));

//// Bathy Testing
//    wposition1 seaPoint( 22.332, -162.615 ) ;
//    double rho;
//    bottom->height(seaPoint, &rho);
//    cout << "height  " <<  wposition::earth_radius - rho << endl;

    boundary_model* surface = new boundary_flat() ;

//	  bottom->reflect_loss(new reflect_loss_constant(0.0));  // Total Reflection
//    boundary_model* bottom = new boundary_flat(3000.0);    // Flat Bottom

    ocean_model ocean( surface, bottom, profile ) ;

    // initialize proploss targets and wavefront

    cout << "initialize " << num_targets << " targets" << endl ;

    wposition target( num_targets, 1, src_pos.latitude(), src_pos.longitude(), target_depth ) ;
    // build a series of targets at 100 km
    double angle = TWO_PI/num_targets;
    double bearing_inc = angle;
    for (unsigned n = 0; n < num_targets; ++n) {
        wposition1 aTarget( src_pos, target_range, bearing_inc) ;
        target.latitude( n, 0, aTarget.latitude());
        target.longitude( n, 0, aTarget.longitude());
        target.altitude( n, 0, aTarget.altitude());
        bearing_inc = bearing_inc + angle;
    }

    proploss loss(freq, src_pos, de, az, time_step, &target);
    wave_queue wave( ocean, freq, src_pos, de, az, time_step, &target ) ;
//    wave_queue wave( ocean, freq, src_pos, de, az, time_step ) ;

    if (!wave.addProplossListener(&loss)) {
    	cout << "Error adding proploss listener! " << endl ;
    	exit(1);
    }

    // propagate wavefront

    cout << "propagate wavefronts for " << time_max << " secs" << endl ;
    struct timeval time ;
    struct timezone zone ;
    gettimeofday( &time, &zone ) ;
    double start = time.tv_sec + time.tv_usec * 1e-6 ;
    while ( wave.time() < time_max ) {
        wave.step() ;
    }
    gettimeofday( &time, &zone ) ;
    double complete = time.tv_sec + time.tv_usec * 1e-6 ;
    cout << "Propagating for " << time_max << " sec with "
         << ( target.size1() * target.size2() ) << " targets took "
         << (complete-start) << " sec."
         << endl ;
}
