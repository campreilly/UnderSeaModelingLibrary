/** 
 * @file ascii_profile_factory.h
 * Read a 1-D profile from an ASCII text file.
 */
#ifndef USML_OCEAN_ASCII_PROFILE_FACTORY_H
#define USML_OCEAN_ASCII_PROFILE_FACTORY_H

#include <usml/ocean/profile_model.h>

namespace usml {
namespace ocean {

/// @ingroup profiles
/// @{

/**
 * Read a 1-D profile from an ASCII text file.
 * Generally used to import experimental data.
 */
class USML_DECLSPEC ascii_profile_factory {

  public:

    /**
     * Construct a 1-D profile from an ASCII text file.
     * Ignores text and whitespace, which allows it to read
     * both comma separated value files, tab separated value files
     * and many other text formats.
     * Assumes that the first column is depth in meters.
     * Assumes that the second column is sound speed in meters/sec.
     * 
     * @param filename  Name of the CSV file to load.
     */
    static profile_model* read( const char* filename ) ;

};

/// @}
}  // end of namespace ocean
}  // end of namespace usml

#endif
