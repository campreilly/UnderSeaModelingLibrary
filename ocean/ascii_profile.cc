/**
 * @file ascii_profile.cc
 * Read a 1-D profile from a text file.
 */

#include <usml/ocean/ascii_profile.h>
#include <usml/types/seq_data.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition.h>

#include <cctype>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

using namespace usml::ocean;

/**
 * Read a 1-D profile from a text file.
 */
ascii_profile::ascii_profile(const char *filename) {
    // count the number of lines in the input file

    std::ifstream infile(filename);
    size_t size = 0;
    std::string line;
    while (getline(infile, line)) {
        ++size;
    }
    infile.clear();
    infile.seekg(0);

    // read data from input file

    double depth;
    auto *height = new double[size];
    auto *speed = new double[size];
    int c;
    for (size_t n = 0; n < size; ++n) {
        infile >> depth;
        while (true) {  // skip comma and space delimiters
            c = infile.peek();
            if (c != ',' && (isspace(c) == 0)) {
                break;
            }
            infile.get();
        }
        infile >> speed[n];
        height[n] = wposition::earth_radius - depth;
    }

    // load into data_grid variables

    this->_axis[0] = seq_vector::csptr(new seq_data(height, size));
    this->_data = std::shared_ptr<const double>(speed);
    delete[] height;

    // set interp type and edge limit

    this->_interp_type[0] = interp_enum::pchip;
    this->_edge_limit[0] = true;
}
