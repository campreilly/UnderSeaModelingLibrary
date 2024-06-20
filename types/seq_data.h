/**
 * @file seq_data.h
 * Sequence defined by an unevenly spaced vector of points.
 */
#pragma once

#include <usml/types/seq_vector.h>

#include <boost/numeric/ublas/vector.hpp>
#include <map>
#include <stdexcept>

namespace usml {
namespace types {

using boost::numeric::ublas::vector;

/// @ingroup data_grid
/// @{

/**
 * Sequence defined by an unevenly spaced vector of points.
 * This class has much worse performance than seq_linear or seq_log
 * and should be avoided when one of those classes could be used instead.
 * But, some grids are just not defined using an evenly spaced sequence
 * of points and this class is needed for completeness.
 *
 * The find_index() routine uses the map::upper_bound() function to lookup
 * the bounding indices for each value.
 */
class USML_DECLSPEC seq_data : public seq_vector {
   public:
    /**
     * Construct sequence from any object that supports operator[].
     * Ensures that the sequence is a monotonic sequence.
     *
     * @param  data               Set of data elements to use.
     * @param  size               Number of elements in data.
     * @throws invalid_argument   If series not monotonic
     */
    template <class Container>
    seq_data(Container data, size_type size) : seq_data(size) {
        init(data, size);
    }

    /**
     * Construct sequence from any object that supports both operator[] and
     * size(). Ensures that the sequence is a monotonic sequence.
     *
     * @param  data               Set of data elements to use.
     * @throws invalid_argument   If series not monotonic
     */
    template <class Container>
    seq_data(Container data) : seq_data(data, data.size()) {}

    /**
     * Quickly search for the interpolation grid index for a value.
     * Normally, this is the index of the sequence member less than or
     * equal to provided value. For sequences with negative increments,
     * it is the sequence member greater than or equal to provided value.
     * Legal values are limited to the range [0,size-2] to ensure that
     * there is always a sequence member to the "right" of the returned index.
     *
     * @param   value       Value of the element to find.
     * @return              Index of the largest value that is not greater
     *                      than the argument.
     */
    size_type find_index(value_type value) const override;

   protected:
    /**
     * Initialize sequence sub-class using number of elements.
     *
     * @param  size       Length of the sequence to create.
     */
    seq_data(size_type size) : seq_vector(size) { _sign = 1.0; }

    /**
     * Initialize sequence from any object that supports operator[].
     * Ensures that the sequence is a monotonic sequence.
     *
     * @param  data               Set of data elements to use.
     * @param  size               Number of elements in data.
     * @throws invalid_argument   If series not monotonic
     */
    template <class Container>
    void init(Container data, size_type size) {
        if (size < 2) {
            _data[0] = value_type(data[0]);
            _increment[0] = 0.0;
            _data_map[_sign * value_type(data[0])] = 0;

        } else {
            // process first element

            value_type left = value_type(data[1]) - value_type(data[0]);
            if (left < 0) {
                _sign = -1.0;
            }
            _data[0] = value_type(data[0]);
            _increment[0] = left;
            _data_map[_sign * _data[0]] = 0;

            // process remaining elements

            for (size_type n = 1; n < size; ++n) {
                value_type right = left;
                if (n < size - 1) {
                    right = value_type(data[n + 1]) - value_type(data[n]);
                    if (left * right <= 0) {  // detect change of sign
                        throw std::invalid_argument("series not monotonic");
                    }
                }
                left = right;
                _data[n] = value_type(data[n]);
                _increment[n] = left;
                _data_map[_sign * _data[n]] = n;
            }
        }
    }

    /// Allow lookup of index from value.
    std::map<value_type, size_type> _data_map;

    /// Sign value is +1 if the sequence is increasing, -1 if decreasing.
    value_type _sign = 1.0;

};  // end of class

/// @}
}  // end of namespace types
}  // end of namespace usml
