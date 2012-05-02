/**
 * @example ocean/test/profile_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <usml/netcdf/netcdf_files.h>
#include <usml/ocean/ocean.h>
#include <fstream>

BOOST_AUTO_TEST_SUITE(profile_test)

using namespace boost::unit_test;
using namespace usml::netcdf;
using namespace usml::ocean;

/**
 * @ingroup ocean_test
 * @{
 */

/**
 * Test the basic features of the profile model using
 * a constant profile model.
 * Generate errors if values differ by more that 1E-6 percent.
 */
BOOST_AUTO_TEST_CASE( constant_profile_test ) {
    cout << "=== profile_test: constant_profile_test ===" << endl;
    try {

        // simple values for points and speed

        wposition points(1, 1);
        matrix<double> speed(1, 1);
        wvector gradient(1, 1);

        // compute profile

        profile_linear model;
        model.sound_speed(points, &speed, &gradient);
        cout << "speed:          " << speed << endl;
        cout << "gradient rho:   " << gradient.rho() << endl;
        cout << "gradient theta: " << gradient.theta() << endl;
        cout << "gradient phi:   " << gradient.phi() << endl;

        // check the answer

        BOOST_CHECK_CLOSE(speed(0, 0), 1500.0, 1e-6);
        BOOST_CHECK_CLOSE(gradient.rho(0, 0), 0.0, 1e-6);
        BOOST_CHECK_CLOSE(gradient.theta(0, 0), 0.0, 1e-6);
        BOOST_CHECK_CLOSE(gradient.phi(0, 0), 0.0, 1e-6);
    } catch (std::exception* except) {
        BOOST_ERROR(except->what());
    }
}

/**
 * Write results of analytic test models out to CSV file for later plotting.
 * The plotted models include:
 *
 *      - Constant Profile
 *      - Linear Profile
 *      - Bi-Linear Profile
 *      - Munk Profile
 *      - N^2 Linear Profile
 *      - Catenary Profile
 */
BOOST_AUTO_TEST_CASE( plot_profile_test ) {
    cout << "=== profile_test: plot_profile_test ===" << endl;
    try {
        const char* name = "profile_test.csv" ;
        std::ofstream os(name);
        cout << "writing tables to " << name << endl;

        // simple values for points and speed

        seq_linear depth(0.0, 200.0, 6000.0);
        const int D = depth.size();
        wposition points(1, D);
        for (int d = 0; d < D; ++d) {
            points.altitude(0, d, -depth(d));
        }

        // compute all analytic profiles


        matrix<double> sConstant(1, D);
        wvector gConstant(1, D);
        profile_linear constant;
        constant.sound_speed(points, &sConstant, &gConstant);
        constant.flat_earth(true);

        matrix<double> sLinear(1, D);
        wvector gLinear(1, D);
        profile_linear linear(1500.0, 0.01);
        linear.sound_speed(points, &sLinear, &gLinear);

        matrix<double> sBiLinear(1, D);
        wvector gBiLinear(1, D);
        profile_linear bilinear(1500.0, -0.02, 1300, 0.01);
        bilinear.sound_speed(points, &sBiLinear, &gBiLinear);

        matrix<double> sMunk(1, D);
        wvector gMunk(1, D);
        profile_munk munk;
        munk.sound_speed(points, &sMunk, &gMunk);

        matrix<double> sN2(1, D);
        wvector gN2(1, D);
        profile_n2 n2(1550.0, 2.4 / 1500.0);
        n2.sound_speed(points, &sN2, &gN2);

        matrix<double> sCatenary(1, D);
        wvector gCatenary(1, D);
        profile_catenary catenary(1500.0, 1e4, 1300.0);
        catenary.sound_speed(points, &sCatenary, &gCatenary);

        // save all data to CSV file

        os << "Depth" << ",FlatEarth,Gradient" << ",Linear,Gradient"
        << ",BiLinear,Gradient" << ",Munk,Gradient" << ",N2,Gradient"
        << ",Catenary,Gradient" << endl;

        for (int d = 0; d < D; ++d) {
            os << -depth(d) << "," << sConstant(0, d) << "," << gConstant.rho(
                    0, d) << "," << sLinear(0, d) << "," << gLinear.rho(0, d)
            << "," << sBiLinear(0, d) << "," << gBiLinear.rho(0, d)
            << "," << sMunk(0, d) << "," << gMunk.rho(0, d) << ","
            << sN2(0, d) << "," << gN2.rho(0, d) << "," << sCatenary(0,
                    d) << "," << gCatenary.rho(0, d) << endl;
        }
    } catch (std::exception* except) {
        BOOST_ERROR(except->what());
    }
}

