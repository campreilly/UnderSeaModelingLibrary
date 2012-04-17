/**
 * @file ascii_profile_factory.cc
 * Read a 1-D profile from a comma separated values (CSV) file.
 */
#include <iostream>
#include <fstream>
#include <list>
#include <usml/ocean/ascii_profile_factory.h>
#include <usml/ocean/profile_grid.h>

using namespace usml::ocean ;

/**
 * Construct a 1-D profile from an ASCII text file.
 */
profile_model* ascii_profile_factory::read( const char* filename ) {
    std::cout << filename << std::endl ;
    // static char label[80] ;

    // read data into temporary lists

    std::ifstream fi(filename);
    fi >> std::skipws ;
    std::list<double> numbers ;
    while( fi.good() ) {
        char c = fi.peek() ;
        if ( (c >= '0') && (c <= '9') ) {
            double x ;
            fi >> x ;
            numbers.push_back(x) ;
            std::cout << x << std::endl ;
        } else {
            fi >> c ;
        }
 //       depth.push_back( wposition::earth_radius - abs(d) ) ;
 //       speed.push_back( abs(s) ) ;
    }
    fi.close() ;
/*
    // create depth axis
    // @todo Fix the memory leaks that dpt[] creates.

    std::list<double>::iterator it_depth = depth.begin() ;
    double *dpt = new double[depth.size()] ;
    for ( unsigned n=0 ; n < depth.size() ; ++n ) {
        dpt[n] = *(it_depth++) ;
    }
    seq_vector* axis[1] ;
    axis[0] = new seq_data( dpt, depth.size() ) ;

    // add speed data to the profile

    data_grid<double,1>* grid = new data_grid<double,1>(axis) ;
    grid->interp_type(0,GRID_INTERP_PCHIP);
    std::list<double>::iterator it_speed = speed.begin() ;
    for ( unsigned n=0 ; n < speed.size() ; ++n ) {
        grid->data(&n, *(it_speed++) ) ;
    }

    profile_model* profile = new profile_grid<double,1>( grid ) ;
    return profile ;
    */
    return 0 ;
}
