/**
 * @example netcdf/test/read_bathy_test.cc
 */
#include <usml/netcdf/netcdf_files.h>

#include <boost/test/unit_test.hpp>
#include <fstream>
#include <iostream>

BOOST_AUTO_TEST_SUITE(read_bathy_test)

using namespace boost::unit_test;
using namespace usml::netcdf;

/**
 * @ingroup netcdf_test
 * @{
 */

/**
 * This test prints out the dimensions and variables in the netCDF header.
 * It is used as a fundamental test to see if access to netCDF files
 * are working at all.
 */
//NOLINTNEXTLINE(readability-function-cognitive-complexity)
BOOST_AUTO_TEST_CASE(read_bathy_header) {
    cout << "=== read_bathy_test: read_bathy_header ===" << endl;
    static const char* type_name[] = {"unknown", "byte",  "char",  "short",
                                      "int",     "float", "double"};

    NcFile file(USML_DATA_DIR "/bathymetry/ETOPO1_Ice_g_gmt4.grd");
    cout << "netcdf read_bathy_header {" << endl;

    // dimensions

    cout << "dimensions:" << endl;
    for (int d = 0; d < file.num_dims(); ++d) {
        NcDim* dim = file.get_dim(d);
        cout << "\t" << dim->name() << " = " << dim->size() << " ;" << endl;
    }

    // variables

    cout << "variables:" << endl;
    for (int v = 0; v < file.num_vars(); ++v) {
        NcVar* var = file.get_var(v);
        cout << "\t" << type_name[var->type()] << " " << var->name() << "(";

        // variable dimensions

        for (int d = 0; d < var->num_dims(); ++d) {
            NcDim* dim = var->get_dim(d);
            cout << dim->name();
            if (d < var->num_dims() - 1) {
                cout << ", ";
            } else {
                cout << ") ;" << endl;
            }
        }

        // variable attributes

        for (int a = 0; a < var->num_atts(); ++a) {
            NcAtt* att = var->get_att(a);
            NcValues* values = att->values();
            cout << "\t\t" << var->name() << ":" << att->name() << " = ";
            if (att->type() == 2) {
                char* str = values->as_string(0);
                cout << "\"" << str << "\" ;" << endl;
                delete[] str;
            } else {
                for (int v = 0; v < att->num_vals(); ++v) {
                    char* str = values->as_string(v);
                    cout << str;
                    delete[] str;
                    if (v < att->num_vals() - 1) {
                        cout << ", ";
                    } else {
                        cout << " ;" << endl;
                    }
                }
            }
            delete att;
            delete values;
        }
    }

    // data

    cout << "data:" << endl;
    const long N = 10;
    double data[N];
    for (int v = 0; v < file.num_vars(); ++v) {
        NcVar* var = file.get_var(v);
        long D1 = min(N, var->num_vals());
        long D2 = max(0L, var->num_dims() - 1L);
        long D3 = max(0L, var->num_dims() - 2L);
        long D4 = max(0L, var->num_dims() - 3L);
        long D5 = max(0L, var->num_dims() - 4L);
        if (D1 > 0) {
            NcBool status = var->get(data, D1, D2, D3, D4, D5);
            cout << var->name() << " = ";
            if (status != 0U) {
                for (long d = 0; d < D1; ++d) {
                    cout << data[d] << ", ";
                }
                cout << "..." << endl;
            } else {
                cout << "error" << endl;
            }
        }
    }
    cout << "}" << endl;
}

/**
 * Extract Hawaii bathymetry from March 2010 version of ETOPO1
 * using the netcdf_bathy class. Dump the resulting bathymetry to the
 * read_etopo.log file.
 *
 * Automatically compares results for a handfull of individual location
 * to data that was extracted by hand from the etopo_cmp.log file created
 * by the Makefile. Generate BOOST errors if these values differ by
 * more that 1E-6 percent.
 */