/**
 * Extract Hawaii ocean temperature and salinity from World Ocean Atlas 2005.
 * Compare some of the results to the interactive version at
 * UK National Physical Laboratory, "Technical Guides -
 * Speed of Sound in Sea-Water," interactive website at
 * http://resource.npl.co.uk/acoustics/techguides/soundseawater/.
 *
 * Generate errors if values differ by more that 1E-3 percent.
 */
BOOST_AUTO_TEST_CASE( compute_mackenzie_test ) {
    cout << "=== profile_test: compute_mackenzie_test ===" << endl;
    try {
        int month = 6;
        wposition::compute_earth_radius((18.5 + 22.5) / 2.0);

        // load temperature & salinity data from World Ocean Atlas

        netcdf_woa temperature(
        	USML_DATA_TEMP_SEASON, USML_DATA_TEMP_MONTH,
            month, 18.5, 22.5, 200.5, 205.5 ) ;
        netcdf_woa salinity(
			USML_DATA_SALT_SEASON, USML_DATA_SALT_MONTH,
            month, 18.5, 22.5, 200.5, 205.5 ) ;

        // compute sound speed

        profile_mackenzie<float, 3> profile(temperature, salinity);

        // print results for first lat/long entry

        const char* name = "mackenzie_test.csv" ;
        std::ofstream os(name);
        cout << "writing tables to " << name << endl;

        unsigned index[3];
        index[1] = 0;
        index[2] = 0;

        matrix<double> speed(1, 1);
        wposition location(1, 1);
        location.latitude(0, 0, 18.5);
        location.longitude(0, 0, 200.5);
        wvector gradient(1, 1);

        os << "Depth,Temp,Sal,Speed,Gradient" << endl;

        for (unsigned d = 0; d < temperature.axis(0)->size(); ++d) {
            index[0] = d;
            location.rho(0, 0, (*temperature.axis(0))(d));
            profile.sound_speed(location, &speed, &gradient);
            os << -location.altitude(0, 0) << "," << temperature.data(index)
            << "," << salinity.data(index) << "," << speed(0, 0) << ","
            << -gradient.rho(0, 0) << std::endl;

            // compare to UK National Physical Laboratory software

            switch (d) {
                case 0:     // depth=0 temp=25.8543 sal=34.6954
                BOOST_CHECK_CLOSE( speed(0, 0), 1535.9781, 1e-3 );
                break;
                case 18:    // depth=1000 temp=4.3149 sal=34.5221
                BOOST_CHECK_CLOSE( speed(0, 0), 1483.6464, 1e-3 );
                break;
                case 32:    // depth=5500 temp=1.52455 sal=34.6926
                BOOST_CHECK_CLOSE( speed(0, 0), 1549.9655, 1e-3 );
                break;
                default:
                break;
            };
        }
    } catch (std::exception* except) {
        BOOST_ERROR(except->what());
    }
}

/**
 * Test the basic features of the profile model using
 * a constant profile model.
 * Generate errors if values differ by more that 1E-6 percent.
 */
//BOOST_AUTO_TEST_CASE( ascii_profile_test ) {
//    cout << "=== profile_test: ascii_profile_test ===" << endl;
//    try {
//        ascii_profile_factory::read( USML_ASCII_PROFILE_TEST_DATA ) ;
//
//    } catch (std::exception* except) {
//        BOOST_ERROR(except->what());
//    }
//}


/// @}

BOOST_AUTO_TEST_SUITE_END()
