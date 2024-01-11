/**
 * @example netcdf/test/read_profile_test.cc
 */
#include <usml/netcdf/netcdf_files.h>

#include <boost/test/unit_test.hpp>
#include <fstream>
#include <iostream>

BOOST_AUTO_TEST_SUITE(read_profile_test)

using namespace boost::unit_test;
using namespace usml::netcdf;

/**
 * @ingroup netcdf_test
 * @{
 */

/**
 * Tests the ability of the netcdf_woa class to splice deep and shallow
 * databases into a combined product. Extract Hawaii ocean temperature and
 * dumps the resulting data to the read_woa.csv file.
 *
 * Demonstrates the ability to modify the earth radius used in data extraction.
 * Also demonstrates the ability to fill missing data.
 *
 * Automatically compares results for a handful of individual location
 * to data that was extracted by hand from the woacmp.log file created
 * by the Makefile. Generates BOOST errors if these values differ by
 * more that 1E-6 percent.
 */
BOOST_AUTO_TEST_CASE(read_woa) {
    cout << "=== read_profile_test: read_woa ===" << endl;
    int month = 6;
    const double old_radius = wposition::earth_radius;
    wposition::earth_radius = 6378137.0;
    netcdf_woa profile(USML_DATA_DIR "/woa09/temperature_seasonal_1deg.nc",
                       USML_DATA_DIR "/woa09/temperature_monthly_1deg.nc",
                       month, 18.5, 22.5, 200.5, 205.5);
    profile.write_netcdf(USML_TEST_DIR "/netcdf/test/read_woa.nc");

    // compare depth axis to values read using ncdump

    const seq_vector& altitude = profile.axis(0);
    size_t num_alt = altitude.size();
    cout << "altitude[" << num_alt << "] =";
    for (size_t n = 0; n < num_alt; ++n) {
        cout << " " << (altitude(n) - wposition::earth_radius);
    }
    cout << endl;
    BOOST_CHECK_EQUAL(num_alt, 33);
    BOOST_CHECK_CLOSE(altitude(0) - wposition::earth_radius, 0.0, 1e-6);
    BOOST_CHECK_CLOSE(altitude(num_alt - 1) - wposition::earth_radius, -5500.0,
                      1e-6);

    // compare latitude axis to values read using ncdump

    const seq_vector& latitude = profile.axis(1);
    size_t num_lat = latitude.size();
    double lat1 = to_latitude(latitude(0));
    double lat2 = to_latitude(latitude(num_lat - 1));
    double inc_lat = to_degrees(latitude.increment(0));
    cout << "latitude[" << num_lat << "] = " << lat1 << " to " << lat2 << " by "
         << inc_lat << endl;
    BOOST_CHECK_EQUAL(num_lat, 5);
    BOOST_CHECK_CLOSE(lat1, 18.5, 1e-6);
    BOOST_CHECK_CLOSE(lat2, 22.5, 1e-6);

    // compare longitude axis to values read using ncdump

    const seq_vector& longitude = profile.axis(2);
    size_t num_lng = longitude.size();
    double lng1 = to_degrees(longitude(0));
    double lng2 = to_degrees(longitude(num_lng - 1));
    double inc_lng = to_degrees(longitude.increment(0));
    cout << "longitude[" << num_lng << "] = " << lng1 << " to " << lng2
         << " by " << inc_lng << endl;
    BOOST_CHECK_EQUAL(num_lng, 6);
    BOOST_CHECK_CLOSE(lng1, 200.5, 1e-6);
    BOOST_CHECK_CLOSE(lng2, 205.5, 1e-6);

    // compare depth to some known values

    size_t index[3];
    index[0] = 0;
    index[1] = 0;
    index[2] = 0;  // 1st depth, 1st lat, 1st long
    BOOST_CHECK_CLOSE(profile.data(index), 25.8543F, 1e-6F);
    index[0] = 0;
    index[1] = 0;
    index[2] = 1;  // 1st depth, 1st lat, 2nd long
    BOOST_CHECK_CLOSE(profile.data(index), 25.7594F, 1e-6F);
    index[0] = 0;
    index[1] = 1;
    index[2] = 0;  // 1st depth, 2nd lat, 1st long
    BOOST_CHECK_CLOSE(profile.data(index), 25.8541F, 1e-6F);
    index[0] = 1;
    index[1] = 0;
    index[2] = 0;  // 2nd depth, 1st lat, 1st long
    BOOST_CHECK_CLOSE(profile.data(index), 25.8575F, 1e-6F);
    index[0] = 32;
    index[1] = 0;
    index[2] = 0;  // last depth, 1st lat, 1st long
    BOOST_CHECK( !std::isnan(profile.data(index)) );

    // dump CSV file for processing by graphics program
    // each row is a depth, cycle through all longitudes
    // before moving to next latitude

    std::ofstream ofile(USML_TEST_DIR "/netcdf/test/read_woa.csv");

    // print header row
    ofile << "depth";
    for (size_t n = 0; n < num_lat; ++n) {
        for (size_t m = 0; m < num_lng; ++m) {
            ofile << ",\"" << to_latitude(latitude(n)) << "N "
                  << to_degrees(longitude(m)) << "E\"";
        }
    }
    ofile << std::endl;

    // print data
    for (size_t d = 0; d < num_alt; ++d) {
        index[0] = d;
        ofile << (altitude(d) - wposition::earth_radius);
        for (size_t n = 0; n < num_lat; ++n) {
            index[1] = n;
            for (size_t m = 0; m < num_lng; ++m) {
                index[2] = m;
                ofile << "," << profile.data(index);
            }
        }
        ofile << std::endl;
    }
    wposition::earth_radius = old_radius;
}

