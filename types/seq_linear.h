/**
 * @file seq_linear.h
 * Sequence defined by an evenly spaced grid of points.
 */
#pragma once

#include <usml/types/seq_vector.h>

namespace usml {
namespace types {

/// @ingroup data_grid
/// @{

/**
 * Sequence defined by an evenly spaced grid of points.
 * Designed to serve the same function as the a=1:2:N Matlab construct.
 */
class USML_DECLSPEC seq_linear : public seq_vector {
   public:
    /**
     * Construct sequence using first value, increment/last, and size.
     *
     * @param  first        Value at start of sequence.
     * @param  temp         Increment or the last value in the sequence.
     * @param  size         Number of elements in this sequence.
     * @param  first_last   if true, the second constructor element is the last
     * value in the sequence, otherwise it is the increment value.
     */
    seq_linear(value_type first, value_type temp, size_type size,
               bool first_last = false)
        : seq_vector(size) {
        value_type increment;
        increment = first_last ? (temp - first) / (size - 1) : temp;
        initialize(first, increment, size);
    }

    /**
     * Construct sequence using first value, increment, and size.
     *
     * @param  first        Value at start of sequence.
     * @param  increment    Spacing between elements.
     * @param  size         Number of elements in this sequence.
     */
    seq_linear(value_type first, value_type increment, int size)
        : seq_vector((size_type)size) {
        initialize(first, increment, (size_type)size);
    }

    /**
     * Construct sequence using first value, increment value,
     * and last value. If last value is not part of the sequence,
     * the sequence will end on a value <= last.  All sequences
     * constructed in this way have at least a single value.
     *
     * @param  first        Value at start of sequence.
     * @param  increment    Spacing between elements.
     * @param  last         Value at end of sequence.
     */
    seq_linear(value_type first, value_type increment, value_type last)
        : seq_vector((increment == 0.0)
                         ? 1
                         : max((difference_type)1,
                               (difference_type)floor(
                                   1.0 + (last + increment / 10 - first) /
                                             increment))) {
        initialize(first, increment, size());
    }

    /**
     * Copies data from another sequence object.
     *
     * @param  copy         The object to be copied.
     */
    seq_linear(const seq_linear& copy) : seq_vector(copy) {}

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
    size_type find_index(value_type value) const override {
        return (size_type)max(
            (difference_type)0,
            min((difference_type)this->size() - 2,
                (difference_type)floor((value - _data[0]) / _increment[0])));
    }

   private:
    /**
     * Construct sequence using first value, increment, and size.
     *
     * @param  first        Value at start of sequence.
     * @param  increment    Spacing between elements.
     * @param  size         Number of elements in this sequence.
     */
    void initialize(value_type first, value_type increment, size_type size) {
        value_type v = first;
        for (size_type n = 0; n < size; ++n) {
            _data[n] = v;
            _increment[n] = increment;
            v += increment;
        }
    }
};

/// @}
}  // end of namespace types
}  // end of namespace usml
