/**
 * @file ascii_arc_bathy.cc
 * Extracts bathymetry data from ASCII files with an ARC header.
 */

#include <usml/ocean/ascii_arc_bathy.h>
#include <usml/types/seq_linear.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition.h>
#include <usml/ublas/math_traits.h>

#include <cstddef>
#include <fstream>
#include <iostream>
#include <memory>

using namespace usml::ocean;
using namespace usml::types;

/**
 * Load bathymetry from disk.
 */
ascii_arc_bathy::ascii_arc_bathy(const char* filename) {
    size_t ncols;
    size_t nrows;
    double xllcorner;
    double yllcorner;
    double cellsize;
    double nodata_value;
    static char label[80];
    const auto R = (double)wposition::earth_radius;

    std::ifstream fi(filename);
    fi >> std::skipws;

    // read the file header

    fi >> label >> ncols;
    fi >> label >> nrows;
    fi >> label >> xllcorner;
    fi >> label >> yllcorner;
    fi >> label >> cellsize;
    fi >> label >> nodata_value;

    // construct latitude and longitude axes in spherical coordinates
    // note that axis[0] starts in the south and moves north

    this->_axis[0] = seq_vector::csptr(new seq_linear(
        to_colatitude(yllcorner + cellsize * (double)(nrows - 1)),
        to_radians(cellsize), nrows));
    this->_axis[1] = seq_vector::csptr(
        new seq_linear(to_radians(xllcorner), to_radians(cellsize), ncols));

    // read depths and convert to rho coordinate of spherical earth system
    // flip latitude direction upside down during the read.

    double* data = new double[ncols * nrows];
    for (int r = 0; r < nrows; ++r) {
        double* ptr = &(data[r * ncols]);
        for (int c = 0; c < ncols; ++c) {
            fi >> *ptr;
            *(ptr++) += R;
        }
    }
    this->_writeable_data = std::shared_ptr<double[]>(data);
    this->_data = this->_writeable_data;  // read only reference

    // set interp type and edge limit

    for (size_t n = 0; n < 2; ++n) {
        this->_interp_type[n] = interp_enum::pchip;
        this->_edge_limit[n] = true;
    }
}