BOOST_AUTO_TEST_CASE(read_etopo) {
    cout << "=== read_bathy_test: read_etopo ===" << endl;
    netcdf_bathy bathy(USML_DATA_DIR "/bathymetry/ETOPO1_Ice_g_gmt4.grd", 18.0,
                       23.0, 200.0, 206.0, 0.0);

    // compare latitude axis to values read using ncdump

    const seq_vector& latitude = bathy.axis(0);
    auto num_lat = latitude.size();
    auto lat1 = to_latitude(latitude(0));
    auto lat2 = to_latitude(latitude(num_lat - 1));
    auto inc_lat = to_degrees(-latitude.increment(0));
    cout << "latitude[" << num_lat << "] = " << lat1 << " to " << lat2 << " by "
         << inc_lat << endl;
    BOOST_CHECK_EQUAL(num_lat, 301);
    BOOST_CHECK_CLOSE(lat1, 18.0, 1e-6);
    BOOST_CHECK_CLOSE(lat2, 23.0, 1e-6);

    // compare longitude axis to values read using ncdump

    const seq_vector& longitude = bathy.axis(1);
    auto num_lng = longitude.size();
    auto lng1 = to_degrees(longitude(0));
    auto lng2 = to_degrees(longitude(num_lng - 1));
    auto inc_lng = to_degrees(longitude.increment(0));
    cout << "longitude[" << num_lng << "] = " << lng1 << " to " << lng2
         << " by " << inc_lng << endl;
    BOOST_CHECK_EQUAL(num_lng, 361);
    BOOST_CHECK_CLOSE(lng1, 200.0, 1e-6);
    BOOST_CHECK_CLOSE(lng2, 206.0, 1e-6);

    // compare depth to some known values
    // extracted by hand from etopo_cmp.log

    size_t index[2];
    index[0] = 0;
    index[1] = 0;
    BOOST_CHECK_CLOSE(bathy.data(index), -5262.0F, 1e-6F);
    index[0] = 0;
    index[1] = 1;
    BOOST_CHECK_CLOSE(bathy.data(index), -5272.0F, 1e-6F);
    index[0] = 1;
    index[1] = 0;
    BOOST_CHECK_CLOSE(bathy.data(index), -5249.0F, 1e-6F);

    // dump CSV file for processing by graphics program

    std::ofstream ofile(USML_TEST_DIR "/netcdf/test/read_etopo.csv");
    for (long n = 0; n < num_lat; ++n) {
        index[0] = n;
        for (long m = 0; m < num_lng; ++m) {
            index[1] = m;
            ofile << bathy.data(index);
            if (m < (num_lng - 1)) {
                ofile << ",";
            }
        }
        ofile << std::endl;
    }
}

/**
 * Extract Hawaii bathymetry from the etopo_cmp.nc file created
 * in the Makefile using the ncks tool.  Compare results for individual
 * array indices to equivalent data that extracted from this database
 * by hand from the etopo_cmp.log file created by the Makefile.
 * Throw errors if values differ by more that 1E-6 percent.
 * Comparisons of the log files output by each phase (etopo.log and
 * coards.log) should also show no differences.
 *
 * The input file may be missing if the current platform does not
 * include the ncks utility.  This test quietly exits if
 * etopo_cmp.nc is missing.
 */
BOOST_AUTO_TEST_CASE(read_coards) {
    cout << "=== read_bathy_test: read_coards ===" << endl;
    static const char* filename = USML_TEST_DIR "/netcdf/test/etopo_cmp.nc";
    NcFile file(filename);
    if (file.id() < 0) {
        cout << filename << " not found, test skipped" << endl;
        return;
    }
    cout << "reading " << filename << endl;
    netcdf_coards<2> bathy(file, "z");

    // compare latitude axis to values read using ncdump

    const seq_vector& latitude = bathy.axis(0);
    auto num_lat = latitude.size();
    cout << "latitude[" << num_lat << "] = " << latitude(0) << " to "
         << latitude(num_lat - 1) << " by " << latitude.increment(0) << endl;
    BOOST_CHECK_EQUAL(num_lat, 301);
    BOOST_CHECK_CLOSE(latitude(0), 18.0, 1e-6);
    BOOST_CHECK_CLOSE(latitude(num_lat - 1), 23.0, 1e-6);

    // compare longitude axis to values read using ncdump

    const seq_vector& longitude = bathy.axis(1);
    auto num_lng = longitude.size();
    cout << "longitude[" << num_lng << "] = " << longitude(0) << " to "
         << longitude(num_lng - 1) << " by " << longitude.increment(0) << endl;
    BOOST_CHECK_EQUAL(num_lng, 361);
    BOOST_CHECK_CLOSE(longitude(0), -160.0, 1e-6);
    BOOST_CHECK_CLOSE(longitude(num_lng - 1), -154.0, 1e-6);

    // compare depth to some known values
    // extracted by hand from etopo_cmp.log

    size_t index[2];
    index[0] = 0;
    index[1] = 0;
    BOOST_CHECK_CLOSE(bathy.data(index), -5262.0F, 1e-6F);
    index[0] = 0;
    index[1] = 1;
    BOOST_CHECK_CLOSE(bathy.data(index), -5272.0F, 1e-6F);
    index[0] = 1;
    index[1] = 0;
    BOOST_CHECK_CLOSE(bathy.data(index), -5249.0F, 1e-6F);

    // dump CSV file for processing by graphics program

    std::ofstream ofile(USML_TEST_DIR "/netcdf/test/read_coards.csv");
    for (long n = 0; n < num_lat; ++n) {
        index[0] = n;
        for (long m = 0; m < num_lng; ++m) {
            index[1] = m;
            ofile << bathy.data(index);
            if (m < (num_lng - 1)) {
                ofile << ",";
            }
        }
        ofile << std::endl;
    }
}

