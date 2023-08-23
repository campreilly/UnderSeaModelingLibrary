/**
 * @file seq_augment.h
 * Augments a seq_vector::csptr
 */
#pragma once

#include <usml/types/seq_data.h>

namespace usml {
namespace types {

/// @ingroup data_grid
/// @{

/**
 * Augments a seq_vector::csptr
 * This augment seq_vector::csptr
 * eigenrays for a monostatic target/sensor scenario.
 */
class USML_DECLSPEC seq_augment : public seq_data {
   public:
    /**
     * Constructor
     * Takes an unaugmented seq_vector::csptr
     * linearly spaced rays to the edges of the vector. ie, if 6 rays
     * are to be added to the seq_vector::csptr
     * at the upper and lower extreme of the seq_vector::csptr
     *
     * @param origin    seq_vector::csptr
     * @param num_rays  number of rays to increase the vector by
     */
    seq_augment(seq_vector::csptr origin, size_t num_rays)
        : seq_data(origin->size() + num_rays) {
        size_t size_old(origin->size());
        size_t size(size_old + num_rays);
        //_index = 0 ;
        _sign = 1.0;
        _max_index = size - 1;
        _data.resize(size);
        _increment.resize(size);

        double first = (*origin)[0];
        //_value = _index_data = first ;
        size_t half = (size_t)floor(num_rays / 2.0) + 1;
        double spacing = origin->increment(0) / half;

        _data[0] = first;
        for (size_t i = 1; i < half + 1; ++i) {
            _data[i] = _data[i - 1] + spacing;
        }
        for (size_t i = half + 1, j = 2; j < size_old - 1; ++i, ++j) {
            _data[i] = (*origin)[j];
        }
        for (size_t i = size_old - 2 + half; i < size - 1; ++i) {
            _data[i] = _data[i - 1] + spacing;
        }
        _data[size - 1] = (*origin)[size_old - 1];

        for (size_t i = 1; i < size; ++i) {
            _increment[i - 1] = _data[i] - _data[i - 1];
        }
        _increment[_max_index] = _data[_max_index] - _data[_max_index - 1];
    }
};

/// @}
}  // end of namespace types
}  // end of namespace usml
