/**
 * @file ascii_profile.h
 * Read a 1-D profile from a text file.
 */
#pragma once

#include <usml/types/gen_grid.h>
#include <usml/usml_config.h>

namespace usml {
namespace ocean {

using namespace usml::types;

/// @ingroup profiles
/// @{

/**
 * Read a 1-D profile from a text file.  The is often used to read
 * tables and CSV files from other applications.
 */
class USML_DECLSPEC ascii_profile : public gen_grid<1> {
   public:
    /**
     * Read a 1-D profile from a file.
     *
     * @param filename  File to be named.
     */
    ascii_profile(const char* filename);
};

}  // end of namespace ocean
}  // end of namespace usml
