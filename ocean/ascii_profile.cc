/** 
 * @file ascii_profile.cc
 * Read a 1-D profile from a text file.
 */
 
#include <iostream>
#include <fstream>
#include <string>
#include <ctype.h>
#include <usml/ocean/ascii_profile.h>

using namespace usml::ocean ;

/**
 * Read a 1-D profile from a text file.
 */
ascii_profile::ascii_profile( const char* filename ) {

    // count the number of lines in the input file

    std::ifstream infile(filename);
    unsigned size = 0 ;
    std::string line ;
    while ( getline(infile,line) ) ++size ;
    infile.clear() ;
    infile.seekg(0) ;

    // read data from input file

    double depth ;
    double *height = new double[size] ;
    double *speed = new double[size] ;
    char c ;
    for ( unsigned n=0 ; n < size ; ++n ) {
        infile >> depth ;
        while ( true ) {    // skip comma and space delimiters
            c = infile.peek() ;
            if ( c != ',' && ! isspace(c) ) break ;
            infile.get() ;
        }
        infile >> speed[n] ;
        height[n] = wposition::earth_radius - depth ;
    }

    // load into data_grid variables

    this->_axis[0] = new seq_data( height, size ) ;
    this->_data = speed ;
    delete[] height ;
}

