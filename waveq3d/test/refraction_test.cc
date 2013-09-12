/**
 * @example waveq3d/test/refraction_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <usml/waveq3d/waveq3d.h>
#include <usml/types/seq_vector.h>
#include <iostream>
#include <iomanip>
#include <fstream>

BOOST_AUTO_TEST_SUITE(refraction_test)

using namespace boost::unit_test;
using namespace usml::waveq3d;

/**
 * @ingroup waveq3d_test
 * @{
 */

static const double time_step = 0.1 ;
static const seq_log freq(10e3, 10e3, 1);

/**
 * In this test, an isovelocity ocean (1500 m/s) is used to verify the
 * accuracy of those terms the ray equations that do not depend on the
 * gradient of the sound speed.  The factors of "r" in these equations are
 * all on the order of the radius of Earth, which is over six million meters.
 * Prior to conducting this test, it was unclear if double precision numbers
 * would be accurate enough to compute small differences in positions that
 * included such large numbers.
 *
 * Four horizontal rays were launched from 45N 45W, at a depth of 1000 meters,
 * with azimuths of 0, 30, 60, and 90 degrees, and a time step of 100
 * milliseconds.  In this scenario, the depth of each ray appears to curve
 * toward the ocean surface and strike it 75 seconds later, at a range of
 * around 110 km.  But, in reality, the rays are actually traveling in a
 * straight line; the earth's surface is curving down to meet it.
 *
 * The accuracy of the ray path locations were computed by converting the
 * latitude, longitude, and altitude of each ray back into geocentric
 * (Cartesian) coordinates and then measuring the distance from the analytic
 * form of straight line, as a function of time, in the geocentric system.
 * The equations for this comparison are
 *  * \f[
 *          x(t) = r(t) \sin{\theta(t)} \cos{\phi(t)} ,
 * \f]\f[
 *          y(t) = r(t) \sin{\theta(t)} \sin{\phi(t)} ,
 * \f]\f[
 *          z(t) = r(t) \cos{\theta(t)} ,
 * \f]\f[
 *          x_a(t) = x(0) + n_x(0) \: c t ,
 * \f]\f[
 *          y_a(t) = y(0) + n_y(0) \: c t ,
 * \f]\f[
 *          z_a(t) = z(0) + n_z(0) \: c t ,
 * \f]\f[
 * n_x = -\sin{\phi(0)} \sin{\varphi(0)} - \cos{\theta(0)} \cos{\phi(0)} \cos{\varphi(0)}
 * \f]\f[
 * n_y = \cos{\phi(0)} \sin{\varphi(0)} - \cos{\theta(0)} \sin{\phi(0)} \cos{\varphi(0)}
 * \f]\f[
 * n_z = \sin{\theta(0)} \cos{\varphi(0)}
 * \f]
 * where
 *      - \f$(x(t), y(t), z(t))\f$ are the geocentric coordinates of the
 *        modeled ray path as a function of time,
 *      - \f$(x_a(t), y_a(t), z_a(t))\f$ are the geocentric coordinates
 *        of the analytic ray path as a function of time, and
 *      - \f$(n_x(0), n_y(0), n_z(0))\f$ are the initial direction in
 *        geocentric coordinates.
 *
 * The difference between the altitude predicted by the model and the analytic
 * solution were also computed using
 * \f[
 * r_a(t) = r(0) sec \left[ arctan \left( \frac{c t}{r(0)} \right) \right] \:.
 * \f]
 * The maximum deviation of any modeled ray from the analytic result
 * is required to be less than 1 millimeter.  If this result is true,
 * we'll conclude that double precision numbers are accurate enough to
 * compute ray paths in spherical Earth coordinates.  A BOOST error
 * is thrown if this condition fails.
 *
 * @xref S.M. Reilly, M.S. Goodrich, "Geodetic Acoustic Rays in the
 * Time Domain, Comprehensive Test Results", Alion Science and Technology,
 * Norfolk, VA, September, 2006.
 */
