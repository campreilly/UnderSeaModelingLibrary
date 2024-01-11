/**
 * @file netcdf_woa.cc
 * Extracts ocean profile data from world-wide databases.
 */

#include <usml/netcdf/netcdf_woa.h>
#include <usml/types/data_grid.h>
#include <usml/types/gen_grid.h>
#include <usml/types/seq_vector.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <memory>

using namespace usml::netcdf;

/**
 * Load deep and shallow parts of WOA ocean profile from disk.
 */
netcdf_woa::netcdf_woa(const char* deep, const char* shallow, int month,
                       double south, double north, double west, double east)
    : netcdf_profile(deep, int(round(30.5 * (month - 0.5))), south, north, west,
                     east) {
    // replace beginning of deep data with shallow values

    if (shallow != nullptr) {
        netcdf_profile replace(shallow, int(round(30.5 * (month - 0.5))), south,
                               north, west, east);
        size_t N = replace.axis(0).size() * replace.axis(1).size() *
                   replace.axis(2).size();
        std::copy_n(replace.data(), N, _writeable_data.get());
    }

    // data conditioning

    fill_missing();  // replace NaNs with real data
    interp_type(0,interp_enum::pchip);  // increase smoothness of depth interp
}
