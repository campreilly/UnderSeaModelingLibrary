/**
 * @file netcdf_coards.h
 * Reads a single COARDS data grid from a netCDF file.
 */
#pragma once

#include <usml/types/gen_grid.h>
#include <usml/types/seq_vector.h>

#include <cmath>
#include <cstddef>
#include <memory>
#include <netcdf>
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
 * See
 * https://ferret.pmel.noaa.gov/Ferret/documentation/coards-netcdf-conventions
 * for more information.
 */
template <int NUM_DIMS>
class netcdf_coards : public gen_grid<NUM_DIMS> {
   private:
    /**
     * Construct a seq_vector from NetCDF dimension name.
     * Inspects the data to see if seq_linear or seq_log can be
     * used to optimize the performance of this dimension->
     *
     * @param  file         NetCDF file to process.
     * @param  dimension    Name of NetCDF dimension.
     * @return              Sequence vector equivalent.
     */
    seq_vector::csptr make_axis(netCDF::NcFile& file, const std::string& name) {
        // search for this axis in the NetCDF file
        netCDF::NcVar axis = file.getVar(name);

        // convert to std::vector
        const size_t N = axis.getDim(0).getSize();
        std::vector<double> data(N);
        axis.getVar(data.data());

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
    netcdf_coards(netCDF::NcFile& file, const std::string& name,
                  bool read_fill = false) {
        this->_zero = 0.0;  // avoid uninitialized values in gen_grid class

        // search for this grid in the NetCDF file

        netCDF::NcVar variable = file.getVar(name);

        // read axis data from NetCDF file.

        size_t N = 1;
        for (int n = 0; n < NUM_DIMS; ++n) {
            seq_vector::csptr ax =
                make_axis(file, variable.getDim(n).getName());
            this->_axis[n] = ax;
            N *= this->_axis[n]->size();
        }

        // extract missing attribute information

        double missing = NAN;  // default value for missing data
        double filling = NAN;  // default for fill value
        {
            // NOLINTBEGIN(bugprone-empty-catch)
            netCDF::NcVarAtt att;

            // extract scale factor attribute
            try {
                att = variable.getAtt("missing_value");
                att.getValues(&missing);
            } catch (const netCDF::exceptions::NcException& ex) {
            }

            // extract fill value attribute
            if (read_fill) {
                try {
                    att = variable.getAtt("_FillValue");
                    att.getValues(&filling);
                } catch (const netCDF::exceptions::NcException& ex) {
                }
            }
            // NOLINTEND(bugprone-empty-catch)
        }

        // copy interpolant data from the NetCDF file into local memory.
        // replace missing data with fill value

        double* data = new double[N];
        this->_writeable_data = std::shared_ptr<double[]>(data);
        this->_data = this->_writeable_data;
        variable.getVar(data);

        for (size_t n = 0; n < N; ++n) {
            if (!std::isnan(missing)) {
                if (data[n] == missing) {
                    data[n] = filling;
                }
            }
        }
    }
};

/// @}
}  // end of namespace netcdf
}  // end of namespace usml