BOOST_AUTO_TEST_CASE(refraction_isovelocity) {
    cout << "=== refraction_test: refraction_isovelocity ===" << endl;

    // initialize propagation model

    const double c0 = 1500.0;
    profile_model* profile = new profile_linear(c0);
    boundary_model* surface = new boundary_flat();
    boundary_model* bottom = new boundary_flat(5000.0);
    ocean_model ocean(surface, bottom, profile);

    wposition1 pos(45.0, -45.0, -1000.0);
    seq_linear de(0.0, 1.0, 1); // along the tangent plane
    seq_linear az(0.0, 30.0, 90.0); // due north, east, and in between

    wave_queue wave(ocean, freq, pos, de, az, time_step);
    cout << "time step = " << time_step << " secs"
         << " freq = " << freq(0) << " Hz" << endl;

    // compute initial position in cartesian coordinates

    double rho = pos.rho();
    double st = sin(pos.theta());
    double ct = cos(pos.theta());
    double sp = sin(pos.phi());
    double cp = cos(pos.phi());

    const double x0 = rho * st * cp;
    const double y0 = rho * st * sp;
    const double z0 = rho * ct;

    // compute initial direction in cartesian coordinates.

    vector<double> east = az;
    east = sin(to_radians(east));
    vector<double> north = az;
    north = cos(to_radians(north));

    // assume initial "up" coordinate is always zero

    vector<double> x_dir = -sp * east - ct * cp * north;
    vector<double> y_dir = cp * east - ct * sp * north;
    vector<double> z_dir = st * north;

    // initialize analytic solution for depth vs. time

    const double r0 = rho;

    // initialize output to spreadsheet file

    const char* name = USML_TEST_DIR "/waveq3d/test/refraction_isovelocity.csv";
    std::ofstream os(name);
    cout << "writing tables to " << name << endl;
    os << "time,x,y,z,d";
    for (unsigned n = 0; n < az.size(); ++n) {
        os << ",x" << az(n)
           << ",y" << az(n)
           << ",z" << az(n)
           << ",d" << az(n);
    }
    os << endl;

    // compute modeled and analytic depth until surface is hit

    double max_error = 0.0;
    while (wave.curr()->position.altitude(0,0) < -10.0) {
        double time = wave.time();

        // compute analytic solution for (x,y,z) vs. time

        vector<double> x;
        x = x0 + c0 * time * x_dir;
        vector<double> y;
        y = y0 + c0 * time * y_dir;
        vector<double> z;
        z = z0 + c0 * time * z_dir;

        // compute analytic solution for depth vs. time

        double d_analytic = r0 / cos(atan(c0 * time / r0))
                - wposition::earth_radius;

        // check answer and write to spreadsheet file

        os << time << "," << x(0) << "," << y(0) << "," << z(0)
                << "," << d_analytic;
        for (unsigned n = 0; n < az.size(); ++n) {

            // decode model's solution for (x,y,z) vs. time

            rho = wave.curr()->position.rho(0, n);
            st = sin(wave.curr()->position.theta(0, n));
            ct = cos(wave.curr()->position.theta(0, n));
            sp = sin(wave.curr()->position.phi(0, n));
            cp = cos(wave.curr()->position.phi(0, n));

            double x_model = rho * st * cp;
            double y_model = rho * st * sp;
            double z_model = rho * ct;

            double dx = x_model - x(n);
            max_error = max(max_error, fabs(dx));
            double dy = y_model - y(n);
            max_error = max(max_error, fabs(dy));
            double dz = z_model - z(n);
            max_error = max(max_error, fabs(dz));

            os << "," << x_model << "," << y_model << "," << z_model;
            BOOST_CHECK(fabs(dx) < 1e-3);
            BOOST_CHECK(fabs(dy) < 1e-3);
            BOOST_CHECK(fabs(dz) < 1e-3);

            // decode model's solution for depth vs. time

            double alt_model = wave.curr()->position.altitude(0, n);
            double dd = alt_model - d_analytic;
            max_error = max(max_error, fabs(dd));
            os << "," << alt_model;
            BOOST_CHECK(fabs(dd) < 1e-3);

        }
        os << endl;

        // move wavefront to next time step

        wave.step();
    }
    cout << "wave breaks surface around " << wave.time() << " secs" << endl
         << "max error = " << max_error << " meters" << endl;
}

/**
 * In this test, an ocean with a small amount of downward refraction is used
 * to verify the model's ability to follow great circle routes along the
 * earth's surface.  Great circle routes, the shortest distance between two
 * points along the Earth's surface, are frequently used in aviation and
 * other long distance navigation problems.  The amount downward refraction
 * needed to test this feature is computed in terms of a "flat Earth"
 * correction given by
 * \f[
 *          c(r) = \frac{r}{R} \: c(z)
 * \f]
 * where
 *      - \f$c(z)\f$ is the speed of sound as a function of depth on a flat
 *        Earth (constant for this test);
 *      - \f$r\f$ is the radial distance from the center of Earth;
 *      - \f$R\f$ is the radius of Earth's curvature in this area of operations; and
 *      - \f$c(r)\f$ is the speed of sound as a function of radial distance
 *        from the center of Earth.
 *
 * Four horizontal rays are launched from 45N 45W, at a depth of 1000 meters,
 * with azimuths of 0, 30, 60, and 90 degrees, for 1000 seconds, with
 * a time step of 100 milliseconds.  The accuracy of the greate circle routes
 * is computed by converting the latitude, longitude, and altitude of each
 * ray back into a great circle azimuth at the point of origin
 * \f[
 *      \varphi_a(t) = arctan \left[
 *	\frac{  \cos{\chi(t)} \sin(\phi(0)-\phi(t))}{ \cos{\chi(0)} \sin{\chi(t)}
 *      - \sin{\chi(0)} \cos{\chi(t)} \cos(\phi(0)-\phi(t)) } \right]
 * \f]
 * where
 *      - \f$(\chi(t), \phi(t))\f$ are latitude and longitude as a function of time, and
 *      - \f$\varphi_a(t)\f$ is the analytic great circle azimuth for a target
 *        at \f$(\chi(t), \phi(t))\f$.
 *
 * The maximum deviation of any modeled ray from the analytic result
 * is required to be less than 1 millimeter and 0.001 degree.  If this
 * result is true, we'll conclude that the model accurately follows
 * great circle routes.  A BOOST error is thrown if this condition fails.
 *
 * @xref S.M. Reilly, M.S. Goodrich, "Geodetic Acoustic Rays in the
 * Time Domain, Comprehensive Test Results", Alion Science and Technology,
 * Norfolk, VA, September, 2006.
 * @xref E. Williams, "Aviation Formulary V1.43",
 * http://williams.best.vwh.net/avform.htm , July 2010.
 */
