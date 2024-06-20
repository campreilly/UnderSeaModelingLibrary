/*
 * @file seq_data.cc
 * Sequence defined by an unevenly spaced vector of points.
 */
#include <usml/types/seq_data.h>
#include <usml/types/seq_vector.h>

#include <stdexcept>

using namespace usml::types;

/**
 * Quickly search for the interpolation grid index for a value.
 */
seq_data::size_type seq_data::find_index(value_type value) const {
    value *= _sign;

    // special cases

    if (_max_index < 1) {
        return 0;
    }
    if (value <= _sign * _data[0]) {
        return 0;
    }
    if (value >= _sign * _data[_max_index - 1]) {
        return _max_index - 1;
    }

    // search for value in map

    auto upper = _data_map.upper_bound(value);
    assert((*upper).second < 1e6);

    return (*upper).second - 1;
}
