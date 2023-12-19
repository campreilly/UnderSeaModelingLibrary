/**
 * @example waveq3d/test/reflection_test.cc
 */
#include <usml/netcdf/netcdf_files.h>
#include <usml/ocean/ocean.h>
#include <usml/waveq3d/waveq3d.h>

#include <boost/test/unit_test.hpp>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>

BOOST_AUTO_TEST_SUITE(waveq3d_reflection_test)

using namespace boost;
using namespace unit_test;
using namespace numeric::ublas;
using namespace usml::netcdf;
using namespace usml::waveq3d;

/**
 * @ingroup waveq3d_test
 * @{
 */

// TODO: reflect_flat_test disabled until reverb model delivered

/**
 * Monitor callbacks from reflection model.
 */
class reflection_callback : public reflection_listener {
   private:
    size_t _old_count{0};

   public:
    size_t count{0};
    double time{0};
    size_t de{0};
    size_t az{0};
    double dt{0};
    double grazing{0};
    double speed{0};
    wposition1 position;
    wvector1 ndirection;
    size_t type{0};

    /**
     * Pure virtual method to process reflection notifications
     *
     * @param de            D/E angle index number.
     * @param az            AZ angle index number.
     * @param dt            Offset in time to collision with the boundary
     * @param grazing       The grazing angle at point of impact (rads)
     * @param speed         Speed of sound at the point of collision.
     * @param position      Location at which the collision occurs
     * @param ndirection    Normalized direction at the point of collision.
     * @param type          Interface number for the interface that generated
     *                      for this eigenverb.  See the eigenverb_collection
     *                      class header for documentation on interpreting
     *                      this number. For some layers, you can also use
     */
    virtual void reflect(double time, size_t de, size_t az, double dt,
                         double grazing, double speed,
                         const wposition1& position, const wvector1& ndirection,
                         size_t type) {
        ++count;
        this->time = time;
        this->de = de;
        this->az = az;
        this->dt = dt;
        this->grazing = grazing;
        this->speed = speed;
        this->position = position;
        this->ndirection = ndirection;
        this->type = type;
    }

    /**
     * Check to see if total number of reflections has changed.
     */
    bool check_count() {
        if (_old_count != count) {
            _old_count = count;
            return true;
        }
        return false;
    }
};

/**
 * Bottom and surface reflection in a flat bottomed isovelocity ocean.
 * Constructs a geometry in which the changes in latitude
 * and travel time between bounces can be calculated analytically.
 * The following equations predict the path of a downwardly
 * steered ray, given a \f$ \Delta \theta \f$ = latitude change
 * between the surface and the bottom.
 * \f[
 *      s^2 = R_1^2 + R_2^2 - 2 R_1 R_2 cos( \Delta \theta )
 * \f]\f[
 *      sin(\gamma_s) = \frac{ R_1^2 - R_2^2 + s^2 }
 *                         { 2 R_1 s }
 * \f]\f[
 *      \tau = s / c
 * \f]
 * where:
 *      - \f$ R_1 \f$ = radius to ocean surface
 *      - \f$ R_2 \f$ = radius to ocean bottom
 *      - \f$ R_2 - R_1 \f$ = ocean depth
 *      - \f$ \Delta\theta \f$ = latitude change between
 *        the surface and the bottom.
 *      - \f$ s \f$ = path length from surface to bottom
 *      - \f$ \gamma_s \f$ = grazing angle at surface
 *        = ray launch angle
 *      - \f$ \gamma_b  \f$ = grazing angle at bottom
 *        = \f$ \gamma_s - \Delta\theta \f$
 *      - \f$ c      \f$ = sound speed
 *      - \f$ \tau   \f$ = travel time between the surface and the bottom.
 *
 * Selecting \f$ \gamma \f$ to make the latitude change exactly 0.1 degrees
 * yields the following test values.
 *
 *      - \f$ R_1 \f$ = 6378101.030201019 m
 *      - \f$ R_2 \f$ = \f$ R_1 \f$ - 1000 m
 *      - \f$ \Delta \theta \f$ = 0.1 deg
 *      - \f$ s   \f$ = 11,175.841460125 m
 *      - \f$ \gamma_s \f$ = 5.183617057 deg
 *      - \f$ \gamma_b \f$ = 5.083617057 deg
 *      - \f$ c      \f$ = 1500 m/sec
 *      - \f$ \tau   \f$ = 7.450560973 sec
 *
 * Uses a 100 millisecond time step so that the time and location of the
 * collision can be compared to the analytic values without breaking
 * into the guts of the reflection model.
 */