BOOST_AUTO_TEST_CASE(refraction_great_circle) {
    cout << "=== refraction_test: refraction_great_circle ===" << endl;

    // initialize propagation model

    const double c0 = 1500.0;
    profile_model* profile = new profile_linear(c0);
    boundary_model* surface = new boundary_flat();
    boundary_model* bottom = new boundary_flat(5000.0);
    ocean_model ocean(surface, bottom, profile);
    profile->flat_earth(true);

    double lat1 = 45.0;
    double lng1 = -45.0;
    double alt1 = -1000.0;

    wposition1 pos(lat1, lng1, alt1);
    seq_linear de(0.0, 1.0, 1);
    seq_linear az(0.0, 30.0, 90.0); // due north, east, and in between

    wave_queue wave(ocean, freq, pos, de, az, time_step);
    cout << "time step = " << time_step << " secs"
         << " freq = " << freq(0) << " Hz" << endl;

    // initialize output to spreadsheet file

    const char* name = USML_TEST_DIR "/waveq3d/test/refraction_great_circle.csv";
    std::ofstream os(name);
    cout << "writing tables to " << name << endl;
    os << "time";
    for (unsigned n = 0; n < az.size(); ++n) {
        os << ",lat" << az(n) << ",long" << az(n)
           << ",dalt" << az(n) << ",dbear" << az(n);
    }
    os << endl;

    // compute modeled and analytic depth until surface is hit

    lat1 *= M_PI / 180;
    lng1 *= M_PI / 180;
    double max_d_alt = 0.0;
    double max_d_tc1 = 0.0;

    while (wave.time() < 1000.0) {

        // move wavefront to next time step

        wave.step();
        os << wave.time();

        // check answer and write to spreadsheet file

        for (unsigned n = 0; n < az.size(); ++n) {

            // compute altitude: is it constant?

            double alt2 = wave.curr()->position.altitude(0, n);
            double d_alt = alt2 - alt1;
            max_d_alt = max(max_d_alt, fabs(d_alt));

            // compute great circle bearing to origin: is it constant?

            double lat2 = wave.curr()->position.latitude(0, n)
                    * M_PI / 180;
            double lng2 = wave.curr()->position.longitude(0, n)
                    * M_PI / 180;

            double tc1 = -atan2(sin(lng1 - lng2) * cos(lat2),
                    cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(lng1 - lng2))
                    * 180 / M_PI;
            double d_tc1 = tc1 - az(n);
            max_d_tc1 = max(max_d_tc1, fabs(d_tc1));

            os << "," << wave.curr()->position.latitude(0, n)
               << "," << wave.curr()->position.longitude(0, n)
               << "," << d_alt << "," << d_tc1;
            BOOST_CHECK(fabs(d_alt) < 1e-3);
            BOOST_CHECK(fabs(d_tc1) < 1e-3);
        }
        os << endl;
    }
    cout << "wave propagates for " << wave.time() << " secs" << endl
         << "max error = " << max_d_alt << " meters and "
         << max_d_tc1 << " degrees" << endl;

}

/**
 * Evaluates the model's ray refraction accuracy using a comparison to the
 * analytic solution for a linear profile. Match conditions of the test
 * and output table format to those in the Reilly/Goodrich report.
 * <pre>
 *      Ocean:          Linear: c0 = 1500 m/s, g0 = 1.6e-2 m/s/m
 *                      The profile_model::flat_earth() option turned on.
 *      Position:       1000 meters deep at 00:00N 00:00W
 *      D/E Angles:     0 degrees (horizontal)
 *      AZ Angles:      90 degrees (east)
 *      Time Step:      100 msec
 * </pre>
 * In Cartesian coordinates, the analytic solution for this profile
 * is defined by the equation:
 * \f[
 *      z(r) = \frac{1}{g_0} \left[ \frac{1}{a_0}
 *             \sqrt{ 1.0 - ( a_0 g_0 r + sin(\delta_0) )^2 }
 *             -c_0 \right]
 * \f]
 * An exception is thrown if the modeled position is not within 1 millimeter
 * of the analytic result.
 *
 * @xref S.M. Reilly, M.S. Goodrich, "Geodetic Acoustic Rays in the
 * Time Domain, Comprehensive Test Results", Alion Science and Technology,
 * Norfolk, VA, September, 2006.
 */
BOOST_AUTO_TEST_CASE(refraction_linear) {
    cout << "=== refraction_test: refraction_linear ===" << endl;

    // initialize propagation model

    const double angle = 0.0;
    const double ang = to_radians( angle );
    const double z0 = 1000.0;
    const double c0 = 1500.0;
    const double g0 = 0.016;
    const double a0 = cos(ang) / (c0 + g0 * z0);
    const double sinT = sin(ang);

    profile_model* profile = new profile_linear(c0, g0);
    boundary_model* surface = new boundary_flat();
    boundary_model* bottom = new boundary_flat(5000.0);
    ocean_model ocean(surface, bottom, profile);
    profile->flat_earth(true);

    wposition1 pos(0.0, 0.0, -z0);
    seq_linear de(angle, 0.0, 1);
    seq_linear az(90.0, 0.0, 1);

    wave_queue wave(ocean, freq, pos, de, az, time_step);
    cout << "time step = " << time_step << " secs"
         << " freq = " << freq(0) << " Hz" << endl;

    // initialize output to spreadsheet file

    const char* name = USML_TEST_DIR "/waveq3d/test/refraction_linear.csv";
    std::ofstream os(name);
    cout << "writing tables to " << name << endl;

    os << "t," << "r," << "theta," << "phi," << "z," << "rng(m),"
       << "rd," << "thd," << "phid,"
       << "alpha," << "beta," << "gamma,"
       << "alphad," << "betad," << "gammad,"
       << "c," << "dcdz,"
       << endl;
    os << std::scientific << std::showpoint << std::setprecision(18);

    // compute modeled and analytic depth until surface is hit

    wposition prev(1, 1);
    prev.rho(0, 0, pos.rho());
    prev.theta(0, 0, pos.theta());
    prev.phi(0, 0, pos.phi());

    double max_error = 0.0;
    double r = 0.0;
    double z = -z0;

    while (wave.time() < 9.0) {

        // compute analytic solution for depth vs. range

        r += 0.5 * (wave.curr()->position.rho(0,0) + prev.rho(0,0))
                * (wave.curr()->position.phi(0,0) - prev.phi(0,0));
        double agr = a0 * g0 * r + sinT;
        z = -(sqrt(1.0 - agr * agr) / a0 - c0) / g0;
        prev = wave.curr()->position;

        // write to spreadsheet file

        os << wave.time() << ','
           << wave.curr()->position.rho(0,0) << ','
           << wave.curr()->position.theta(0,0) << ','
           << wave.curr()->position.phi(0,0) << ','
           << z << ',' << r << ','
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
           << wave.curr()->sound_gradient.rho(0,0) << endl;

        // check answer

        double d_alt = fabs(wave.curr()->position.altitude(0,0) - z);
        max_error = max(max_error, d_alt);
        BOOST_CHECK(d_alt < 1e-3);

        // move wavefront to next time step

        wave.step();
    }
    cout << "max error = " << max_error << " meters" << endl;

}

