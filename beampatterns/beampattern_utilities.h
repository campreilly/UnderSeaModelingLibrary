/**
 * @file beampattern_utilities.h
 * A collection of utilities for helping to build and configure beampatterns.
 */
#pragma once

#include <usml/types/types.h>

namespace usml {
namespace beampatterns {

using namespace usml::types;

/// @ingroup beampatterns
/// @{

/**
 * @ingroup beampatterns
 * @{
 * @defgroup beampattern_utilities Beampattern Utilities
 * @{
 * A collection of utilities for helping to build and configure beampatterns
 * @}
 * @}
 *
 * @ingroup beampattern_utilities
 * @{
 */

/**
 * Provides the element locations of a circular planar array on the
 * front/right plane
 *
 * @param radii			    The radius of each ring
 * @param num_elements	    The number of elements in each ring
 * @param offsets		    The offset, in radians, for the offset of the
 * 						    first element of each ring
 * @param elem_locations    The returned element locations in meters in an
 * 						    Nx3 matrix where N is the sum of the elements
 * 						    in num_elements
 */
void bp_con_ring(vector<double> radii, vector<int> num_elements,
        vector<double> offsets, matrix<double> *elem_locations = nullptr);

/**
 * Provides the element locations of a uniformly spaced array in
 * 3 dimensions
 *
 * @param num_e_front	    Number of elements in the front/back direction
 * @param spacing_front	    Spacing, in meters, in the front/back direction
 * @param num_e_right        Number of elements in the right/left direction
 * @param spacing_right      Spacing, in meters, in the right/left direction
 * @param num_e_up          Number of elements in the up/down direction
 * @param spacing_up        Spacing, in meters, in the up/down direction
 * @param elem_locations    The returned element locations in meters in an
 * 						    Nx3 matrix where N is the sum of the elements
 * 						    in num_elements. In front-right-up order
 */
void bp_con_uniform(int num_e_front, double spacing_front,
        int num_e_right, double spacing_right, int num_e_up, double spacing_up,
        matrix<double> *elem_locations = nullptr);

/// @}
}
}