/**
 * Tests the ability of the netcdf_bathy class to span a longitude
 * cut point in the database.  To test this, it reads data from ETOPO1
 * across the area from 179E to 181E (aka 179W).  This assumes that
 * ETOPO1 defines its data in terms of longitudes from -180 to 180.
 *
 * Dump the resulting bathymetry to the span_bathy.log file.
 */
BOOST_AUTO_TEST_CASE(span_bathy) {
    cout << "=== read_bathy_test: span_bathy ===" << endl;
    netcdf_bathy bathy(USML_DATA_DIR "/bathymetry/ETOPO1_Ice_g_gmt4.grd", -1.0,
                       2.0, 179, 182, 0.0);

    // compare latitude axis to values read using ncdump

    const seq_vector& latitude = bathy.axis(0);
    auto num_lat = latitude.size();
    auto lat1 = to_latitude(latitude(0));
    auto lat2 = to_latitude(latitude(num_lat - 1));
    auto inc_lat = to_degrees(latitude.increment(0));
    cout << "latitude[" << num_lat << "] = " << lat1 << " to " << lat2 << " by "
         << inc_lat << endl;
    BOOST_CHECK_EQUAL(num_lat, 181);
    BOOST_CHECK_CLOSE(lat1, -1.0, 1e-6);
    BOOST_CHECK_CLOSE(lat2, 2.0, 1e-6);

    // compare longitude axis to values read using ncdump

    const seq_vector& longitude = bathy.axis(1);
    auto num_lng = longitude.size();
    auto lng1 = to_degrees(longitude(0));
    auto lng2 = to_degrees(longitude(num_lng - 1));
    auto inc_lng = to_degrees(longitude.increment(0));
    cout << "longitude[" << num_lng << "] = " << lng1 << " to " << lng2
         << " by " << inc_lng << endl;
    BOOST_CHECK_EQUAL(num_lng, 181);
    BOOST_CHECK_CLOSE(lng1, 179.0, 1e-6);
    BOOST_CHECK_CLOSE(lng2, 182.0, 1e-6);

    // compare depth to some known values
    // extracted by hand from etopo_cmp2w.log

    size_t index[2];
    index[0] = 0;
    index[1] = 0;  // 1S 179E = first entry, first row
    BOOST_CHECK_CLOSE(bathy.data(index), -5436.0F, 1e-6F);
    index[0] = 0;
    index[1] = 60;  // 1S 180E = last entry, first row
    BOOST_CHECK_CLOSE(bathy.data(index), -5249.0F, 1e-6F);
    index[0] = 180;
    index[1] = 0;  // 2N 179E = first entry, last row
    BOOST_CHECK_CLOSE(bathy.data(index), -5600.0F, 1e-6F);

    // compare depth to some known values
    // extracted by hand from etopo_cmp2w.log

    index[0] = 0;
    index[1] = 61;  // 1S 180.0167E (aka -179.98333W) = first entry, first row
    BOOST_CHECK_CLOSE(bathy.data(index), -5284.0F, 1e-6F);
    index[0] = 0;
    index[1] = 180;  // 1S 182E (aka 178W) = last entry, first row
    BOOST_CHECK_CLOSE(bathy.data(index), -4775.0F, 1e-6F);

    // dump CSV file for processing by graphics program

    std::ofstream ofile(USML_TEST_DIR "/netcdf/test/span_bathy.csv");
    for (long n = 0; n < num_lat; ++n) {
        index[0] = n;
        for (long m = 0; m < num_lng; ++m) {
            index[1] = m;
            ofile << bathy.data(index);
            if (m < (num_lng - 1)) {
                ofile << ",";
            }
        }
        ofile << std::endl;
    }
}