/**
 * Evaluates the model's ray refraction accuracy using a comparison to the
 * analytic solution for a profile where the square of the index of refraction
 * is linear. Match conditions of the test and output table format to those in
 * the Reilly/Goodrich report.  These are also the specific conditions used by
 * Jensen, Kupperman, et. al. in their formulation of the analytic
 * ray tracing solution for the N^2 linear profile.
 * <pre>
 *      Ocean:          N^2 Linear: c0 = 1550 m/s, g0 = 1.2 1/sec
 *                      The profile_model::flat_earth() option turned on.
 *      Position:       1000 meters deep at 00:00N 00:00W
 *      D/E Angles:     50 degrees (up)
 *      AZ Angles:      90 degrees (east)
 *      Time Step:      100 msec
 * </pre>
 * In Cartesian coordinates, the analytic solution for this profile
 * is defined by the equation:
 * \f[
 *      z(r) = z_0 + \frac{ 2*g_0/c_0 }{ (2 a_0 c_1)^2 } r^2 -r tan(\delta_0)
 * \f]
 * where:
 *      - \f$ r \f$ = horizontal range
 *      - \f$ z(r) \f$ = depth as a function of range
 *      - \f$ z_0 \f$ = initial depth
 *      - \f$ \delta_0 \f$ = launch angle
 *      - \f$ c_1 \f$ = sound speed at initial depth
 *      - \f$ g_0 \f$ =  sound speed gradient at z=0.
 *      - \f$ a_0 \f$ = Snell coefficient = \f$ cos(\delta_0)/c_0 \f$
 *
 * An exception is thrown if the modeled position is not within 200 millimeters
 * of the analytic result.  Note that this threshold is much different
 * than the linear case where the maximum error is significantly
 * less than 1 millimeter.  The maximum error shrinks to about 14 millimeters
 * if a step size of 10 milliseconds is used.  It appears that the larger
 * step size undersamples the sound velocity profile field.
 *
 * @xref S.M. Reilly, M.S. Goodrich, "Geodetic Acoustic Rays in the
 * Time Domain, Comprehensive Test Results", Alion Science and Technology,
 * Norfolk, VA, September, 2006.
 * @xref F.B. Jensen, W.A. Kuperman, M.B. Porter, H. Schmidt,
 * "Computational Ocean Acoustics", equation (3.128).
 */
BOOST_AUTO_TEST_CASE(refraction_n2_linear) {
    cout << "=== refraction_test: refraction_n2_linear ===" << endl;

    // initialize propagation model

    const double angle = 50.0;
    const double ang = to_radians( angle ) ;
    const double z0 = 1000.0;
    const double c0 = 1550.0;
    const double g0 = 1.2 ;
    const double a0 = cos(ang) / (c0 / sqrt(1.0 + 2*g0/c0 * z0));

    profile_model* profile = new profile_n2(c0, g0) ;
    boundary_model* surface = new boundary_flat();
    boundary_model* bottom = new boundary_flat(5000.0);
    ocean_model ocean(surface, bottom, profile);
    profile->flat_earth(true);

    wposition1 pos(0.0, 0.0, -z0);
    seq_linear de(angle, 0.0, 1);
    seq_linear az(90.0, 0.0, 1);

    wave_queue wave(ocean, freq, pos, de, az, time_step);
    cout << "time step = " << time_step << " secs"
         << " freq = " << freq(0) << endl;

    // initialize output to spreadsheet file

    const char* name = USML_TEST_DIR "/waveq3d/test/refraction_n2_linear.csv";
    std::ofstream os(name);
    cout << "writing tables to " << name << endl;

    os << "t," << "r," << "theta," << "phi," << "z," << "rng(m),"
       << "rd," << "thd," << "phid,"
       << "alpha," << "beta," << "gamma,"
       << "alphad," << "betad," << "gammad,"
       << "c," << "dcdz,"
       << endl;
    os << std::scientific << std::showpoint << std::setprecision(18);

    // compute modeled and analytic depth until max range hit

    wposition prev(1, 1);
    prev.rho(0, 0, pos.rho());
    prev.theta(0, 0, pos.theta());
    prev.phi(0, 0, pos.phi());

    double max_error = 0.0;
    double r = 0.0;
    double z = 0.0;

    while (z > -2000.0) {
        double time = wave.time();

        // compute analytic solution for depth vs. range

        r += 0.5 * (wave.curr()->position.rho(0,0) + prev.rho(0,0))
                * (wave.curr()->position.phi(0,0) - prev.phi(0,0));
        z = -(2*g0/c0 / (4.0 * a0 * a0 * c0 * c0) * r * r
                - r * tan(ang) + z0);
        prev = wave.curr()->position;

        // write to spreadsheet file

        os << time << ','
           << wave.curr()->position.rho(0,0) << ','
           << wave.curr()->position.theta(0,0) << ','
           << wave.curr()->position.phi(0,0) << ','
           << z << ',' << r << ','
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
           << wave.curr()->sound_gradient.rho(0,0) << endl;

        // check answer

        double d_alt = fabs(wave.curr()->position.altitude(0,0) - z);
        max_error = max(max_error, d_alt);
        BOOST_CHECK_SMALL( wave.curr()->position.altitude(0,0)-z, 2.0 ) ;
        // cout << "r=" << r << " z=" << z << " d_alt=" << d_alt << endl ;

        // move wavefront to next time step

        wave.step();
    }
    cout << "wave reaches " << z << " m depth around "
         << wave.time() << " secs" << endl
         << "max error = " << max_error << " meters" << endl;
}

