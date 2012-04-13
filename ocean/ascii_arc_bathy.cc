/**
 * @file ascii_arc_bathy.cc
 * Extracts bathymetry data from from ASCII files with an ARC header.
 */
#include <iostream>
#include <fstream>
#include <usml/ocean/ascii_arc_bathy.h>

using namespace usml::ocean ;

/**
 * Load bathymetry from disk.
 */
ascii_arc_bathy::ascii_arc_bathy( const char* filename, double earth_radius )
{
    int ncols, nrows ;
    double xllcenter, yllcenter, cellsize, nodata_value ;
    static char label[80] ;
    const float R = (float) earth_radius ;

    std::ifstream fi(filename);
    fi >> std::skipws ;

    // read the file header

    fi >> label >> ncols ;
    fi >> label >> nrows ;
    fi >> label >> xllcenter ;
    fi >> label >> yllcenter ;
    fi >> label >> cellsize ;
    fi >> label >> nodata_value ;

    // construct latitude and longitude axes in spherical coordinates

    this->_axis[0] = new seq_linear(
        to_colatitude(yllcenter),
        to_radians(-cellsize),
        nrows );
    this->_axis[1] = new seq_linear(
        to_radians(xllcenter),
        to_radians(cellsize),
        ncols );

    // depth depths and convert to rho coordinate of spherical earth system

    this->_data = new float[ ncols * nrows ] ;
    float* ptr = this->_data ;
    for ( int r=0 ; r < nrows ; ++r ) {
        for ( int c=0 ; c < ncols ; ++c ) {
            fi >> *ptr ;
            *(ptr++) += R ;
        }
    }
}
