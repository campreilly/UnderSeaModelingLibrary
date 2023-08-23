/**
 * @file netcdf_coards.h
 * Reads a single COARDS data grid from a netCDF file.
 */
#pragma once

#include <ncvalues.h>
#include <netcdfcpp.h>
#include <usml/types/gen_grid.h>
#include <usml/types/seq_vector.h>

#include <cmath>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <vector>

namespace usml {
namespace netcdf {

using namespace usml::ublas;
using namespace usml::types;

/// @ingroup netcdf_files
/// @{

/**
 * Reads a single COARDS data grid from a netCDF file.
 * Assumes that the entire file should be read into memory.
 *
 * The Cooperative Ocean/Atmosphere Research Data Service
 * (COARDS) is a NOAA/university cooperative for the sharing
 * and distribution of global atmospheric and oceanographic
 * research data sets.  COARDS is also a NAVOCEANO
 * recommended practice for netCDF files.
 *
 * @xref Cooperative Ocean/Atmosphere Research Data Service,
 * "Conventions for the standardization of NetCDF files," May 1995.
 * See http://ferret.wrc.noaa.gov/noaa_coop/coop_cdf_profile.html
 * for more information.
 */
template <int NUM_DIMS>
class netcdf_coards : public gen_grid<NUM_DIMS> {
   private:
    /**
     * Construct a seq_vector from NetCDF dimension object.
     * Inspects the data to see if seq_linear or seq_log can be
     * used to optimize the performance of this dimension->
     *
     * @param  file         NetCDF file to process.
     * @param  dimension    NetCDF dimension. The COARDS spec assumes that
     *                      there is a NetCDF variable of the same name.
     * @return              Sequence vector equivalent.
     */
    seq_vector::csptr make_axis(NcFile& file, NcDim* dimension) {
        // search for this axis in the NetCDF file

        NcVar* axis = file.get_var(dimension->name());
        if (axis == 0) {
            throw std::invalid_argument("NetCDF variable not found");
        }

        const int N = (int)axis->num_vals();
        std::vector<double> data;

        // Convert to std::vector
        for (int n = 0; n < N; ++n) {
            double p = axis->as_double(n);
            data.push_back(p);
        }

        // build and return
        // best fit for seq_linear or seq_log or worst case seq_data
        return seq_vector::build_best(data);
    }

   public:
    /**
     * Extract a named data grid from an open NetCDF file.
     * Creates new memory area for field data.
     * Replaces missing data with fill value.
     *
     * @param  file         Reference to an open NetCDF file.
     * @param  name         Name of the data grid to extract (case sensitive).
     * @param  read_fill    Read _FillValue from NetCDF file if true.
     *                         Use NAN as fill value if false.
     */
    netcdf_coards(NcFile& file, NcToken name, bool read_fill = false) {
        // search for this grid in the NetCDF file

        NcVar* variable = file.get_var(name);
        if (variable == 0) {
            throw std::invalid_argument("NetCDF variable not found");
        }

        // read axis data from NetCDF file.

        size_t N = 1;
        for (int n = 0; n < NUM_DIMS; ++n) {
            seq_vector::csptr ax = make_axis(file, variable->get_dim(n));
            this->_axis[n] = ax;
            N *= this->_axis[n]->size();
        }

        // extract missing attribute information

        NcError nc_error(NcError::silent_nonfatal);

        double missing = NAN;  // default value for missing data
        NcAtt* att = variable->get_att("missing_value");
        if (att) {
            NcValues* values = att->values();
            missing = (double)values->as_double(0);
            delete att;
            delete values;
        }

        double filling = NAN;  // default for fill value
        if (read_fill) {
            att = variable->get_att("_FillValue");
            if (att) {
                NcValues* values = att->values();
                filling = values->as_double(0);
                delete att;
                delete values;
            }
        }

        // copy interpolant data from the NetCDF file into local memory.
        // replace missing data with fill value

        double* data = new double[N];
        this->_writeable_data = std::shared_ptr<double>(data);
        this->_data = this->_writeable_data;

        NcValues* values = variable->values();
        for (size_t n = 0; n < N; ++n) {
            data[n] = (double)values->as_double((long)n);
            if (!std::isnan(missing)) {
                if (data[n] == missing) {
                    data[n] = filling;
                }
            }
        }
        delete values;
    }
};

/// @}
}  // end of namespace netcdf
}  // end of namespace usml