/**
 * Evaluates the model's ray refraction accuracy using a comparison to the
 * a deep sound channel with a hyperbolic cosine profile. Match conditions
 * of the test and output table format to those in the Reilly/Goodrich report.
 * <pre>
 *      Ocean:          Catenary: c1 = 1500 m/s, g1 = 1500 m/s
 *                      The profile_model::flat_earth() option turned on.
 *      Position:       1000 meters deep at 00:00N 00:00W
 *      D/E Angles:     -3 degrees (down)
 *      AZ Angles:      90 degrees (east)
 *      Time Step:      100 msec
 * </pre>
 * This test is designed to isolate the effects of the radial component
 * of the sound speed gradient.
 * \f[
 *      z(r) = z_0 + c_1 arccosh \left[ sqrt \left( \frac{
 *             sin^2(\delta_0) cos\left( \frac{2r}{c_1} \right)-1 }
 *             {2 cos^2(\delta_0)} \right) \right]
 * \f]
 * where:
 *      - \f$ r \f$ = horizontal range
 *      - \f$ z(r) \f$ = depth as a function of range
 *      - \f$ z_0 \f$ = depth of sound channel axis
 *      - \f$ c_1 \f$ = sound speed at sound channel axis
 *      - \f$ \delta_0 \f$ = launch angle
 *
 * An exception is thrown if the modeled position is not within 2 meters
 * of the analytic result.  Note that this threshold is much different
 * than the linear case where the maximum error is significantly
 * less than 1 millimeter.  The catenary error oscillates between
 * positive and negative and grows with increasing range.  For a -3 degree
 * launch angle, the error has grown to +/- 2 meters by the time the
 * ray reach a range of 100,000 meters.  The maximum error
 * shrinks to about 1.9 millimeters if a step size of 10 milliseconds is
 * used.  It appears that the larger step size undersamples the sound
 * velocity profile field.
 *
 * @xref S.M. Reilly, M.S. Goodrich, "Geodetic Acoustic Rays in the
 * Time Domain, Comprehensive Test Results", Alion Science and Technology,
 * Norfolk, VA, September, 2006.
 */
BOOST_AUTO_TEST_CASE(refraction_catenary) {
    cout << "=== refraction_test: refraction_catenary ===" << endl;

    // initialize propagation model

    const double angle = -3.0;
    const double ang = to_radians( angle );
    const double z0 = 1000.0;
    const double c1 = 1500.0;
    const double g1 = 1500.0;
    const double sinT2 = sin(ang) * sin(ang);
    const double cosT2 = cos(ang) * cos(ang);

    profile_model* profile = new profile_catenary(c1, g1, z0);
    boundary_model* surface = new boundary_flat();
    boundary_model* bottom = new boundary_flat(5000.0);
    ocean_model ocean(surface, bottom, profile);
    profile->flat_earth(true);

    wposition1 pos(0.0, 0.0, -z0);
    seq_linear de(angle, 0.0, 1);
    seq_linear az(90.0, 0.0, 1);

    wave_queue wave(ocean, freq, pos, de, az, time_step);
    cout << "time step = " << time_step << " secs"
         << " freq = " << freq(0) << " Hz" << endl;

    wposition prev(1, 1);
    prev.rho(0, 0, pos.rho());
    prev.theta(0, 0, pos.theta());
    prev.phi(0, 0, pos.phi());

    // initialize output to spreadsheet file

    const char* name = USML_TEST_DIR "/waveq3d/test/refraction_catenary.csv";
    std::ofstream os(name);
    cout << "writing tables to " << name << endl;

    os << "t," << "r," << "theta," << "phi," << "z," << "rng(m),"
       << "rd," << "thd," << "phid,"
       << "alpha," << "beta," << "gamma,"
       << "alphad," << "betad," << "gammad,"
       << "c," << "dcdz,"
       << endl;
    os << std::scientific << std::showpoint << std::setprecision(18);

    // compute modeled and analytic depth until surface is hit

    double max_error = 0.0;
    double r = 0.0;
    int n = 1;
    double sign = -1.0;
    while (r < 100000) {
        double time = wave.time();

        // compute analytic solution for depth vs. range

        r += 0.5 * (wave.curr()->position.rho(0,0) + prev.rho(0,0))
                * (wave.curr()->position.phi(0,0) - prev.phi(0,0));
        double t = 2.0 * r / c1;
        if (t >= TWO_PI * n) {
            ++n;
            sign *= -1.0;
        }
        double s = -0.5 * ((sinT2 * cos(t) - 1.0) / cosT2 - 1.0);
        double z = -(z0 - sign * c1 * acosh(sqrt(s)));
        prev = wave.curr()->position;

        // write to spreadsheet file

        os << time << ','
           << wave.curr()->position.rho(0,0) << ','
           << wave.curr()->position.theta(0,0) << ','
           << wave.curr()->position.phi(0,0) << ','
           << z << ',' << r << ','
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
           << wave.curr()->sound_gradient.rho(0,0) << endl;

        // check answer

        double d_alt = fabs(wave.curr()->position.altitude(0,0) - z);
        max_error = max(max_error, d_alt);
        BOOST_CHECK_SMALL( d_alt, 2.0 );
        // cout << "r=" << r << " t=" << t << " s=" << s << " z=" << z << " d_alt=" << d_alt << endl ;

        // move wavefront to next time step

        wave.step();
    }
    cout << "wave propagates for " << wave.time() << " secs" << endl
         << "max error = " << max_error << " meters" << endl;
}