/**
 * Tests the ability of the netcdf_profile class to span a longitude
 * cut point in the database.  To test this, it reads data from WOA09
 * across the area from -1E (aka 1W) to 1E.  It relies on the fact that
 * WOA09 defines its data in terms of longitudes from 0 to 360.
 * Dumps the resulting profiles to the span_profile.csv file.
 *
 * Demonstrates the ability to extract data without converting altitudes
 * to earth-centric coordinates.
 *
 * Automatically compares results for a handful of individual location
 * to data that was extracted by hand from the woacmp2.log file created
 * by the Makefile. Generates BOOST errors if these values differ by
 * more that 1E-6 percent.
 */
BOOST_AUTO_TEST_CASE(span_profile) {
    cout << "=== read_profile_test: span_profile ===" << endl;
    const double old_radius = wposition::earth_radius;
    wposition::earth_radius = 0.0;
    netcdf_profile profile(USML_DATA_DIR "/woa09/temperature_monthly_1deg.nc",
                           15.0, -1.0, 2.0, -1.0, 2.0);

    // compare depth axis to values read using ncdump

    const seq_vector& altitude = profile.axis(0);
    size_t num_alt = altitude.size();
    cout << "altitude = " << altitude << endl;
    BOOST_CHECK_EQUAL(num_alt, 24);
    BOOST_CHECK_CLOSE(altitude(0), 0.0, 1e-6);
    BOOST_CHECK_CLOSE(altitude(num_alt - 1), -1500.0, 1e-6);

    // compare latitude axis to values read using ncdump

    const seq_vector& latitude = profile.axis(1);
    size_t num_lat = latitude.size();
    double lat1 = to_latitude(latitude(0));
    double lat2 = to_latitude(latitude(num_lat - 1));
    double inc_lat = -to_degrees(latitude.increment(0));
    cout << "latitude[" << num_lat << "] = " << lat1 << " to " << lat2 << " by "
         << inc_lat << endl;
    BOOST_CHECK_EQUAL(num_lat, 5);
    BOOST_CHECK_CLOSE(lat1, -1.5, 1e-6);
    BOOST_CHECK_CLOSE(lat2, 2.5, 1e-6);

    // compare longitude axis to values read using ncdump

    const seq_vector& longitude = profile.axis(2);
    size_t num_lng = longitude.size();
    double lng1 = to_degrees(longitude(0));
    double lng2 = to_degrees(longitude(num_lng - 1));
    double inc_lng = to_degrees(longitude.increment(0));
    cout << "longitude[" << num_lng << "] = " << lng1 << " to " << lng2
         << " by " << inc_lng << endl;
    BOOST_CHECK_EQUAL(num_lng, 5);
    BOOST_CHECK_CLOSE(lng1, -1.5, 1e-6);
    BOOST_CHECK_CLOSE(lng2, 2.5, 1e-6);

    // compare temperature to some known values

    size_t index[3];
    index[0] = 0;
    index[1] = 2;
    index[2] = 2;  // zero depth at 0.5N 0.5E
    BOOST_CHECK_CLOSE(profile.data(index), 28.2047F, 1e-6F);
    index[0] = 23;
    index[1] = 2;
    index[2] = 2;  // max depth at 0.5N 0.5E
    BOOST_CHECK_CLOSE(profile.data(index), 4.1774F, 1e-6F);

    // dump CSV file for processing by graphics program
    // each row is a depth, cycle through all longitudes
    // before moving to next latitude

    std::ofstream ofile(USML_TEST_DIR "/netcdf/test/span_profile.csv");

    // print header row
    ofile << "depth";
    for (size_t n = 0; n < num_lat; ++n) {
        for (size_t m = 0; m < num_lng; ++m) {
            ofile << ",\"" << to_latitude(latitude(n)) << "N "
                  << to_degrees(longitude(m)) << "E\"";
        }
    }
    ofile << std::endl;

    // print data
    for (size_t d = 0; d < num_alt; ++d) {
        index[0] = d;
        ofile << altitude(d);
        for (size_t n = 0; n < num_lat; ++n) {
            index[1] = n;
            for (size_t m = 0; m < num_lng; ++m) {
                index[2] = m;
                ofile << "," << profile.data(index);
            }
        }
        ofile << std::endl;
    }
    wposition::earth_radius = old_radius;
}

