/**
 * @file netcdf_bathy.cc
 * Extracts bathymetry data from world-wide bathymetry databases.
 */

#include <usml/netcdf/netcdf_bathy.h>
#include <usml/types/seq_linear.h>
#include <usml/types/seq_vector.h>
#include <usml/ublas/math_traits.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <memory>
#include <stdexcept>

using namespace usml::netcdf;

/**
 * Load bathymetry from disk.
 */
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
netcdf_bathy::netcdf_bathy(const char* filename, double south, double north,
                           double west, double east, double earth_radius) {
    // initialize access to NetCDF file.

    std::string fname(filename);
    netCDF::NcFile file(fname, netCDF::NcFile::FileMode::read);
    netCDF::NcVar longitude;
    netCDF::NcVar latitude;
    netCDF::NcVar altitude;
    decode_filetype(file, latitude, longitude, altitude);

    double offset = 0.0;
    unsigned duplicate = 0;
    const unsigned lat_index_max = latitude.getDim(0).getSize() - 1;
    const unsigned lng_index_max = longitude.getDim(0).getSize() - 1;

    double value0;
    double valueN;
    std::vector<size_t> index0(1, 0);
    std::vector<size_t> indexN(1, lng_index_max);
    longitude.getVar(index0, &value0);
    longitude.getVar(indexN, &valueN);

    // Is the data set bounds 0 to 359(360)
    bool zero_to_360 = value0 < 1.0 && valueN >= 359.0;

    // Is the data set bounds -180 to 179(180)
    bool bounds_180 = value0 < -179.0 && valueN > 179.0;

    // Is this set a global data set
    bool global = (zero_to_360 || bounds_180);
    if (global) {
        // check to see if database has duplicate data at cut point
        if (abs(value0 + 360 - valueN) < 1e-4) {
            duplicate = 1;
        }

        // manage wrap-around between eastern and western hemispheres
        if (value0 < 0.0) {
            // if database has a range (-180,180)
            // make western longitudes into negative numbers
            // unless they span the 180 latitude
            if (west > 180.0 && east > 180.0) {
                offset = -360.0;
            }
        } else {
            // if database has a range (0,360)
            // make all western longitudes into positive numbers
            if (west < 0.0) {
                offset = 360.0;
            }
        }
    } else {
        if (value0 > 180.0) {
            if (west < 0.0) {
                offset = 360.0;
            }
        } else if (value0 < 0.0) {
            if (east > 180.0) {
                offset = -360.0;
            }
        }
    }
    west += offset;
    east += offset;

    // read latitude axis data from NetCDF file.
    // lat_first and lat_last are the integer offsets along this axis
    // _axis[0] is expressed as co-latitude in radians [0,PI]

    indexN[0] = lat_index_max;
    latitude.getVar(index0, &value0);
    latitude.getVar(indexN, &valueN);

    double inc = (valueN - value0) / lat_index_max;
    const int lat_first = max(0, (int)floor(1e-6 + (south - value0) / inc));
    const int lat_last =
        min((int)lat_index_max, (int)(floor(0.5 + (north - value0) / inc)));
    const size_t lat_num = lat_last - lat_first + 1;
    _axis[0] = seq_vector::csptr(
        new seq_linear(to_colatitude(double(lat_first) * inc + value0),
                       to_radians(-inc), lat_num));

    // read longitude axis data from NetCDF file
    // lng_first and lng_last are the integer offsets along this axis
    // _axis[1] is expressed as longitude in radians [-PI,2*PI]

    indexN[0] = lng_index_max;
    longitude.getVar(index0, &value0);
    longitude.getVar(indexN, &valueN);

    inc = (valueN - value0) / lng_index_max;
    int index = int(floor(1e-6 + (west - value0) / inc));
    const int lng_first = (global) ? index : max(0, index);
    index = int(floor(0.5 + (east - value0) / inc));
    const int lng_last = (global) ? index : min(int(lng_index_max), index);
    const size_t lng_num = lng_last - lng_first + 1;
    _axis[1] = seq_vector::csptr(
        new seq_linear(to_radians(lng_first * inc + value0 - offset),
                       to_radians(inc), lng_num));

    // load depth data out of NetCDF file

    auto* data = new double[lat_num * lng_num];
    _writeable_data = std::shared_ptr<double[]>(data);
    _data = _writeable_data;

    std::vector<size_t> start(2);
    std::vector<size_t> count(2);
    start[0] = lat_first;
    start[1] = lng_first;
    count[0] = lat_num;
    count[1] = lng_num;

    if (lng_last <= lng_index_max || !global) {
        altitude.getVar(start, count, data);
    } else {
        // support datasets that cross the unwrapping longitude
        // assumes that bathy data is repeated on both sides of cut point

        const size_t M = lng_last - lng_index_max;  // # pts on east side
        const size_t N = lng_num - M;               // # pts on west side
        double* ptr = data;
        for (int lat = lat_first; lat <= lat_last; ++lat) {
            // the west side of the block is the portion from
            // lng_first to the last latitude
            start[0] = lat;
            start[1] = lng_first;
            count[0] = 1;
            count[1] = N;
            altitude.getVar(start, count, ptr);
            ptr += N;

            // the missing points on the east side of the block
            // are read from zero until the right # of points are read
            // skip first longitude if it is a duplicate
            start[0] = lat;
            start[1] = duplicate;
            count[0] = 1;
            count[1] = M;
            altitude.getVar(start, count, ptr);
            ptr += M;
        }
    }

    // convert depth to rho coordinate of spherical earth system

    double* ptr = data;
    double* end = data + (lat_num * lng_num);
    while (ptr < end) {
        *(ptr++) += earth_radius;
    }
}

/**
 * Deduces the variables to be loaded based on their dimensionality.
 */
void netcdf_bathy::decode_filetype(netCDF::NcFile& file,
                                   netCDF::NcVar& latitude,
                                   netCDF::NcVar& longitude,
                                   netCDF::NcVar& altitude) {
    bool found = false;
    for (const auto& entry : file.getVars()) {
        const netCDF::NcVar& var = entry.second;
        if (var.getDimCount() == 2) {
            // extract depth variable
            altitude = var;

            // extract latitude variable
            netCDF::NcDim dim = var.getDim(0);
            latitude = file.getVar(dim.getName());

            // extract longitude variable
            dim = var.getDim(1);
            longitude = file.getVar(dim.getName());

            // stop searching
            found = true;
            break;
        }
    }
    if (!found) {
        throw std::invalid_argument("unrecognized file type");
    }
}