/**
 * Compares modeled ray paths to an analytic solution for the Munk profile.
 * The profile_munk model creates an idealized representation
 * of a deep sound (SOFAR) channel.
 * <pre>
 *      Ocean:          Munk Profile: z1=1300, B=1300, c1=1500, e=7.37e-3
 *                      The profile_model::flat_earth() option turned on.
 *      Position:       1300 meters deep at 45:00N 45:00W
 *      D/E Angles:     -14 to 14 degrees (avoids surface relfection)
 *      AZ Angles:      0 degrees (north)
 *      Time Step:      100 msec
 * </pre>
 * In this test, rays are launched from the deep sound channel axis,
 * pass through two vertices, and then return to the axis (heading in it's
 * original direction). The process is then repeated for a second cycle.
 * The range required to complete each cycle is compared to an analytic
 * solution computed using the munk_range_compute.m routine.
 *
 * Munk derives a power series expression for the cycle range in his
 * 1974 paper. But, it appears that the number of terms in the expansion
 * are too small to model the range accurately enough for this test.
 * Jensen, Kuperman, et. al. use this scenario to look for false caustics
 * in the ray trace plot, with an emphasis toward looking problems in
 * the cubic spline interpolators.  The parameters for this test were selected
 * to match Figure 3.19 in Jensen, Kuperman, et. al. which are actually
 * defined in the KRAKEN Normal Mode Program documentation.
 *
 * @xref W. H. Munk, "Sound channel in an exponentially stratified ocean,
 * with application to SOFAR", J. Acoust. Soc. Amer. (55) (1974) pp. 220-226.
 * @xref F.B. Jensen, W.A. Kuperman, M.B. Porter, H. Schmidt,
 * "Computational Ocean Acoustics", Section 3.6.4.
 * @xref M.B. Porter, "The KRAKEN Normal Mode Program (DRAFT)",
 * Section called "A Deep Water Problem: the Munk Profile", Oct 1997
 * http://oalib.hlsresearch.com/Modes/AcousticsToolbox/manual_html/node8.html
 */
BOOST_AUTO_TEST_CASE(refraction_munk_range) {
    cout << "=== refraction_test: refraction_munk_range ===" << endl;
    const char* ncname_wave = USML_TEST_DIR "/waveq3d/test/refraction_munk_range.nc";
    const char* name = USML_TEST_DIR "/waveq3d/test/refraction_munk_range.csv";

    // analytic solution for cycle ranges for angles -14:14 degrees;
    // computed using the munk_range_compute.m routine

    static const double cycle_ranges[] = {
	64977.771509,
	62686.699943,
	60536.790347,
	58539.834823,
	56706.277890,
	55044.418981,
	53559.948084,
	52255.876772,
	51132.827760,
	50189.572079,
	49423.683193,
	48832.195747,
	48412.185973,
	48161.238557,
	48077.771909,
	48161.238557,
	48412.185973,
	48832.195747,
	49423.683193,
	50189.572079,
	51132.827760,
	52255.876772,
	53559.948084,
	55044.418981,
	56706.277890,
	58539.834823,
	60536.790347,
	62686.699943,
	64977.771509
    } ;

    // initialize propagation model

    profile_munk* profile = new profile_munk();
    boundary_model* surface = new boundary_flat();
    boundary_model* bottom = new boundary_flat(1e4); // infinitely deep
    ocean_model ocean(surface, bottom, profile);
    profile->flat_earth(true);

    double lat1 = 45.0;
    double lng1 = -45.0;
    double alt1 = -1000.0 ;

    wposition1 pos(lat1, lng1, alt1);
    seq_linear de(-14.0,1.0,14.0);
    seq_linear az(0.0, 0.0, 1);

    wave_queue wave(ocean, freq, pos, de, az, time_step);
    cout << "time step = " << time_step << " secs"
         << " freq = " << freq(0) << " Hz" << endl;

    // initialize output to spreadsheet file

    std::ofstream os(name);
    cout << "writing error data to " << name << endl;

    os << "t," << "de," << "Rtheory," << "Rtheory," << "diff," << "diff%,"
       << endl;
    os << std::scientific << std::showpoint << std::setprecision(18);

    // compute wavefronts and write them to disk
    // storing wavefronts allow us to compare ray trace plots to Jensen, et. al.

    cout << "writing wavefronts to " << ncname_wave << endl;
    wave.init_netcdf(ncname_wave);  // open a log file for wavefront data
    wave.save_netcdf();             // write ray data to log file

    vector<double> loop( de.size() ) ;
    loop.clear() ;

    double max_error = 0.0 ;
    while (wave.time() < 95.0) {

        // increment wavefront by one time step

        wave.step();
        wave.save_netcdf();         // write ray data to log file

        // compare to analytic solution if crossing axis

        for ( unsigned d=0 ; d < de.size() ; ++d ) {

            // look for axis crossings in same direction as launch angle
            // this is how we recognize that one cycle is complete

            const double Hprev = wave.prev()->position.altitude(d,0) - alt1 ;
            const double Hcurr = wave.curr()->position.altitude(d,0) - alt1 ;
            const double Hnext = wave.next()->position.altitude(d,0) - alt1 ;
            if ( Hcurr*Hnext < 0.0 &&
                 wave.curr()->ndirection.rho(d,0) * de(d) > 0.0 )
            {
                // find the analytic result for this launch angle

                loop(d) += 1.0 ;
                const double Rtheory = loop(d) * cycle_ranges[d] ;

                // quadratic interpolation of wavefront range

                const double Rprev = wave.prev()->position.rho(d,0)
                    * to_radians(wave.prev()->position.latitude(d,0)-lat1) ;
                const double Rcurr = wave.curr()->position.rho(d,0)
                    * to_radians(wave.curr()->position.latitude(d,0)-lat1) ;
                const double Rnext = wave.next()->position.rho(d,0)
                    * to_radians(wave.next()->position.latitude(d,0)-lat1) ;

                double h = (Hnext-Hprev) ;
                const double slope = (Rnext-Rprev) / h  ;
                h *= 0.5 ;
                const double curve = (Rnext-2.0*Rcurr+Rprev) / (h*h)  ;
                const double dx = -Hcurr ;
                const double Rmodel = Rcurr + slope*dx + 0.5*curve*dx*dx ;

                // store error to spreadsheet file

                os << wave.time()
                   << "," << de(d) << "," << Rtheory << "," << Rmodel
                   << "," << (Rmodel-Rtheory)
                   << "," << ((Rmodel-Rtheory)/Rtheory*100.0)
                   << endl ;
                BOOST_CHECK_CLOSE( Rtheory, Rmodel, 0.01 );
                max_error = max( max_error, abs(Rmodel-Rtheory) ) ;
            }
        }
    }
    wave.close_netcdf();            // close log file for wavefront data
    cout << "max error = " << max_error << " m" << endl ;
}

