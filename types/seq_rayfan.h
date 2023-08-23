/**
 * @file seq_rayfan.h
 * Defines a sequence of values that are tangentially spaced.
 */
#pragma once

#include <usml/types/seq_data.h>

namespace usml {
namespace types {

/// @ingroup data_grid
/// @{

/**
 * Defines a sequence of values that are tangentially spaced.
 * <pre>
 *      de = s * tan( u ) + de0
 *
 * where:
 *      u   = uniformly spaced sequence
 *      s   = spreading factor
 *      de  = resulting sequence of angles
 * </pre>
 * This spacing organizes the D/E angles of acoustic rays such that
 * they intersect horizontal planes with an even spacing in an
 * isovelocity environment.
 */
class USML_DECLSPEC seq_rayfan : public seq_data {
   public:
    /**
     * Construct sequence of tangentially spaced angles using
     * first value, last value, size, and center.
     *
     * @param  first        Angle at start of sequence (deg).
     * @param  last         Angle at end of sequence (deg).
     * @param  size         Number of elements in this sequence.
     * @param  center       Angle at which rays are densest (deg).
     * @param  spread       Spreading factor.
     */
    seq_rayfan(value_type first = -90.0, value_type last = 90.0,
               size_type size = 181, value_type center = 0.0,
               value_type spread = 6.0);
};

/// @}
}  // end of namespace types
}  // end of namespace usml
