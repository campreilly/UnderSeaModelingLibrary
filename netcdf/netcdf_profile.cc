/**
 * @file netcdf_profile.cc
 * Extracts ocean profile data from world-wide databases.
 */
#include <usml/netcdf/netcdf_profile.h>
#include <usml/types/seq_data.h>
#include <usml/types/seq_linear.h>
#include <usml/types/seq_vector.h>

#include <boost/algorithm/string/predicate.hpp>

using namespace usml::netcdf;

/**
 * Load ocean profile from disk.
 */
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
netcdf_profile::netcdf_profile(const char *filename, double date, double south,
                               double north, double west, double east,
                               const char *varname) {
    // initialize access to NetCDF file.

    std::string fname(filename);
    netCDF::NcFile file(fname, netCDF::NcFile::FileMode::read);

    double missing = NAN;       // default value for missing information
    double scale_factor = NAN;  // default value for scale_factor
    double add_offset = NAN;    // default value for add_offset
    netCDF::NcVar longitude;
    netCDF::NcVar latitude;
    netCDF::NcVar altitude;
    netCDF::NcVar time;
    netCDF::NcVar profile;

    decode_filetype(file, missing, scale_factor, add_offset, time, altitude,
                    latitude, longitude, profile, varname);

    // find the time closest to the specified value

    size_t time_index = 0;
    const size_t time_num = time.getDim(0).getSize();
    std::vector<double> time_values(time_num);
    time.getVar(time_values.data());
    double old_diff = abs(date - time_values[0]);
    for (size_t t = 1; t < time_num; ++t) {
        double diff = abs(date - time_values[t]);
        if (old_diff > diff) {
            old_diff = diff;
            time_index = t;
        }
    }

    // read altitude axis data from NetCDF variable

    const size_t alt_num = altitude.getDim(0).getSize();
    std::vector<double> alt_values(alt_num);
    altitude.getVar(alt_values.data());
    for (size_t a = 0; a < alt_num; ++a) {
        alt_values[a] = wposition::earth_radius - alt_values[a];
    }
    _axis[0] = seq_vector::csptr(new seq_data(alt_values));

    // read latitude and longitude axis data from NetCDF variables

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
        if (abs(value0 + 360.0 - valueN) < 1e-4) {
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
    _axis[1] = seq_vector::csptr(
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
    _axis[2] = seq_vector::csptr(
        new seq_linear(to_radians(lng_first * inc + value0 - offset),
                       to_radians(inc), lng_num));

    // load depth data out of NetCDF file

    auto *data = new double[lat_num * lng_num * alt_num];
    _writeable_data = std::shared_ptr<double[]>(data);
    _data = _writeable_data;

    std::vector<size_t> start(4);
    std::vector<size_t> count(4);

    if (lng_last <= lng_index_max || !global) {
        start[0] = time_index;
        start[1] = 0;
        start[2] = lat_first;
        start[3] = lng_first;
        count[0] = 1;
        count[1] = alt_num;
        count[2] = lat_num;
        count[3] = lng_num;
        profile.getVar(start, count, data);
    } else {
        // support datasets that cross the unwrapping longitude
        // assumes that bathy data is repeated on both sides of cut point

        const size_t M = lng_last - lng_index_max;  // # pts on east side
        const size_t N = lng_num - M;               // # pts on west side
        double *ptr = data;
        for (long alt = 0; alt < alt_num; ++alt) {
            for (int lat = lat_first; lat <= lat_last; ++lat) {
                // the west side of the block is the portion from
                // lng_first to the last latitude
                start[0] = time_index;
                start[1] = alt;
                start[2] = lat;
                start[3] = lng_first;
                count[0] = 1;
                count[1] = 1;
                count[2] = 1;
                count[3] = N;
                profile.getVar(start, count, ptr);
                ptr += N;

                // the missing points on the east side of the block
                // are read from zero until the right # of points are read
                // skip first longitude if it is a duplicate
                start[0] = time_index;
                start[1] = alt;
                start[2] = lat;
                start[3] = duplicate;
                count[0] = 1;
                count[1] = 1;
                count[2] = 1;
                count[3] = M;
                profile.getVar(start, count, ptr);
                ptr += M;
            }
        }
    }

    // apply logic for missing, scale_factor, and add_offset

    if (!std::isnan(missing) || !std::isnan(scale_factor * add_offset)) {
        const size_t N = alt_num * lat_num * lng_num;
        double *ptr = data;
        while (ptr < data + N) {
            if (!std::isnan(missing) && *ptr == missing) {
                *ptr = NAN;
            }
            if (!std::isnan(scale_factor * add_offset)) {
                *ptr = *ptr * scale_factor + add_offset;
            }
            ++ptr;
        }
    }
}

/**
 * Fill missing values with average data at each depth.
 */
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void netcdf_profile::fill_missing() {
    gen_grid<3> replace(_axis);

    seq_vector::csptr depth = _axis[0];
    size_t ndepth = depth->size();
    size_t nlat = _axis[1]->size();
    size_t nlon = _axis[2]->size();

    // compute the profile depth gradient for points beyond the first depth
    // initialize the replacement gradient while we are here

    vector<matrix<double>> profile_grad(ndepth);
    vector<matrix<double>> replace_grad(ndepth);
    size_t max_depth = 0;
    for (size_t d = 0; d < ndepth; ++d) {
        replace_grad(d) = zero_matrix<double>(nlat, nlon);
        profile_grad(d) = scalar_matrix<double>(nlat, nlon, NAN);
        if (d > 0) {
            for (size_t j = 0; j < nlat; ++j) {
                for (size_t k = 0; k < nlon; ++k) {
                    const size_t index[] = {d, j, k};
                    const double curr = data(index);
                    if (!std::isnan(curr)) {
                        max_depth = max(max_depth, d);
                        const size_t index2[] = {d - 1, j, k};
                        const double prev = data(index2);
                        profile_grad(d)(j, k) =
                            (curr - prev) / depth->increment(d - 1);
                    }
                }
            }
        }
    }

    // build up replacements as the sum of the points around them
    // in latitude and longitude, weighted by the fourth power of the distance

    const double distExp = -4.0;  // distance weighting exponent
    for (size_t d = 0; d < max_depth + 1; ++d) {
        for (size_t j = 0; j < nlat; ++j) {
            for (size_t k = 0; k < nlon; ++k) {
                const size_t index[] = {d, j, k};
                double r = data(index);
                if (!std::isnan(r)) {
                    replace.setdata(index, r);
                    replace_grad(d)(j, k) = profile_grad(d)(j, k);
                } else {
                    // compute weighted sum

                    double weight = 0.0;
                    for (size_t n = 0; n < nlat; ++n) {
                        for (size_t m = 0; m < nlon; ++m) {
                            const size_t index2[] = {d, n, m};
                            const double tmp = data(index2);
                            if (!std::isnan(tmp)) {
                                const double dj = double(j) - double(n);
                                const double dk = double(k) - double(m);
                                const double t = dj * dj + dk * dk;
                                const double distScale = pow(t, distExp);
                                weight += distScale;

                                // for the first depth, compute the weighted
                                // sum of the actual data values

                                if (d == 0) {
                                    r = replace.data(index) + distScale * tmp;
                                    replace.setdata(index, r);

                                    // for the other depths, compute the
                                    // weighted sum of the depth gradients

                                } else {
                                    replace_grad(d)(j, k) +=
                                        distScale * profile_grad(d)(n, m);
                                }  // end if( d==0 )
                            }      // end if( !isnan(tmp) )
                        }          // end for m<nlon
                    }              // end for n<nlat

                    // convert sums to weighted average

                    if (weight > 0.0) {
                        if (d == 0) {
                            r = replace.data(index) / weight;
                            replace.setdata(index, r);
                        } else {
                            replace_grad(d)(j, k) /= weight;
                            const size_t index2[] = {d - 1, j, k};
                            r = replace.data(index2) +
                                replace_grad(d)(j, k) * depth->increment(d - 1);
                            replace.setdata(index, r);
                        }
                    }  // end if( weight > 0.0 )
                }      // end if( ! isnan(r) )
            }          // end for k<nlon
        }              // end for j<nlat
    }                  // end for d<ndepth

    // fill in values beyond the maximum depth
    // assumes that the gradient at each latitude and longitude tapers to zero
    // the gradient at each depth is taken to be half of the gradient above it

    for (size_t j = 0; j < nlat; ++j) {
        for (size_t k = 0; k < nlon; ++k) {
            double w = 2.0;
            if (max_depth + 1 < ndepth) {
                for (size_t d = max_depth + 1; d < ndepth; ++d) {
                    replace_grad(d)(j, k) = replace_grad(max_depth)(j, k) / w;
                    const size_t index[] = {d, j, k};
                    const size_t prev[] = {d - 1, j, k};
                    const double v =
                        replace.data(prev) +
                        replace_grad(d)(j, k) * abs(depth->increment(d - 1));
                    replace.setdata(index, v);
                    w *= 2.0;
                }
            } else {
                replace_grad(max_depth)(j, k) =
                    replace_grad(max_depth)(j, k) / w;
                const size_t index[] = {max_depth, j, k};
                const size_t prev[] = {max_depth - 1, j, k};
                const double v = replace.data(prev) +
                                 replace_grad(max_depth)(j, k) *
                                     abs(depth->increment(max_depth - 1));
                replace.setdata(index, v);
            }  // end if( max_depth+1 < ndepth )
        }      // end for k<nlon
    }          // end for j<nlat

    size_t N = _axis[0]->size() * _axis[1]->size() * _axis[2]->size();
    std::copy_n(replace.data(), N, _writeable_data.get());
}

/**
 * Deduces the variables to be loaded based on their dimensionality.
 */
void netcdf_profile::decode_filetype(
    netCDF::NcFile &file, double &missing, double &scale, double &offset,
    netCDF::NcVar &time, netCDF::NcVar &altitude, netCDF::NcVar &latitude,
    netCDF::NcVar &longitude, netCDF::NcVar &profile, const char *varname) {
    bool found = false;
    for (const auto &entry : file.getVars()) {
        const netCDF::NcVar &var = entry.second;
        // search for variables with time, lat, lon, alt components

        if (var.getDimCount() != 4) {
            continue;
        }

        // match variable name, if provided

        if ((varname != nullptr) &&
            !boost::algorithm::icontains(var.getName(), varname)) {
            continue;
        }

        // extract profile variable, it's coordinates, and attributes

        profile = var;
        time = file.getVar(var.getDim(0).getName());
        altitude = file.getVar(var.getDim(1).getName());
        latitude = file.getVar(var.getDim(2).getName());
        longitude = file.getVar(var.getDim(3).getName());

        {
            // NOLINTBEGIN(bugprone-empty-catch)
            netCDF::NcVarAtt att;

            // extract fill value attribute
            try {
                att = profile.getAtt("_FillValue");
                att.getValues(&missing);
            } catch (const netCDF::exceptions::NcException &ex) {
            }

            // extract scale factor attribute
            try {
                att = profile.getAtt("scale_factor");
                att.getValues(&scale);
            } catch (const netCDF::exceptions::NcException &ex) {
            }

            // extract add offset attribute
            try {
                att = profile.getAtt("add_offset");
                att.getValues(&offset);
            } catch (const netCDF::exceptions::NcException &ex) {
            }
            // NOLINTEND(bugprone-empty-catch)
        }

        // stop searching
        found = true;
        break;
    }

    if (!found) {
        throw std::invalid_argument("unrecognized file type");
    }
}