/**
 * Compares modeled ray paths to an analytic solution for the Pedersen profile.
 * The profile_pedersen model creates an idealized representation
 * of a deep sound (SOFAR) channel.
 * <pre>
 *      Ocean:          N^2 Linear Profile: c0=1550, g0=1.2
 *                      The profile_model::flat_earth() option turned on.
 *      Position:       1000 meters deep at 45:00N 45:00W
 *      D/E Angles:     20 to 50 degrees (avoids surface reflection)
 *      AZ Angles:      0 degrees (north)
 *      Time Step:      100 msec
 * </pre>
 * In this test, rays are launched from a deep source, they pass through
 * one vertex, and then return to the axis. The range required to complete
 * one cycle is compared to an analytic solution computed using the
 * pedersen_range_compute.m routine.
 *
 * The profile and source depth parameters were choosen to force the
 * creation of a caustic for launch angles > 44 deg. As Pedersen notes,
 * this profile is not physcially realistic at depths greater than 61 meters.
 * But it has been used by many authors, including those listed in the
 * references, to stress propagation loss models at the edge of a shadow zone.
 *
 * @xref M. A. Pedersen, D. F. Gordon, "Normal-Mode and Ray Theory Applied
 * to Underwater Acoustic conditions of Extreme Downward Refraction",
 * J. Acoust. Soc. Am. 51 (1B), 323-368 (June 1972).
 * @xref H. Weinberg, R. E. Keenan, Gaussian ray bundles for modeling high-frequency
 * propagation loss under shallow-water conditions,
 * J. Acoust. Soc. Amer. 100 (1996) 1421.
 * @xref M. B. Porter, H. P. Bucker, Gaussian beam tracing for computing
 * ocean acoustic fields, J. Acoust. Soc. Amer. 93 (1987) 1349.
 */
BOOST_AUTO_TEST_CASE(refraction_pedersen_range) {
    cout << "=== refraction_test: refraction_pedersen_range ===" << endl;
    const char* ncname_wave = USML_TEST_DIR "/waveq3d/test/refraction_pedersen_range.nc";
    const char* name = USML_TEST_DIR "/waveq3d/test/refraction_pedersen_range.csv";

    // analytic solution for cycle ranges for angles 20:2:50 degrees;
    // computed using the pedersen_range_compute.m routine

    static const double cycle_ranges[] = {
	2115.799965,
	2286.528610,
	2446.115431,
	2593.782977,
	2728.811865,
	2850.544337,
	2958.387468,
	3051.815999,
	3130.374953,
	3193.681828,
	3241.428421,
	3273.382397,
	3289.388370,
	3289.368623,
	3273.323545,
	3241.331594,
    } ;

    // initialize propagation model

    const double c0 = 1550 ;
    const double g0 = 1.2 ;
    profile_model* profile = new profile_n2( c0, g0 );
    boundary_model* surface = new boundary_flat();
    boundary_model* bottom = new boundary_flat(1e4); // infinitely deep
    ocean_model ocean(surface, bottom, profile);
    profile->flat_earth(true);

    double lat1 = 45.0;
    double lng1 = -45.0;
    double alt1 = -1000.0 ;

    wposition1 pos(lat1, lng1, alt1);
    seq_linear de(20.0,2.0,50.0);
    seq_linear az(0.0, 0.0, 1);

    wave_queue wave(ocean, freq, pos, de, az, time_step);
    cout << "time step = " << time_step << " secs"
         << " freq = " << freq(0) << " Hz" << endl;

    // initialize output to spreadsheet file

    std::ofstream os(name);
    cout << "writing error data to " << name << endl;

    os << "t," << "de," << "Rtheory," << "Rtheory," << "diff," << "diff%,"
       << endl;
    os << std::scientific << std::showpoint << std::setprecision(18);

    // compute wavefronts and write them to disk
    // storing wavefronts allow us to compare ray trace plots to Jensen, et. al.

    cout << "writing wavefronts to " << ncname_wave << endl;
    wave.init_netcdf(ncname_wave);  // open a log file for wavefront data
    wave.save_netcdf();             // write ray data to log file

    double max_error = 0.0 ;
    while (wave.time() < 4.0) {

        // increment wavefront by one time step

        wave.step();
        wave.save_netcdf();         // write ray data to log file

        // compare to analytic solution if crossing axis

        for ( unsigned d=0 ; d < de.size() ; ++d ) {

            // look for axis crossings
            // this is how we recognize that one cycle is complete

            const double Hprev = wave.prev()->position.altitude(d,0) - alt1 ;
            const double Hcurr = wave.curr()->position.altitude(d,0) - alt1 ;
            const double Hnext = wave.next()->position.altitude(d,0) - alt1 ;
            if ( Hcurr*Hnext < 0.0 ) {

                // find the analytic result for this launch angle

                const double Rtheory = cycle_ranges[d] ;

                // quadratic interpolation of wavefront range

                const double Rprev = wave.prev()->position.rho(d,0)
                    * to_radians(wave.prev()->position.latitude(d,0)-lat1) ;
                const double Rcurr = wave.curr()->position.rho(d,0)
                    * to_radians(wave.curr()->position.latitude(d,0)-lat1) ;
                const double Rnext = wave.next()->position.rho(d,0)
                    * to_radians(wave.next()->position.latitude(d,0)-lat1) ;

                double h = (Hnext-Hprev) ;
                const double slope = (Rnext-Rprev) / h  ;
                h *= 0.5 ;
                const double curve = (Rnext-2.0*Rcurr+Rprev) / (h*h)  ;
                const double dx = -Hcurr ;
                const double Rmodel = Rcurr + slope*dx + 0.5*curve*dx*dx ;

                // store error to spreadsheet file

                os << wave.time()
                   << "," << de(d) << "," << Rtheory << "," << Rmodel
                   << "," << (Rmodel-Rtheory)
                   << "," << ((Rmodel-Rtheory)/Rtheory*100.0)
                   << endl ;
                BOOST_CHECK_CLOSE( Rtheory, Rmodel, 0.12 );
                max_error = max( max_error, abs(Rmodel-Rtheory) ) ;
            }
        }
    }
    wave.close_netcdf();            // close log file for wavefront data
    cout << "max error = " << max_error << " m" << endl ;
}

