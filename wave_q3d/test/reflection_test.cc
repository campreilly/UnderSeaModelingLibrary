/**
 * @example wave_q3d/test/reflection_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <usml/netcdf/netcdf_files.h>
#include <usml/wave_q3d/wave_q3d.h>
#include <usml/wave_q3d/reverb_model.h>
#include <iostream>
#include <iomanip>
#include <fstream>

BOOST_AUTO_TEST_SUITE(reflection_test)

using namespace boost::unit_test ;
using namespace usml::netcdf ;
using namespace usml::wave_q3d ;

/**
 * @ingroup wave_q3d_test
 * @{
 */

/**
 * Monitor callbacks from reflection model.
 */
class reflection_callback : public reverb_model {

public:
    int counter ;
    double time ;
    wposition1 position ;
    wvector1 ndirection ;

    /**
     * Initialize counter.
     */
    reflection_callback() : counter(0) {}

    /**
     * Record a collision of a single ray with a reverbation surface.
     */
    virtual void collision(
        unsigned de, unsigned az, double time,
        const wposition1& position, const wvector1& ndirection, double speed,
        const seq_vector& frequencies,
        const vector<double>& amplitude, const vector<double>& phase )
    {
        ++counter ;
        this->time = time ;
        this->position = position ;
        this->ndirection = ndirection ;
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
 * Uses a 1 millisecond time step so that the time and location of the
 * collision can be compared to the analytic valaues without breaking
 * into the guts of the reflection model.
 */
BOOST_AUTO_TEST_CASE( reflect_flat_test ) {
    cout << "=== reflection_test: reflect_flat_test ===" << endl ;
    try {

        // initialize propagation model

        const double c0 = 1500.0 ;
        profile_model*  profile = new profile_linear(c0) ;
        boundary_model* surface = new boundary_flat() ;
        boundary_model* bottom  = new boundary_flat(1000.0) ;
        ocean_model ocean( surface, bottom, profile ) ;

        seq_log freq( 10.0, 10.0, 1 ) ;
        wposition1 pos( 45.0, -45.0, 0.0 ) ;
        seq_linear de( -5.183617057, 0.0, 1 ) ;  // steer down
        seq_linear az( 0.0, 0.0, 1 ) ;           // north
        const double time_step = 0.1 ;

        wave_queue wave( ocean, freq, pos, de, az, time_step ) ;
        reflection_callback callback ;
        wave.set_bottom_reverb( &callback ) ;
        wave.set_surface_reverb( &callback ) ;
        int old_counter = callback.counter ;
        double max_time_error = 0.0 ;
        double max_lat_error = 0.0 ;

        // initialize output to spreadsheet file

        const char* name = "reflect_flat_test.csv" ;
        std::ofstream os(name) ;
        cout << "writting tables to " << name << endl ;

        os << "t,"
           << "lat,lng,alt,"
           << "de,az,bot,srf,cst,"
           << "r,theta,phi,"
           << "rd,thd,phid,"
           << "mu,eta,nu,"
           << "mud,etad,nud,"
           << "c,dcdz"
           << endl ;
        os << std::scientific << std::showpoint << std::setprecision(18) ;

        cout << "time step = " << time_step << " secs" << endl ;

        // propagate rays to stimulate bottom and surface reflections

        int bounce = 0 ;
        while ( wave.time() < 60.0 ) {

            // write to spreadsheet file

            wvector1 ndir( wave.curr()->ndirection, 0, 0 ) ;
            double de, az ;
            ndir.direction( &de, &az ) ;

            os << wave.time() << ','
               << wave.curr()->position.latitude(0,0) << ','
               << wave.curr()->position.longitude(0,0) << ','
               << wave.curr()->position.altitude(0,0) << ','
               << de << "," << az << ","
               << wave.curr()->surface(0,0) << ','
               << wave.curr()->bottom(0,0) << ','
               << wave.curr()->caustic(0,0) << ','
               << wave.curr()->position.rho(0,0) << ','
               << wave.curr()->position.theta(0,0) << ','
               << wave.curr()->position.phi(0,0) << ','
               << wave.curr()->pos_gradient.rho(0,0) << ','
               << wave.curr()->pos_gradient.theta(0,0) << ','
               << wave.curr()->pos_gradient.phi(0,0) << ','
               << wave.curr()->ndirection.rho(0,0) << ','
               << wave.curr()->ndirection.theta(0,0) << ','
               << wave.curr()->ndirection.phi(0,0) << ','
               << wave.curr()->ndir_gradient.rho(0,0) << ','
               << wave.curr()->ndir_gradient.theta(0,0) << ','
               << wave.curr()->ndir_gradient.phi(0,0) << ','
               << wave.curr()->sound_speed(0,0) << ','
               << wave.curr()->sound_gradient.rho(0,0) << endl ;

            // move wavefront to next time step

            wave.step() ;

            // check location and time of reflections against analytic result

            if ( old_counter != callback.counter ) {
                old_counter = callback.counter ;
                ++bounce ;

                double predict_time = bounce * 7.450560973 ;
                double current_time = callback.time ;
                double predict_lat = 45.0 + bounce * 0.1 ;
                double current_lat = callback.position.latitude() ;

                cout << (( callback.ndirection.rho() < 0.0 ) ? "bottom " : "surface")
                     << " reflection at t=" << current_time
                     << " lat=" << current_lat
                     << endl ;

                double time_error = abs( current_time - predict_time ) ;
                max_time_error = max( time_error, max_time_error ) ;
                BOOST_CHECK_SMALL( time_error, 1e-4 ) ;

                double lat_error = abs( current_lat - predict_lat ) ;
                max_lat_error = max( lat_error, max_lat_error ) ;
                BOOST_CHECK_SMALL( lat_error, 1e-6 ) ;
            }
        }
        cout << "wave propagates for " << wave.time() << " secs" << endl
             << "max_time_error = " << max_time_error << " secs "
             << "max_lat_error = " << max_lat_error  << " deg " << endl ;

    } catch ( std::exception* except ) {
        BOOST_ERROR( except->what() ) ;
    }
}

/**
 * Modified version of the reflect_flat_test() in which the bottom
 * has a 1 degree up-slope in the latitude direction.  This change should
 * create a 2 degree increase in grazing angle for each reflection that
 * hits the bottom.
 */
BOOST_AUTO_TEST_CASE( reflect_slope_test ) {
    cout << "=== reflection_test: reflect_slope_test ===" << endl ;
    try {

        // initialize propagation model

        const double c0 = 1500.0 ;
        profile_model*  profile = new profile_linear(c0) ;
        boundary_model* surface = new boundary_flat() ;

        wposition1 slope_ref( 45.1, -45.0, 0.0 ) ;
        boundary_model* bottom  = new boundary_slope(
            slope_ref, 1000.0, to_radians(1.0) ) ;

        ocean_model ocean( surface, bottom, profile ) ;

        seq_log freq( 10.0, 10.0, 1 ) ;
        wposition1 pos( 45.0, -45.0, 0.0 ) ;
        seq_linear de( -5.175034664, 0.0, 1 ) ;  // steer down
        seq_linear az( 0.0, 0.0, 1 ) ;           // north
        const double time_step = 0.001 ;

        wave_queue wave( ocean, freq, pos, de, az, time_step ) ;

        // initialize output to spreadsheet file

        const char* name = "reflect_slope_test.csv" ;
        std::ofstream os(name) ;
        cout << "writting tables to " << name << endl ;

        os << "t,"
           << "lat,lng,alt,"
           << "de,az,bot,surf,"
           << "r,theta,phi,"
           << "rd,thd,phid,"
           << "mu,eta,nu,"
           << "mud,etad,nud,"
           << "c,dcdz"
           << endl ;
        os << std::scientific << std::showpoint << std::setprecision(18) ;

        cout << "time step = " << time_step << " secs" << endl ;

        // propagate rays to stimulate bottom and surface reflections

        int bounce = 0 ;
        double old_de = de(0) ;
        while ( wave.time() < 25.0 ) {

            // write to spreadsheet file

            wvector1 ndir( wave.curr()->ndirection, 0, 0 ) ;
            double de, az ;
            ndir.direction( &de, &az ) ;

            os << wave.time() << ','
               << wave.curr()->position.latitude(0,0) << ','
               << wave.curr()->position.longitude(0,0) << ','
               << wave.curr()->position.altitude(0,0) << ','
               << de << "," << az << ","
               << wave.curr()->surface(0,0) << ','
               << wave.curr()->bottom(0,0) << ','
               << wave.curr()->position.rho(0,0) << ','
               << wave.curr()->position.theta(0,0) << ','
               << wave.curr()->position.phi(0,0) << ','
               << wave.curr()->pos_gradient.rho(0,0) << ','
               << wave.curr()->pos_gradient.theta(0,0) << ','
               << wave.curr()->pos_gradient.phi(0,0) << ','
               << wave.curr()->ndirection.rho(0,0) << ','
               << wave.curr()->ndirection.theta(0,0) << ','
               << wave.curr()->ndirection.phi(0,0) << ','
               << wave.curr()->ndir_gradient.rho(0,0) << ','
               << wave.curr()->ndir_gradient.theta(0,0) << ','
               << wave.curr()->ndir_gradient.phi(0,0) << ','
               << wave.curr()->sound_speed(0,0) << ','
               << wave.curr()->sound_gradient.rho(0,0) << endl ;

            // move wavefront to next time step

            wave.step() ;

            // check angle change for each reflection

            if ( old_de * de < 0.0 ) {
                ++bounce ;

                if ( old_de < 0.0 ) {
                    cout << "bottom  reflection at t=" << wave.time()
                         << " lat=" << wave.curr()->position.latitude(0,0)
                         << " old de=" << old_de << " new de=" << de
                         << " diff=" << (de+old_de)
                         << endl ;
                    BOOST_CHECK_SMALL( 2.0-(de+old_de), 0.001 ) ;
                } else {
                    cout << "surface reflection at t=" << wave.time()
                         << " lat=" << wave.curr()->position.latitude(0,0)
                         << " old de=" << old_de << " new de=" << de
                         << " diff=" << (old_de+de)
                         << endl ;
                    BOOST_CHECK_SMALL( old_de+de, 0.001 ) ;
                }
            }
            old_de = de ;
        }
        cout << "wave propagates for " << wave.time() << " secs" << endl ;

    } catch ( std::exception* except ) {
        BOOST_ERROR( except->what() ) ;
    }
}

/**
 * Reflect rays off of the ETOPO1 bottom near the Malta escarpment.
 * Ray path starts from 35:59N 16:00E, D/E=-20 deg AZ=270 deg.
 * A run for 90 seconds should propagate up the slope, turn around
 * and head back down the slope.  Because of slope features in the
 * latitude direction, the ray does not stay in the east-west plane.
 *
 * When the gcc -ffast-math compiler option is turned off, these results are
 * expected to be accurate to at least 1e-6 percent.  With fast-math turned on,
 * the accuracy of the lat/long drops to 5e-5 percent, and the about 1 meters
 * is lost on the accuracy in depth.
 */
BOOST_AUTO_TEST_CASE( reflect_grid_test ) {
    const char* csvname = "reflect_grid_test.csv" ;
    const char* ncname = "reflect_grid_test.nc" ;
    cout << "=== reflection_test: reflect_grid_test ===" << endl ;
    try {

        // define scenario parameters

        const double c0 = 1500.0 ;  // speed of sound

        const double lat1 = 35.5 ;  // mediterrian sea
        const double lat2 = 36.5 ;  // malta escarpment
        const double lng1 = 15.25 ; // south-east of Sicily
        const double lng2 = 16.25 ;
        wposition::compute_earth_radius( (lat1+lat2)/2.0 ) ;

        wposition1 pos( 35.983333333, 16.0, -10.0 ) ;
        seq_linear de( -20.0, 1.0, 1 ) ;    // down
        seq_linear az( 270.0, 1.0, 1 ) ;    // west
        const double time_step = 0.1 ;
        const double time_max = 80.0 ;

        seq_log freq( 3000.0, 1.0, 1 ) ;

        // load bathymetry from ETOPO1 database

        cout << "load bathymetry" << endl ;
        boundary_model* bottom = new boundary_grid<float,2>( new netcdf_bathy(
            USML_DATA_BATHYMETRY, lat1, lat2, lng1, lng2 ) ) ;

        // combine sound speed and bathymetry into ocean model

        profile_model*  profile = new profile_linear(c0) ;
        boundary_model* surface = new boundary_flat() ;
        ocean_model ocean( surface, bottom, profile ) ;

        // initialize output to spreadsheet file

        std::ofstream os(csvname) ;
        cout << "writting tables to " << csvname << endl ;

        os << "t,"
           << "lat,lng,alt,"
           << "de,az,bot,surf,"
           << "r,theta,phi,"
           << "rd,thd,phid,"
           << "mu,eta,nu,"
           << "mud,etad,nud,"
           << "c,dcdz"
           << endl ;
        os << std::scientific << std::showpoint << std::setprecision(18) ;

        cout << "time step = " << time_step << " secs" << endl ;

        // propagate rays & record to netCDF file

        wave_queue wave( ocean, freq, pos, de, az, time_step ) ;
        cout << "writting wavefronts to " << ncname << endl ;
        wave.init_netcdf( ncname ) ;
        wave.save_netcdf() ;
        while ( wave.time() < time_max ) {

            // move wavefront to next time step

            wave.step() ;
            wave.save_netcdf() ;

            // write to spreadsheet file

            wvector1 ndir( wave.curr()->ndirection, 0, 0 ) ;
            double de, az ;
            ndir.direction( &de, &az ) ;

            os << wave.time() << ','
               << wave.curr()->position.latitude(0,0) << ','
               << wave.curr()->position.longitude(0,0) << ','
               << wave.curr()->position.altitude(0,0) << ','
               << de << "," << az << ","
               << wave.curr()->surface(0,0) << ','
               << wave.curr()->bottom(0,0) << ','
               << wave.curr()->position.rho(0,0) << ','
               << wave.curr()->position.theta(0,0) << ','
               << wave.curr()->position.phi(0,0) << ','
               << wave.curr()->pos_gradient.rho(0,0) << ','
               << wave.curr()->pos_gradient.theta(0,0) << ','
               << wave.curr()->pos_gradient.phi(0,0) << ','
               << wave.curr()->ndirection.rho(0,0) << ','
               << wave.curr()->ndirection.theta(0,0) << ','
               << wave.curr()->ndirection.phi(0,0) << ','
               << wave.curr()->ndir_gradient.rho(0,0) << ','
               << wave.curr()->ndir_gradient.theta(0,0) << ','
               << wave.curr()->ndir_gradient.phi(0,0) << ','
               << wave.curr()->sound_speed(0,0) << ','
               << wave.curr()->sound_gradient.rho(0,0) << endl ;
        }
        wave.close_netcdf() ;
        cout << "wave propagates for " << wave.time() << " secs" << endl ;

        // compare to prior runs

        #ifdef __FAST_MATH__
            const double position_accuracy = 5e-4 ;
        #else
            const double position_accuracy = 1e-6 ;
        #endif
        BOOST_CHECK_CLOSE( wave.curr()->position.latitude(0,0),36.183195549220635, position_accuracy ) ;
        BOOST_CHECK_CLOSE( wave.curr()->position.longitude(0,0),16.021086325519299, position_accuracy ) ;

        #ifdef __FAST_MATH__
            BOOST_CHECK_SMALL( wave.curr()->position.altitude(0,0)+2728.2357222689316, 6.0 ) ;
        #else
            BOOST_CHECK_CLOSE( wave.curr()->position.altitude(0,0),-2728.2357222689316, 1e-6 ) ;
        #endif

    } catch ( std::exception* except ) {
        BOOST_ERROR( except->what() ) ;
    }
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
