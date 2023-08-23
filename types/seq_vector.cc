/*
 * @file seq_vector.cc
 * A read-only, monotonic sequence of values.
 */

#include <usml/types/seq_data.h>
#include <usml/types/seq_linear.h>
#include <usml/types/seq_log.h>
#include <usml/types/seq_vector.h>

#include <boost/numeric/ublas/storage.hpp>
#include <cstdlib>

using namespace usml::types;

/**
 * Search for a value in this sequence. If the value is outside of the
 * legal range, the index for the nearest endpoint will be returned.
 *
 * @param   value       Value of the element to find.
 * @return              Index of the value that is closest.
 */
seq_vector::size_type seq_vector::find_nearest(value_type value) const {
    size_type n = find_index(value);
    if (abs(_data[n + 1] - value) <= abs(_data[n] - value)) {
        n = n + 1;
    }
    return n;
}

/**
 * constructs clipped sequence from the current seq_vector::csptr
 */
seq_vector::csptr seq_vector::clip(double min, double max) const {
    size_t index = 0;
    std::vector<value_type> data(size());
    for (double value : *this) {
        if ((value >= min) && (value <= max)) {
            data[index] = value;
            ++index;
        }
    }
    return build_best(data.data(), index);
}

/**
 * Builds the best seq_vector::csptr
 * based on the contents of the array.
 */
seq_vector::csptr seq_vector::build_best(const double* data, size_t count) {
    bool isLinear = true;
    bool isLog = true;

    const int N = (int)count;
    double p1 = data[0];
    double minValue = p1;
    double maxValue = p1;

    if (N > 1) {
        double p2 = data[1];
        maxValue = p2;
        for (size_t n = 2; n < N; ++n) {
            double p3 = data[n];
            maxValue = p3;
            if (p2 != 0.0 && abs(((p3 - p2) - (p2 - p1)) / p2) > 1E-4) {
                isLinear = false;
            }
            if (p1 != 0.0 && p2 != 0.0 && abs((p3 / p2) - (p2 / p1)) > 1E-5) {
                isLog = false;
            }
            p1 = p2;
            p2 = p3;
        }
    }

    // build a new sequence vector

    const seq_vector* seq;
    if (N < 2) {
        seq = new seq_linear(minValue, 1.0, 1);
    } else if (isLinear) {
        seq = new seq_linear(minValue, (maxValue - minValue) / (N - 1), N);
    } else if (isLog) {
        seq = new seq_log(minValue, (maxValue / minValue) / (N - 1), N);
    } else {
        seq = new seq_data(data, count);
    }
    return seq_vector::csptr(seq);
}

/**
 * Builds the best seq_vector::csptr
 * based on the contents of a ublas vector.
 */
seq_vector::csptr seq_vector::build_best(
    const boost::numeric::ublas::vector<double>& data) {
    return build_best(data.data().begin(), data.size());
}

/**
 * Builds the best seq_vector::csptr
 * based on the contents of the std::vector.
 */
seq_vector::csptr seq_vector::build_best(const std::vector<double>& data) {
    return build_best(data.data(), data.size());
}