/**
 * Tests the ability of the netcdf_profile class to extract data
 * from a NetCDF file that does not have a global range. This
 * would in return prevent the data from wrapping around and
 * instead limit the bounds of the data to the database provided.
 */
BOOST_AUTO_TEST_CASE(nonglobal_database) {
    cout << "=== read_profile_test: nonglobal_database ===" << endl;
    netcdf_profile profile(USML_TEST_DIR "/netcdf/test/flstrts_temperature.nc",
                           9, -90.0, 90.0, 0.0, 360.0);

    // compare latitude axis to values read using ncdump

    const seq_vector& latitude = profile.axis(1);
    size_t num_lat = latitude.size();
    double lat1 = to_latitude(latitude(0));
    double lat2 = to_latitude(latitude(num_lat - 1));
    double inc_lat = to_degrees(latitude.increment(0));
    cout << "latitude[" << num_lat << "] = " << lat1 << " to " << lat2 << " by "
         << inc_lat << endl;
    BOOST_CHECK_EQUAL(num_lat, 7);
    BOOST_CHECK_CLOSE(lat1, 27.5, 1e-6);
    BOOST_CHECK_CLOSE(lat2, 33.5, 1e-6);

    // compare longitude axis to values read using ncdump

    const seq_vector& longitude = profile.axis(2);
    size_t num_lng = longitude.size();
    double lng1 = to_degrees(longitude(0));
    double lng2 = to_degrees(longitude(num_lng - 1));
    double inc_lng = to_degrees(longitude.increment(0));
    cout << "longitude[" << num_lng << "] = " << lng1 << " to " << lng2
         << " by " << inc_lng << endl;
    BOOST_CHECK_EQUAL(num_lng, 8);
    BOOST_CHECK_CLOSE(lng1, 278.5, 1e-6);
    BOOST_CHECK_CLOSE(lng2, 285.5, 1e-6);
}

/**
 * Test the ability to load a  3D profile file downloaded from
 * the HYCOM.org web site.
 *
 * https://ncss.hycom.org/thredds/ncss/grid/GLBv0.08/expt_93.0/ts3z/dataset.html
 * - type = NetcdfSubset
 * - latitudes 25.9-27.1
 * - longitudes 279.9-281.1
 * - time 2019-10-18T10:00:00Z
 *
 * These files have a few differences from the other NetCDF files we have
 * worked with in the past.
 *
 * - Both the water_temp and salinity are contained in the same file, so the
 *   variables must be looked up by name.
 * - The water_temp and salinity data are stored as scaled short integers,
 *   where all other file types to date have used floating point values.
 */
BOOST_AUTO_TEST_CASE(hycom_file_profile_test) {
    cout << "=== read_profile_test: hycom_file_profile_test ===" << endl;
    const char* ssp_file = USML_TEST_DIR "/netcdf/test/hycom_ts3z.nc4";
    const char* grid_file = USML_TEST_DIR "/netcdf/test/hycom_grid.nc";
    cout << "opening " << ssp_file << endl;
    netcdf_profile temperature(ssp_file, 0.0, 25.9, 27.2, -80.1, -78.8, "temp");
    temperature.write_netcdf(grid_file);
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