BOOST_AUTO_TEST_CASE(reflect_flat_test) {
    cout << "=== reflection_test: reflect_flat_test ===" << endl;
    const double src_lat = 45.0;   // default to 45 degrees
    const double time_step = 0.1;  // 100 msec
    const double max_time = 60.0;  // maximum travel time

    // initialize propagation model

    wposition::compute_earth_radius(src_lat);
    profile_model::csptr profile(new profile_linear());
    boundary_model::csptr surface(new boundary_flat());
    boundary_model::csptr bottom(new boundary_flat(1000.0));
    ocean_model::csptr ocean(new ocean_model(surface, bottom, profile));

    seq_vector::csptr freq(new seq_log(10.0, 10.0, 1));
    wposition1 pos(src_lat, -45.0, 0.0);
    seq_vector::csptr de(new seq_linear(-5.183617057, 0.0, 1));
    seq_vector::csptr az(new seq_linear(0.0, 0.0, 1));

    wave_queue wave(ocean, freq, pos, de, az, time_step);
    reflection_callback callback;
    wave.add_reflection_listener(&callback);
    double max_time_error = 0.0;
    double max_lat_error = 0.0;

    // initialize output to spreadsheet file

    const char* name = USML_TEST_DIR "/waveq3d/test/reflect_flat_test.csv";
    std::ofstream os(name);
    cout << "Writing tables to " << name << endl;

    os << "t,"
       << "lat,lng,alt,"
       << "de,az,bot,srf,cst,"
       << "r,theta,phi,"
       << "rd,thd,phid,"
       << "mu,eta,nu,"
       << "mud,etad,nud,"
       << "c,dcdz" << endl;
    os << std::scientific << std::showpoint << std::setprecision(18);

    cout << "time step = " << time_step << " secs" << endl;

    // propagate rays to stimulate bottom and surface reflections

    while (wave.time() < max_time) {
        // write to spreadsheet file

        wvector1 ndir(wave.curr()->ndirection, 0, 0);
        double de, az;
        ndir.direction(&de, &az);

        os << wave.time() << ',' << wave.curr()->position.latitude(0, 0) << ','
           << wave.curr()->position.longitude(0, 0) << ','
           << wave.curr()->position.altitude(0, 0) << ',' << de << "," << az
           << "," << wave.curr()->surface(0, 0) << ','
           << wave.curr()->bottom(0, 0) << ',' << wave.curr()->caustic(0, 0)
           << ',' << wave.curr()->position.rho(0, 0) << ','
           << wave.curr()->position.theta(0, 0) << ','
           << wave.curr()->position.phi(0, 0) << ','
           << wave.curr()->pos_gradient.rho(0, 0) << ','
           << wave.curr()->pos_gradient.theta(0, 0) << ','
           << wave.curr()->pos_gradient.phi(0, 0) << ','
           << wave.curr()->ndirection.rho(0, 0) << ','
           << wave.curr()->ndirection.theta(0, 0) << ','
           << wave.curr()->ndirection.phi(0, 0) << ','
           << wave.curr()->ndir_gradient.rho(0, 0) << ','
           << wave.curr()->ndir_gradient.theta(0, 0) << ','
           << wave.curr()->ndir_gradient.phi(0, 0) << ','
           << wave.curr()->sound_speed(0, 0) << ','
           << wave.curr()->sound_gradient.rho(0, 0) << endl;

        // move wavefront to next time step

        wave.step();

        // check location and time of reflections against analytic result

        if (callback.check_count()) {
            auto count = callback.count;
            double predict_time = count * 7.450560973;
            double current_time = callback.time;
            double predict_lat = 45.0 + count * 0.1;
            double current_lat = callback.position.latitude();

            if (callback.type) {
                cout << "bottom";
            } else {
                cout << "surface";
            }
            cout << " reflection at t=" << current_time
                 << " lat=" << current_lat << endl;

            double time_error = abs(current_time - predict_time);
            max_time_error = max(time_error, max_time_error);
            BOOST_CHECK_SMALL(time_error, 1e-4);

            double lat_error = abs(current_lat - predict_lat);
            max_lat_error = max(lat_error, max_lat_error);
            BOOST_CHECK_SMALL(lat_error, 1e-6);
        }
    }
    cout << "wave propagates for " << wave.time() << " secs" << endl
         << "max_time_error = " << max_time_error << " secs "
         << "max_lat_error = " << max_lat_error << " deg " << endl;
    BOOST_CHECK_GT(max_time_error, 0.0);
}

