/*
 * @file seq_rayfan.cc
 * Defines a sequence of values that are tangentially spaced.
 */
#include <usml/types/seq_rayfan.h>

using namespace usml::types;

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
seq_rayfan::seq_rayfan(value_type first, value_type last, size_type size,
                       value_type center, value_type spread)
    : seq_data(size) {
    // guarantee that first value is smallest

    if (first > last) {
        auto value = first;
        first = last;
        last = value;
    }

    // compute tangent spaced limits

    const double first_ang = atan((first - center) / spread);
    const double last_ang = atan((last - center) / spread);
    const double scale = (last_ang - first_ang) / double(size - 1);

    // fill vector with data

    vector<double> data_vec(size);
    for (size_type n = 0; n < size; ++n) {
        const double x = first_ang + scale * (double) n;
        data_vec[n] = center + tan(x) * spread;
    }

    // initialize seq_data structures

    init(data_vec, size);
}
