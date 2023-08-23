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
netcdf_profile::netcdf_profile(const char *profile, double date, double south,
                               double north, double west, double east,
                               const char *varname) {
    // initialize access to NetCDF file.

    double missing = NAN;       // default value for missing information
    double scale_factor = NAN;  // default value for scale_factor
    double add_offset = NAN;    // default value for add_offset
    NcVar *time;
    NcVar *altitude;
    NcVar *latitude;
    NcVar *longitude;
    NcVar *value;
    NcFile pfile(profile);
    if (pfile.is_valid() == 0) {
        throw std::invalid_argument("file not found");
    }
    decode_filetype(pfile, &missing, &scale_factor, &add_offset, &time,
                    &altitude, &latitude, &longitude, &value, varname);

    // find the time closest to the specified value

    long time_index = 0;
    double old_diff = abs(date - time->as_double(0));
    for (long t = 1; t < time->num_vals(); ++t) {
        double diff = abs(date - time->as_double(t));
        if (old_diff > diff) {
            old_diff = diff;
            time_index = t;
        }
    }

    // read altitude axis data from NetCDF variable

    const auto alt_num = long(altitude->num_vals());
    vector<double> vect(alt_num);
    for (long d = 0; d < alt_num; ++d) {
        vect[d] = wposition::earth_radius - abs(altitude->as_double(d));
    }
    _axis[0] = seq_vector::csptr(new seq_data(vect));

    // manage wrap-around between eastern and western hemispheres

    double offset = 0.0;
    long duplicate = 0;
    long n = longitude->num_vals() - 1;
    // Is the data set bounds 0 to 359(360)
    bool zero_to_360 =
        longitude->as_double(0) <= 1.0 && longitude->as_double(n) >= 359.0;
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

    // read latitude axis data from NetCDF variable
    // lat_first and lat_last are the integer offsets along this axis
    // _axis[1] is expressed as co-latitude in radians [0,PI]

    double a = latitude->as_double(0);
    n = latitude->num_vals() - 1;
    double inc = double(latitude->as_double(n) - a) / double(n);
    const long lat_first = max(0L, long(floor(1e-6 + (south - a) / inc)));
    const long lat_last = min(n, long(floor(0.5 + (north - a) / inc)));
    const long lat_num = lat_last - lat_first + 1;
    _axis[1] = seq_vector::csptr(
        new seq_linear(to_colatitude(double(lat_first) * inc + a),
                       to_radians(-inc), size_t(lat_num)));

    // read longitude axis data from NetCDF variable
    // lng_first and lng_last are the integer offsets along this axis
    // _axis[2] is expressed as longitude in radians [-PI,2*PI]

    a = longitude->as_double(0);
    n = long(longitude->num_vals() - 1);
    inc = double(longitude->as_double(n) - a) / double(n);
    auto index = long(floor(1e-6 + (west - a) / inc));
    const long lng_first = (global) ? index : max(0L, index);
    index = long(floor(0.5 + (east - a) / inc));
    const long lng_last = (global) ? index : min(n, index);
    const long lng_num = lng_last - lng_first + 1;
    _axis[2] = seq_vector::csptr(
        new seq_linear(to_radians(double(lng_first) * inc + a - offset),
                       to_radians(inc), size_t(lng_num)));

    // load profile data out of NetCDF variable

    auto *data = new double[lat_num * lng_num * alt_num];
    _writeable_data = std::shared_ptr<double>(data);
    _data = _writeable_data;

    if (longitude->num_vals() > lng_last) {
        value->set_cur(time_index, 0, lat_first, lng_first);
        value->get(data, 1, alt_num, lat_num, lng_num);

        // support datasets that cross the unwrapping longitude
        // assumes that bathy data is NOT repeated on both sides of cut point

    } else {
        long M = lng_last - longitude->num_vals() + 1;  // # pts on east side
        long N = lng_num - M;                           // # pts on west side
        double *ptr = data;
        for (long alt = 0; alt < alt_num; ++alt) {
            for (long lat = lat_first; lat <= lat_last; ++lat) {
                // the west side of the block is the portion from
                // lng_first to the last latitude
                value->set_cur(time_index, alt, lat, lng_first);
                value->get(ptr, 1, 1, 1, N);
                ptr += N;

                // the missing points on the east side of the block
                // are read from zero until the right # of points are read
                // skip first longitude if it is a duplicate
                value->set_cur(time_index, alt, lat, duplicate);
                value->get(ptr, 1, 1, 1, M);
                ptr += M;
            }
        }
    }

    // apply logic for missing, scale_factor, and add_offset
    if (!std::isnan(missing) || !std::isnan(scale_factor * add_offset)) {
        const long N = alt_num * lat_num * lng_num;
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
//NOLINTNEXTLINE(readability-function-cognitive-complexity)
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
void netcdf_profile::decode_filetype(NcFile &file, double *missing,
                                     double *scale, double *offset,
                                     NcVar **time, NcVar **altitude,
                                     NcVar **latitude, NcVar **longitude,
                                     NcVar **value, const char *varname) {
    bool found = false;
    for (int n = 0; n < file.num_vars(); ++n) {
        NcVar *var = file.get_var(n);

        // search for variables with time, lat, lon, alt components
        if (var->num_dims() != 4) {
            continue;
        }

        // match variable name, if provided
        if ((varname != nullptr) &&
            !boost::algorithm::icontains(var->name(), varname)) {
            continue;
        }

        // extract profile variable
        *value = var;

        // extract time variable
        NcDim *dim = var->get_dim(0);
        *time = file.get_var(dim->name());

        // extract altitude variable
        dim = var->get_dim(1);
        *altitude = file.get_var(dim->name());

        // extract latitude variable
        dim = var->get_dim(2);
        *latitude = file.get_var(dim->name());

        // extract longitude variable
        dim = var->get_dim(3);
        *longitude = file.get_var(dim->name());

        {
            // turn off errors for missing attributes
            auto *ncdfError = new NcError(NcError::silent_nonfatal);
            NcAtt *att;

            // extract missing attribute
            att = var->get_att("_FillValue");
            if (att != nullptr) {
                NcValues *values = att->values();
                *missing = values->as_double(0);
                delete att;
                delete values;
            }

            // extract missing attribute
            att = var->get_att("scale_factor");
            if (att != nullptr) {
                NcValues *values = att->values();
                *scale = values->as_double(0);
                delete att;
                delete values;
            }

            // extract missing attribute
            att = var->get_att("add_offset");
            if (att != nullptr) {
                NcValues *values = att->values();
                *offset = values->as_double(0);
                delete att;
                delete values;
            }

            delete ncdfError;
        }

        // stop searching
        found = true;
        break;
    }
    if (!found) {
        throw std::invalid_argument("unrecognized file type");
    }
}
