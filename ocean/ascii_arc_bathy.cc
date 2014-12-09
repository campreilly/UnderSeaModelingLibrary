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
    double xllcorner, yllcorner, cellsize, nodata_value ;
    static char label[80] ;
    const double R = (double) wposition::earth_radius ;

    std::ifstream fi(filename);
    fi >> std::skipws ;

    // read the file header

    fi >> label >> ncols ;
    fi >> label >> nrows ;
    fi >> label >> xllcorner ;
    fi >> label >> yllcorner ;
    fi >> label >> cellsize ;
    fi >> label >> nodata_value ;

    // construct latitude and longitude axes in spherical coordinates
    // note that axis[0] starts in the south and moves north

    this->_axis[0] = new seq_linear(
        to_colatitude(yllcorner+cellsize*(nrows-1)),
        to_radians(cellsize),
        nrows );
    this->_axis[1] = new seq_linear(
        to_radians(xllcorner),
        to_radians(cellsize),
        ncols );

    // read depths and convert to rho coordinate of spherical earth system
    // flip latitude direction upside down during the read.

    this->_data = new double[ ncols * nrows ] ;
    for ( int r=0 ; r < nrows ; ++r ) {
        double* ptr = &( this->_data[r*ncols] ) ;
        for ( int c=0 ; c < ncols ; ++c ) {
            fi >> *ptr ;
            *(ptr++) += R ;
        }
    }

    #ifdef USML_DEBUG
        std::ofstream of("usml_ascii_arc_raw.csv") ;
        for ( int e=0 ; e < nrows ; ++e ) {
            of << to_latitude((*(this->_axis[0]))[e])<< ",";
            for (int f=0 ; f < ncols ; ++f ) {
                of << this->_data[f+ncols*e] - wposition::earth_radius << ",";
            }
            of << endl;
        }
        for ( int k=0 ; k < ncols ; ++k ) {
            if(k==0) {of << ",";}
            of << to_degrees((*(this->_axis[1]))[k]) << ",";
            if(k==ncols) {of << endl;}
        }
    #endif
}
