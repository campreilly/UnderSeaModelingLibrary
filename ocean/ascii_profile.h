/** 
 * @file ascii_profile.h
 * Read a 1-D profile from a text file.
 */
#pragma once

#include <usml/ocean/ocean.h>

namespace usml {
namespace ocean {

/// @ingroup profiles
/// @{

/**
 * Read a 1-D profile from a text file.  The is often used to read
 * tables and CSV files from other applications.
 */
class USML_DECLSPEC ascii_profile : public data_grid<double,1> {

  public:

    //**************************************************
    // initialization

    /**
     * Read a 1-D profile from a file.
     *
     * @param filename  File to be named.
     */
    ascii_profile( const char* filename ) ;

};

}  // end of namespace ocean
}  // end of namespace usml
