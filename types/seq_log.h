/**
 * @file seq_log.h
 * Sequence defined by a logarithmically spaced grid of points.
 */
#pragma once

#include <usml/types/seq_vector.h>

namespace usml {
namespace types {

/// @ingroup data_grid
/// @{

/**
 * Sequence defined by a logarithmically spaced grid of points.
 * Designed to be used as the frequency axis for acoustic properties.
 */
class USML_DECLSPEC seq_log : public seq_vector {
   public:
    /**
     * Construct sequence using first value, increment, and size.
     *
     * @param  first        Value at start of sequence.
     * @param  increment    Spacing between elements.
     * @param  size         Number of elements in this sequence.
     */
    seq_log(value_type first, value_type increment, size_type size)
        : seq_vector(size) {
        initialize(first, increment, size);
    }

    /**
     * Construct sequence using first value, increment, and size.
     *
     * @param  first        Value at start of sequence.
     * @param  increment    Spacing between elements.
     * @param  size         Number of elements in this sequence.
     */
    seq_log(value_type first, value_type increment, int size)
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
    seq_log(value_type first, value_type increment, value_type last)
        : seq_vector((increment == 1.0)
                         ? 1
                         : max((difference_type)1,
                               (difference_type)floor(
                                   1.0 + (log((last + increment / 10) / first) /
                                          log(increment))))) {
        initialize(first, increment, size());
    }

    /**
     * Copies data from another sequence object.
     *
     * @param  copy         The object to be copied.
     */
    seq_log(const seq_log &copy) : seq_vector(copy) {}

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
                (difference_type)floor(log(value / _data[0]) /
                                       log(_increment[1] / _increment[0]))));
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
            v *= increment;
            if (n > 0) {
                _increment[n - 1] = _data[n] - _data[n - 1];
                _increment[n] = _increment[n - 1];
            }
        }
    }
};

/// @}
}  // end of namespace types
}  // end of namespace usml
