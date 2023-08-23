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
//NOLINTNEXTLINE(readability-function-cognitive-complexity)
netcdf_bathy::netcdf_bathy(const char* filename, double south, double north,
                           double west, double east, double earth_radius) {
    // initialize access to NetCDF file.

    NcFile file(filename);
    if (file.is_valid() == 0) {
        throw std::invalid_argument("file not found");
    }
    NcVar* longitude;
    NcVar* latitude;
    NcVar* altitude;
    decode_filetype(file, &latitude, &longitude, &altitude);

    double offset = 0.0;
    long duplicate = 0;
    long n = longitude->num_vals() - 1;

    // Is the data set bounds 0 to 359(360)
    bool zero_to_360 =
        longitude->as_double(0) == 0.0 && longitude->as_double(n) >= 359.0;

    // Is the data set bounds -180 to 179(180)
    bool bounds_180 =
        longitude->as_double(n) >= 179.0 && longitude->as_double(0) == -180.0;

    // Is this set a global data set
    bool global = (zero_to_360 || bounds_180);
    if (global) {
        // check to see if database has duplicate data at cut point
        if (abs(longitude->as_double(0) + 360 - longitude->as_double(n)) <
            1e-4) {
            duplicate = 1;
        }

        // manage wrap-around between eastern and western hemispheres
        if (longitude->as_double(0) < 0.0) {
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
        if (longitude->as_double(0) > 180.0) {
            if (west < 0.0) {
                offset = 360.0;
            }
        } else if (longitude->as_double(0) < 0.0) {
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

    double a = latitude->as_double(0);
    n = latitude->num_vals() - 1;
    double inc = double(latitude->as_double(n) - a) / double(n);
    const long lat_first = max(0L, long(floor(1e-6 + (south - a) / inc)));
    const long lat_last = min(n, long(floor(0.5 + (north - a) / inc)));
    const long lat_num = lat_last - lat_first + 1;
    _axis[0] = seq_vector::csptr(
        new seq_linear(to_colatitude(double(lat_first) * inc + a),
                       to_radians(-inc), size_t(lat_num)));

    // read longitude axis data from NetCDF file
    // lng_first and lng_last are the integer offsets along this axis
    // _axis[1] is expressed as longitude in radians [-PI,2*PI]

    a = longitude->as_double(0);
    n = longitude->num_vals() - 1;
    inc = double(longitude->as_double(n) - a) / double(n);
    auto index = long(floor(1e-6 + (west - a) / inc));
    const long lng_first = (global) ? index : max(0L, index);
    index = long(floor(0.5 + (east - a) / inc));
    const long lng_last = (global) ? index : min(n, index);
    const long lng_num = lng_last - lng_first + 1;
    _axis[1] = seq_vector::csptr(
        new seq_linear(to_radians(double(lng_first) * inc + a - offset),
                       to_radians(inc), size_t(lng_num)));

    // load depth data out of NetCDF file

    auto* data = new double[lat_num * lng_num];
    _writeable_data = std::shared_ptr<double>(data);
    _data = _writeable_data;

    if (longitude->num_vals() > lng_last || !global) {
        altitude->set_cur(lat_first, lng_first);
        altitude->get(data, lat_num, lng_num);

        // support datasets that cross the unwrapping longitude
        // assumes that bathy data is repeated on both sides of cut point

    } else {
        long M = lng_last - longitude->num_vals() + 1;  // # pts on east side
        long N = lng_num - M;                           // # pts on west side
        double* ptr = data;
        for (long lat = lat_first; lat <= lat_last; ++lat) {
            // the west side of the block is the portion from
            // lng_first to the last latitude
            altitude->set_cur(lat, lng_first);
            altitude->get(ptr, 1, N);
            ptr += N;

            // the missing points on the east side of the block
            // are read from zero until the right # of points are read
            // skip first longitude if it is a duplicate
            altitude->set_cur(lat, duplicate);
            altitude->get(ptr, 1, M);
            ptr += M;
        }
    }

    // convert depth to rho coordinate of spherical earth system

    double* ptr = data;
    auto R = double(earth_radius);
    while (ptr < data + (lat_num * lng_num)) {
        *(ptr++) += R;
    }
}

/**
 * Deduces the variables to be loaded based on their dimensionality.
 */
void netcdf_bathy::decode_filetype(NcFile& file, NcVar** latitude,
                                   NcVar** longitude, NcVar** altitude) {
    bool found = false;
    for (int n = 0; n < file.num_vars(); ++n) {
        NcVar* var = file.get_var(n);
        if (var->num_dims() == 2) {
            // extract depth variable
            *altitude = var;

            // extract latitude variable
            NcDim* dim = var->get_dim(0);
            *latitude = file.get_var(dim->name());

            // extract longitude variable
            dim = var->get_dim(1);
            *longitude = file.get_var(dim->name());

            // stop searching
            found = true;
            break;
        }
    }
    if (!found) {
        throw std::invalid_argument("unrecognized file type");
    }
}