/**
 * Tests the ability of the netcdf_bathy class to extract data
 * from a netcdf file that does not have a global range. This
 * would in return prevent the data from wrapping around and
 * instead limit the bounds of the data to the database provided.
 */
BOOST_AUTO_TEST_CASE(nonglobal_database) {
    cout << "=== read_bathy_test: nonglobal_database ===" << endl;
    cout << "reading " << USML_TEST_DIR << "/netcdf/test/flstrts_bathymetry.nc" << endl;
    netcdf_bathy bathy(USML_TEST_DIR "/netcdf/test/flstrts_bathymetry.nc",
                       -90.0, 90.0, -180.0, 180.0, 0.0);

    // compare latitude axis to values read using ncdump

    const seq_vector& latitude = bathy.axis(0);
    auto num_lat = latitude.size();
    auto lat1 = to_latitude(latitude(0));
    auto lat2 = to_latitude(latitude(num_lat - 1));
    auto inc_lat = to_degrees(latitude.increment(0));
    cout << "latitude[" << num_lat << "] = " << lat1 << " to " << lat2 << " by "
         << inc_lat << endl;
    BOOST_CHECK_EQUAL(num_lat, 420);
    BOOST_CHECK_CLOSE(lat1, 27.0, 1e-6);
    BOOST_CHECK_CLOSE(lat2, 33.9833333333, 1e-6);

    // compare longitude axis to values read using ncdump

    const seq_vector& longitude = bathy.axis(1);
    auto num_lng = longitude.size();
    auto lng1 = to_degrees(longitude(0));
    auto lng2 = to_degrees(longitude(num_lng - 1));
    auto inc_lng = to_degrees(longitude.increment(0));
    cout << "longitude[" << num_lng << "] = " << lng1 << " to " << lng2
         << " by " << inc_lng << endl;
    BOOST_CHECK_EQUAL(num_lng, 481);
    BOOST_CHECK_CLOSE(lng1, -82.0, 1e-6);
    BOOST_CHECK_CLOSE(lng2, -74.0, 1e-6);
}

/**
 * Test for issue #114 - data_grid linear interpolation produces unpredictable
 * results in Win64 release builds.  When running usml_test using a Win x64
 * release build the reflection_test/reflect_grid_test fails.  We traced this
 * down to an error in using the linear() interpolation method with a 2-D
 * data_grid. If we add debug cout statements after the const  a =
 * interp() and const  b = interp() we can see that a changes value
 * while b is being computed. Even though linear is used recursively,
 * a is a local variable and this should not happen.  It only seems to happen
 * for linear 2-D interpolation of ETOPO1 bathymetry, and only using the
 * Release x64 model of Visual C+++ 2012 in Visual Studio 2013. Perhaps
 * it has something to do with optimizing a recursive template function.
 */
BOOST_AUTO_TEST_CASE(grid_2d_test) {
    cout << "=== grid_2d_test ===" << endl;
    const double lat1 = 35.5;   // Mediterranean sea
    const double lat2 = 36.5;   // malta escarpment
    const double lng1 = 15.25;  // south-east of Sicily
    const double lng2 = 16.25;
    netcdf_bathy bathy(USML_DATA_DIR "/bathymetry/ETOPO1_Ice_g_gmt4.grd", lat1,
                       lat2, lng1, lng2);

    const char* filename = USML_TEST_DIR "/netcdf/test/grid_2d_test.nc";
    bathy.write_netcdf(filename);
    bathy.interp_type(0, interp_enum::pchip);
    bathy.interp_type(1, interp_enum::pchip);

    std::srand(0);
    size_t N = 100;
    double rho = 0;
    size_t size1(bathy.axis(0).size());
    size_t size2(bathy.axis(1).size());
    size_t loc = 0;
    double location[2];
    for (size_t i = 0; i < N; ++i) {
        loc = std::rand() % size1; // NOLINT(concurrency-mt-unsafe)
        location[0] = bathy.axis(0)[loc];
        // cout << "loc[0]: " << location[0];
        loc = std::rand() % size2; // NOLINT(concurrency-mt-unsafe)
        location[1] = bathy.axis(1)[loc];
        // cout << " loc[1]: " << location[1] << endl;
        rho = bathy.interpolate(location);
        double height = rho - wposition::earth_radius;
        // cout << "height: " << height << endl;
        BOOST_CHECK(height > -6000.0);
    }
}

/// @}
BOOST_AUTO_TEST_SUITE_END()