BOOST_AUTO_TEST_CASE( surface_duct_test ) {
    cout << "=== refraction_test: surface_duct_test ===" << endl;
    const char* ncname_wave = USML_TEST_DIR "/waveq3d/test/refraction_surface_duct.nc";
    const char* csvname = USML_TEST_DIR "/waveq3d/test/refraction_surface_duct.csv";

    // environmental parameters
    data_grid<double,1>* sound_profile;
    seq_vector* axis[1];
    axis[0] = new seq_linear( wposition::earth_radius, -0.5, 1000 );
    sound_profile = new data_grid<double,1>(axis);
    unsigned index[1];
    for(int i=0; i < axis[0]->size(); ++i){
        index[0] = i;
        double value = 1500.0;
        if( (*axis[0])[i] > wposition::earth_radius-150.0 ) { value+=(0.016*i) ;}
        if( (*axis[0])[i] <= wposition::earth_radius-150.0 && (*axis[0])[i] > wposition::earth_radius-250.0 ) { value-=(0.1*(i-300)-4.8) ;}
        if( (*axis[0])[i] <= wposition::earth_radius-250.0 ) { value-=(0.01*(i-500) + 15.2) ; }
        sound_profile->data(index, value);
    }
    sound_profile->interp_type(0, GRID_INTERP_LINEAR);
    sound_profile->edge_limit(0, true);
    profile_model* profile = new profile_grid<double,1>(sound_profile);

    cout << "writing sound speed profile to " << csvname << endl;
    matrix<double> speed(1,1);
    wposition test(1,1);
    test.latitude(0,0,45.0);
    test.longitude(0,0,-45.0);
    std::ofstream file(csvname);
    file << "depth,speed,interp" << endl;
    for(int j=0; j <axis[0]->size(); ++j){
        index[0] = j;
        test.rho(0,0,(*axis[0])(j));
        profile->sound_speed( test, &speed );
        file << (*axis[0])(j)-wposition::earth_radius << "," << sound_profile->data(index) << "," << speed(0,0) << endl;
    }

//    profile_model* profile = new profile_linear(1500.0,0.016);
    profile->flat_earth(true);
    boundary_model* bottom = new boundary_flat(1e4);
    boundary_model* surface = new boundary_flat();
    ocean_model ocean(surface, bottom, profile);

    // test parameters
    double lat = 45.0;
    double lon = -45.0;
    wposition1 source( lat, lon, -40.0 );
    seq_rayfan de(-10.0, 10.0, 51);
    seq_linear az( 0.0, 0.0, 1 );

    wave_queue wave(ocean, freq, source, de, az, time_step);

    cout << "writing wavefronts to " << ncname_wave << endl;
    wave.init_netcdf(ncname_wave);  // open a log file for wavefront data
    wave.save_netcdf();             // write ray data to log file

    while (wave.time() < 25.0) {

        // increment wavefront by one time step

        wave.step();
        wave.save_netcdf();
    }
    wave.close_netcdf();
}

/// This test is not working as intended and therefore not yet ready
//BOOST_AUTO_TEST_CASE( sloped_bottom_duct_test ) {
//    cout << "=== refreaction_test: sloped_bottom_duct_test ===" << endl;
//    const char* ncname_wave = USML_TEST_DIR "/waveq3d/test/refraction_sloped_bottom_duct.nc";
//
//    // environmental parameters
//    wposition1 slope(-45.0, 45.0, 0.0);
//    profile_model* profile = new profile_linear(1500.0,-0.95);
//    profile->flat_earth(true);
//    boundary_model* bottom = new boundary_slope(slope,1500.0,to_radians(2.86));
//    boundary_model* surface = new boundary_flat();
//    ocean_model ocean(surface, bottom, profile);
//
//    // test parameters
//    double lat = 45.0;
//    double lon = -45.0;
//    wposition1 source( lat, lon, -1495.0 );
//    seq_linear de( -75.0, 5.0, 25.0 );
//    seq_linear az( 0.0, 0.0, 1 );
//
//    wave_queue wave(ocean, freq, source, de, az, time_step);
//
//    cout << "writing wavefronts to " << ncname_wave << endl;
//    wave.init_netcdf(ncname_wave);  // open a log file for wavefront data
//    wave.save_netcdf();             // write ray data to log file
//
//    while (wave.time() < 25.0) {
//
//        // increment wavefront by one time step
//
//        wave.step();
//        wave.save_netcdf();
//    }
//    wave.close_netcdf();
//}

/// @}

BOOST_AUTO_TEST_SUITE_END()
