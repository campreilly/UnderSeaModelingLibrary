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
ascii_arc_bathy::ascii_arc_bathy( const char* filename )
{
    int ncols, nrows ;
    double xllcenter, yllcenter, cellsize, nodata_value ;
    static char label[80] ;
    const float R = (float) wposition::earth_radius ;

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
    // note that axis[0] starts in the south and moves north

    this->_axis[0] = new seq_linear(
        to_colatitude(yllcenter),
        to_radians(-cellsize),
        nrows );
    this->_axis[1] = new seq_linear(
        to_radians(xllcenter),
        to_radians(cellsize),
        ncols );

    // read depths and convert to rho coordinate of spherical earth system
    // flip latitude direction upside down during the read.

    this->_data = new float[ ncols * nrows ] ;
    for ( int r=nrows-1 ; r >= 0 ; --r ) {
        float* ptr = &( this->_data[r*ncols] ) ;
        for ( int c=0 ; c < ncols ; ++c ) {
            fi >> *ptr ;
            *(ptr++) += R ;
        }
    }
}