///**
// * Modified version of the reflect_flat_test() in which the bottom
// * has a 1 degree up-slope in the latitude direction.  This change should
// * create a 2 degree increase in grazing angle for each reflection that
// * hits the bottom.
// */
// BOOST_AUTO_TEST_CASE( reflect_slope_test ) {
//    cout << "=== reflection_test: reflect_slope_test ===" << endl ;
//    try {
//
//        // initialize propagation model
//
//        const double c0 = 1500.0 ;
//        profile_model*  profile = new profile_linear(c0) ;
//        boundary_model* surface = new boundary_flat() ;
//
//        wposition1 slope_ref( 45.1, -45.0, 0.0 ) ;
//        reflect_loss_model* slope_loss = new reflect_loss_rayleigh(
//        reflect_loss_rayleigh::SILT ) ; boundary_model* bottom  = new
//        boundary_slope(
//            slope_ref, 1000.0, to_radians(1.0), 0.0, slope_loss ) ;
//
//        ocean_model ocean( surface, bottom, profile ) ;
//
//        seq_log freq( 10.0, 10.0, 1 ) ;
//        wposition1 pos( 45.0, -45.0, 0.0 ) ;
//        seq_linear de( -5.175034664, 0.0, 1 ) ;  // steer down
//        seq_linear az( 0.0, 0.0, 1 ) ;           // north
//        const double time_step = 0.001 ;
//
//        wave_queue wave( ocean, freq, pos, de, az, time_step ) ;
//
//        // initialize output to spreadsheet file
//
//        const char* name = USML_TEST_DIR
//        "/waveq3d/test/reflect_slope_test.csv" ; std::ofstream os(name) ; cout
//        << "Writing tables to " << name << endl ;
//
//        os << "t,"
//           << "lat,lng,alt,"
//           << "de,az,bot,surf,"
//           << "r,theta,phi,"
//           << "rd,thd,phid,"
//           << "mu,eta,nu,"
//           << "mud,etad,nud,"
//           << "c,dcdz"
//           << endl ;
//        os << std::scientific << std::showpoint << std::setprecision(18) ;
//
//        cout << "time step = " << time_step << " secs" << endl ;
//
//        // propagate rays to stimulate bottom and surface reflections
//
//        int bounce = 0 ;
//        double old_de = de(0) ;
//        while ( wave.time() < 25.0 ) {
//
//            // write to spreadsheet file
//
//            wvector1 ndir( wave.curr()->ndirection, 0, 0 ) ;
//            double de, az ;
//            ndir.direction( &de, &az ) ;
//
//            os << wave.time() << ','
//               << wave.curr()->position.latitude(0,0) << ','
//               << wave.curr()->position.longitude(0,0) << ','
//               << wave.curr()->position.altitude(0,0) << ','
//               << de << "," << az << ","
//               << wave.curr()->surface(0,0) << ','
//               << wave.curr()->bottom(0,0) << ','
//               << wave.curr()->position.rho(0,0) << ','
//               << wave.curr()->position.theta(0,0) << ','
//               << wave.curr()->position.phi(0,0) << ','
//               << wave.curr()->pos_gradient.rho(0,0) << ','
//               << wave.curr()->pos_gradient.theta(0,0) << ','
//               << wave.curr()->pos_gradient.phi(0,0) << ','
//               << wave.curr()->ndirection.rho(0,0) << ','
//               << wave.curr()->ndirection.theta(0,0) << ','
//               << wave.curr()->ndirection.phi(0,0) << ','
//               << wave.curr()->ndir_gradient.rho(0,0) << ','
//               << wave.curr()->ndir_gradient.theta(0,0) << ','
//               << wave.curr()->ndir_gradient.phi(0,0) << ','
//               << wave.curr()->sound_speed(0,0) << ','
//               << wave.curr()->sound_gradient.rho(0,0) << endl ;
//
//            // move wavefront to next time step
//
//            wave.step() ;
//
//            // check angle change for each reflection
//
//            if ( old_de * de < 0.0 ) {
//                ++bounce ;
//
//                if ( old_de < 0.0 ) {
//                    cout << "bottom  reflection at t=" << wave.time()
//                         << " lat=" << wave.curr()->position.latitude(0,0)
//                         << " old de=" << old_de << " new de=" << de
//                         << " diff=" << (de+old_de)
//                         << endl ;
//                    BOOST_CHECK_SMALL( 2.0-(de+old_de), 0.001 ) ;
//                } else {
//                    cout << "surface reflection at t=" << wave.time()
//                         << " lat=" << wave.curr()->position.latitude(0,0)
//                         << " old de=" << old_de << " new de=" << de
//                         << " diff=" << (old_de+de)
//                         << endl ;
//                    BOOST_CHECK_SMALL( old_de+de, 0.001 ) ;
//                }
//            }
//            old_de = de ;
//        }
//        cout << "wave propagates for " << wave.time() << " secs" << endl ;
//
//    } catch ( std::exception* except ) {
//        BOOST_ERROR( except->what() ) ;
//    }
//}
//
///**
// * Reflect rays off of the ETOPO1 bottom near the Malta escarpment.
// * Ray path starts from 35:59N 16:00E, D/E=-20 deg AZ=270 deg.
// * A run for 90 seconds should propagate up the slope, turn around
// * and head back down the slope.  Because of slope features in the
// * latitude direction, the ray does not stay in the east-west plane.
// *
// * When the gcc -ffast-math compiler option is turned off, these results are
// * expected to be accurate to at least 0.09 percent.  With fast-math turned
// on,
// * the accuracy of the lat/long drops to 5e-5 percent, and about 6 meters
// * are lost on the accuracy in depth.
// */
// BOOST_AUTO_TEST_CASE( reflect_grid_test ) {
//    const char* csvname = USML_TEST_DIR "/waveq3d/test/reflect_grid_test.csv"
//    ; const char* ncname = USML_TEST_DIR "/waveq3d/test/reflect_grid_test.nc"
//    ; cout << "=== reflection_test: reflect_grid_test ===" << endl ; try {
//
//        // define scenario parameters
//
//        const double c0 = 1500.0 ;  // speed of sound
//
//        const double lat1 = 35.5 ;  // Mediterranean sea
//        const double lat2 = 36.5 ;  // malta escarpment
//        const double lng1 = 15.25 ; // south-east of Sicily
//        const double lng2 = 16.25 ;
//        wposition::compute_earth_radius( (lat1+lat2)/2.0 ) ;
//
//        wposition1 pos( 35.983333333, 16.0, -10.0 ) ;
//        seq_linear de( -20.0, 1.0, 1 ) ;    // down
//        seq_linear az( 270.0, 1.0, 1 ) ;    // west
//        const double time_step = 0.1 ;
//        const double time_max = 80.0 ;
//
//        seq_log freq( 3000.0, 1.0, 1 ) ;
//
//        // load bathymetry from ETOPO1 database
//
//        cout << "load bathymetry" << endl ;
//        boundary_model* bottom = new boundary_grid<double,2>( new
//        netcdf_bathy(
//    	    USML_DATA_DIR "/bathymetry/ETOPO1_Ice_g_gmt4.grd",
//            lat1, lat2, lng1, lng2 ) ) ;
//
//        // combine sound speed and bathymetry into ocean model
//
//        profile_model*  profile = new profile_linear(c0) ;
//        boundary_model* surface = new boundary_flat() ;
//        ocean_model ocean( surface, bottom, profile ) ;
//
//        // initialize output to spreadsheet file
//
//        std::ofstream os(csvname) ;
//        cout << "writting tables to " << csvname << endl ;
//
//        os << "t,"
//           << "lat,lng,alt,"
//           << "de,az,surf,bot,"
//           << "r,theta,phi,"
//           << "rd,thd,phid,"
//           << "mu,eta,nu,"
//           << "mud,etad,nud,"
//           << "c,dcdz"
//           << endl ;
//        os << std::scientific << std::showpoint << std::setprecision(18) ;
//
//        cout << "time step = " << time_step << " secs" << endl ;
//
//        // propagate rays & record to netCDF file
//
//        wave_queue wave( ocean, freq, pos, de, az, time_step ) ;
//        cout << "Writing wavefronts to " << ncname << endl ;
//        wave.init_netcdf( ncname ) ;
//        wave.save_netcdf() ;
//        while ( wave.time() < time_max ) {
//
//            // move wavefront to next time step
//
//            wave.step() ;
//            wave.save_netcdf() ;
//
//            // write to spreadsheet file
//
//            wvector1 ndir( wave.curr()->ndirection, 0, 0 ) ;
//            double de, az ;
//            ndir.direction( &de, &az ) ;
//
//            os << wave.time() << ','
//               << wave.curr()->position.latitude(0,0) << ','
//               << wave.curr()->position.longitude(0,0) << ','
//               << wave.curr()->position.altitude(0,0) << ','
//               << de << "," << az << ","
//               << wave.curr()->surface(0,0) << ','
//               << wave.curr()->bottom(0,0) << ','
//               << wave.curr()->position.rho(0,0) << ','
//               << wave.curr()->position.theta(0,0) << ','
//               << wave.curr()->position.phi(0,0) << ','
//               << wave.curr()->pos_gradient.rho(0,0) << ','
//               << wave.curr()->pos_gradient.theta(0,0) << ','
//               << wave.curr()->pos_gradient.phi(0,0) << ','
//               << wave.curr()->ndirection.rho(0,0) << ','
//               << wave.curr()->ndirection.theta(0,0) << ','
//               << wave.curr()->ndirection.phi(0,0) << ','
//               << wave.curr()->ndir_gradient.rho(0,0) << ','
//               << wave.curr()->ndir_gradient.theta(0,0) << ','
//               << wave.curr()->ndir_gradient.phi(0,0) << ','
//               << wave.curr()->sound_speed(0,0) << ','
//               << wave.curr()->sound_gradient.rho(0,0) << endl ;
//        }
//        wave.close_netcdf() ;
//        cout << "wave propagates for " << wave.time() << " secs" << endl ;
//
//        // compare to prior runs
//
//        #ifdef __FAST_MATH__
//            const double position_accuracy = 5e-4 ;
//        #else
//            const double position_accuracy = 0.09 ;
//        #endif
//        BOOST_CHECK_CLOSE(
//        wave.curr()->position.latitude(0,0), 36.169253160619995,
//        position_accuracy ) ; BOOST_CHECK_CLOSE(
//        wave.curr()->position.longitude(0,0), 16.012084836798909,
//        position_accuracy ) ;
//
////        #ifdef __FAST_MATH__
////            BOOST_CHECK_SMALL(
/// wave.curr()->position.altitude(0,0)+566.97501238062978, 6.0 ) ; / #else /
/// BOOST_CHECK_SMALL( wave.curr()->position.altitude(0,0)+566.97501238062978,
/// 1e-6 ) ; /        #endif
//
//    } catch ( std::exception* except ) {
//        BOOST_ERROR( except->what() ) ;
//    }
//}
//
// BOOST_AUTO_TEST_CASE( reflect_interp_spd_acc_test ){
//    const char* csvname = USML_TEST_DIR
//    "/waveq3d/test/reflect_interp_test.csv" ; cout << "=== reflection_test:
//    reflect_interp_spd_acc_test ===" << endl ;
//
//    // define scenario parameters
//
//    const double c0 = 1500.0 ;  // speed of sound
//
//    const double lat1 = 35.5 ;  // Mediterranean sea
//    const double lat2 = 36.5 ;  // malta escarpment
//    const double lng1 = 15.25 ; // south-east of Sicily
//    const double lng2 = 16.25 ;
//    wposition::compute_earth_radius( (lat1+lat2)/2.0 ) ;
//
//    wposition1 pos( 35.983333333, 16.0, -10.0 ) ;
//    seq_linear de( -20.0, 1.0, 1 ) ;    // down
//    seq_linear az( 270.0, 1.0, 1 ) ;    // west
//    const double time_step = 0.1 ;
//    const double time_max = 80.0 ;
//
//    seq_log freq( 3000.0, 1.0, 1 ) ;
//
//    // load bathymetry from ETOPO1 database
//
//    cout << "load bathymetry" << endl ;
//    boundary_model* bottom = new boundary_grid<double,2>( new netcdf_bathy(
//        USML_DATA_DIR "/bathymetry/ETOPO1_Ice_g_gmt4.grd",
//        lat1, lat2, lng1, lng2 ) ) ;
//
//    // combine sound speed and bathymetry into ocean model
//
//    profile_model*  profile = new profile_linear(c0) ;
//    boundary_model* surface = new boundary_flat() ;
//    ocean_model ocean( surface, bottom, profile ) ;
//
//    std::ofstream os(csvname) ;
//    cout << "Writing tables to " << csvname << endl ;
//
//    os << "t,"
//       << "lat,lng,alt,"
//       << "de,az,surf,bot,"
//       << "r,theta,phi,"
//       << "rd,thd,phid,"
//       << "mu,eta,nu,"
//       << "mud,etad,nud,"
//       << "c,dcdz"
//       << endl ;
//    os << std::scientific << std::showpoint << std::setprecision(18) ;
//
//    cout << "time step = " << time_step << " secs" << endl ;
//
//    // propagate rays & record to netCDF file
//
//    wave_queue wave( ocean, freq, pos, de, az, time_step ) ;
//    while ( wave.time() < time_max ) {
//        wave.step() ;
//
//        wvector1 ndir( wave.curr()->ndirection, 0, 0 ) ;
//        double de, az ;
//        ndir.direction( &de, &az ) ;
//
//        os << wave.time() << ','
//           << wave.curr()->position.latitude(0,0) << ','
//           << wave.curr()->position.longitude(0,0) << ','
//           << wave.curr()->position.altitude(0,0) << ','
//           << de << "," << az << ","
//           << wave.curr()->surface(0,0) << ','
//           << wave.curr()->bottom(0,0) << ','
//           << wave.curr()->position.rho(0,0) << ','
//           << wave.curr()->position.theta(0,0) << ','
//           << wave.curr()->position.phi(0,0) << ','
//           << wave.curr()->pos_gradient.rho(0,0) << ','
//           << wave.curr()->pos_gradient.theta(0,0) << ','
//           << wave.curr()->pos_gradient.phi(0,0) << ','
//           << wave.curr()->ndirection.rho(0,0) << ','
//           << wave.curr()->ndirection.theta(0,0) << ','
//           << wave.curr()->ndirection.phi(0,0) << ','
//           << wave.curr()->ndir_gradient.rho(0,0) << ','
//           << wave.curr()->ndir_gradient.theta(0,0) << ','
//           << wave.curr()->ndir_gradient.phi(0,0) << ','
//           << wave.curr()->sound_speed(0,0) << ','
//           << wave.curr()->sound_gradient.rho(0,0) << endl ;
//   }
//}
//
//
///**
// *
// */
// BOOST_AUTO_TEST_CASE( bounce_threshold_test ) {
//    cout << "=== reflection_test: bounce_threshold_test ===" << endl ;
//
//    double depth = 1000.0 ;
//    double c0 = 1500.0 ;
//    double dt = 0.1 ;
//    double max_time = 10.0 ;
//
//    seq_linear freq( 900.0, 1.0, 1) ;
//    seq_linear de(-90.0, 1.0, 90.0) ;
//    seq_linear az(0.0, 30.0, 360.0) ;
//    wposition1 src( 0.0, 0.0, -500.0 ) ;
//
//    boundary_model* surface = new boundary_flat() ;
//    boundary_model* bottom = new boundary_flat( depth, new
//    reflect_loss_constant(0.0) ) ; profile_model* profile = new
//    profile_linear( c0 ) ; ocean_model ocean( surface, bottom, profile ) ;
//
//    wposition target(1, 1, 0.01, 0.0, -500.0) ;
//    eigenray_collection loss( freq, src, de, az, dt, &target ) ;
//    eigenverb_collection reverb( 0 ) ;
//    wave_queue wave( ocean, freq, src, de, az, dt, &target ) ;
//    wave.add_eigenray_listener(&loss) ;
//    wave.add_eigenverb_listener(&reverb) ;
//    wave.max_surface(3) ;
//    wave.max_bottom(5) ;
//
//    cout << "propagating for " << max_time << " seconds" << endl ;
//    while ( wave.time() < max_time ) {
//        wave.step() ;
//    }
//
//    eigenray_list* elist = loss.eigenrays(0,0) ;
//    cout << "propagation complete, " << elist->size() << " rays were produced"
//    << endl ; BOOST_FOREACH( eigenray e, *elist ) {
//        BOOST_CHECK( e.bottom <= wave.max_bottom() ) ;
//        BOOST_CHECK( e.surface <= wave.max_surface() ) ;
//    }
//    cout << "Checking eigenerbs for bottom interface" << endl ;
//    eigenverb_list vlist = reverb.eigenverbs( eigenverb::BOTTOM ) ;
//    BOOST_FOREACH( eigenverb v, vlist ) {
//        BOOST_CHECK( v.bottom <= wave.max_bottom() ) ;
//        BOOST_CHECK( v.surface <= wave.max_surface() ) ;
//    }
//    cout << "Checking eigenverbs for surface interface" << endl ;
//    vlist = reverb.eigenverbs( eigenverb::SURFACE ) ;
//    BOOST_FOREACH( eigenverb v, vlist ) {
//        BOOST_CHECK( v.bottom <= wave.max_bottom() ) ;
//        BOOST_CHECK( v.surface <= wave.max_surface() ) ;
//    }
//
//}
/// @}

BOOST_AUTO_TEST_SUITE_END()
