/**
 * @example ocean/test/profile_test.cc
 */
#include <usml/netcdf/netcdf_woa.h>
#include <usml/ocean/ascii_profile.h>
#include <usml/ocean/data_grid_mackenzie.h>
#include <usml/ocean/profile_catenary.h>
#include <usml/ocean/profile_grid.h>
#include <usml/ocean/profile_linear.h>
#include <usml/ocean/profile_model.h>
#include <usml/ocean/profile_munk.h>
#include <usml/ocean/profile_n2.h>
#include <usml/types/types.h>
#include <usml/ublas/ublas.h>

#include <boost/test/unit_test.hpp>
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
BOOST_AUTO_TEST_CASE(constant_profile_test) {
    cout << "=== profile_test: constant_profile_test ===" << endl;

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
BOOST_AUTO_TEST_CASE(plot_profile_test) {
    cout << "=== profile_test: plot_profile_test ===" << endl;
    const char* name = USML_TEST_DIR "/ocean/test/profile_test.csv";
    std::ofstream os(name);
    cout << "writing tables to " << name << endl;

    // simple values for points and speed

    seq_linear depth(0.0, 200.0, 6000.0);
    const size_t D = depth.size();
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
    // TODO Check to see if earth flattening working correctly. Gradient is all zeros.

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

    os << "Depth"
       << ",FlatEarth,Gradient"
       << ",Linear,Gradient"
       << ",BiLinear,Gradient"
       << ",Munk,Gradient"
       << ",N2,Gradient"
       << ",Catenary,Gradient" << endl;

    for (int d = 0; d < D; ++d) {
        os << -depth(d) << "," << sConstant(0, d) << "," << gConstant.rho(0, d)
           << "," << sLinear(0, d) << "," << gLinear.rho(0, d) << ","
           << sBiLinear(0, d) << "," << gBiLinear.rho(0, d) << ","
           << sMunk(0, d) << "," << gMunk.rho(0, d) << "," << sN2(0, d) << ","
           << gN2.rho(0, d) << "," << sCatenary(0, d) << ","
           << gCatenary.rho(0, d) << endl;
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
BOOST_AUTO_TEST_CASE(compute_mackenzie_test) {
    cout << "=== profile_test: compute_mackenzie_test ===" << endl;
    int month = 6;
    wposition::compute_earth_radius((18.5 + 22.5) / 2.0);

    // load temperature & salinity data from World Ocean Atlas

    auto* temp =
        new netcdf_woa(USML_DATA_DIR "/woa09/temperature_seasonal_1deg.nc",
                       USML_DATA_DIR "/woa09/temperature_monthly_1deg.nc",
                       month, 18.5, 18.5, 200.5, 200.5);
    auto* salt =
        new netcdf_woa(USML_DATA_DIR "/woa09/salinity_seasonal_1deg.nc",
                       USML_DATA_DIR "/woa09/salinity_monthly_1deg.nc", month,
                       18.5, 18.5, 200.5, 200.5);
    for (size_t n = 0; n < 3; ++n) {
        temp->interp_type(n, interp_enum::nearest);
        salt->interp_type(n, interp_enum::nearest);
    }
    netcdf_woa::csptr temperature(temp);
    netcdf_woa::csptr salinity(salt);

    // compute sound speed

    data_grid<3>::csptr grid(new data_grid_mackenzie(temperature, salinity));
    profile_grid<3> profile(grid);

    // print results for first lat/long entry
    // TODO Gradient incorrectly comes out as all zeros.

    const char* name = USML_TEST_DIR "/ocean/test/mackenzie_test.csv";
    std::ofstream os(name);
    os.precision(12);
    cout << "writing tables to " << name << endl;

    size_t index[3];
    index[1] = 0;
    index[2] = 0;

    matrix<double> speed(1, 1);
    wposition location(1, 1);
    location.latitude(0, 0, 18.5);
    location.longitude(0, 0, 200.5);
    wvector gradient(1, 1);

    os << "Depth,Temp,Sal,Speed,Gradient" << endl;
    for (size_t d = 0; d < temperature->axis(0).size(); ++d) {
        index[0] = d;
        location.rho(0, 0, temperature->axis(0)(d));
        profile.sound_speed(location, &speed, &gradient);
        os << -location.altitude(0, 0) << "," << temperature->data(index) << ","
           << salinity->data(index) << "," << speed(0, 0) << ","
           << -gradient.rho(0, 0) << endl;

        // compare to UK National Physical Laboratory software

        switch (d) {
            case 0:  // depth=0 temp=25.8543 salinity=34.6954
                BOOST_CHECK_CLOSE(speed(0, 0), 1535.9781, 1e-3);
                break;
            case 18:  // depth=1000 temp=4.3149 salinity=34.5221
                BOOST_CHECK_CLOSE(speed(0, 0), 1483.6464, 1e-3);
                break;
            case 31:  // depth=5000 temp=1.4465 salinity=34.6968
                BOOST_CHECK_CLOSE(speed(0, 0), 1540.6472, 1e-3);
                break;
            default:
                break;
        };
    }
}

/**
 * Test the ability to load 1D profile data from an ASCII text file.
 *
 * Generate errors if 1st and 8th values differ by more that 1E-5 percent.
 */
BOOST_AUTO_TEST_CASE(ascii_profile_test) {
    cout << "=== profile_test: ascii_profile_test ===" << endl;
    const char* ssp_file = (USML_TEST_DIR "/ocean/test/ascii_profile_test.csv");
    ascii_profile profile(ssp_file);
    const seq_vector& axis = profile.axis(0);
    BOOST_CHECK_EQUAL(axis.size(), 8);
    size_t index[1];
    index[0] = 0;
    double value1 = profile.data(index);
    index[0] = 7;
    double value8 = profile.data(index);
    BOOST_CHECK_CLOSE(value1, 1546.50, 1e-5);
    BOOST_CHECK_CLOSE(value8, 1490.00, 1e-5);
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
