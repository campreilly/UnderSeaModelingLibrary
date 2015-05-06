/*
 * @file seq_vector.cc
 * A read-only, monotonic sequence of values.
 */
#include <usml/types/seq_vector.h>

#include <boost/foreach.hpp>
#include <usml/types/seq_data.h>
#include <usml/types/seq_linear.h>
#include <usml/types/seq_log.h>

using namespace usml::types;

/**
 * Clips the current seq_vector based on the intersection of the min
 * and max values provided. Creates and returns new seq_vector using
 * the build_best method. Caller is responsible to delete the returned
 * pointer.
 */
seq_vector::self_type* seq_vector::clip(double min, double max) const {

    size_t index = 0;
    double* data = new double[size() - 1];
    BOOST_FOREACH(double value, *this) {
        if ( ( value >= min ) && ( value <= max ) ) {
            data[index] = value;
             ++index;
        }
    }
    self_type* seq = build_best(data, index);
    delete[] data;
    return seq;
}

/**
 * Builds the best seq_vector pointer from seq_data, seq_linear or seq_log
 * based on the contents of the array.
 */
seq_vector::self_type* seq_vector::build_best(double* data, size_t count) {

    bool isLinear = true;
    bool isLog = true;
   
    const int N = ( int ) count;
    double p1 = data[0];
    double minValue = p1;
    double maxValue = p1;

    if ( N > 1 ) {
        double p2 = data[1];
        maxValue = p2;
        for ( int n = 2; n < N; ++n ) {
            double p3 = data[n];
            maxValue = p3;
            if ( abs(( ( p3 - p2 ) - ( p2 - p1 ) ) / p2) > 1E-4 ) {
                isLinear = false;
            }
            if ( p1 == 0.0 || p2 == 0.0 ||
                abs(( p3 / p2 ) - ( p2 / p1 )) > 1E-5 ) {
                isLog = false;
            }
            if ( !( isLinear || isLog ) ) break;
            p1 = p2;
            p2 = p3;
        }
    }

    // build a new seq_vector sub type
    if ( isLinear ) {
        return new seq_linear(minValue, ( maxValue - minValue ) / ( N - 1 ), N);
    }
    else if ( isLog ) {
        return new seq_log(minValue, ( maxValue / minValue ) / ( N - 1 ), N);
    }
    return new seq_data(data, count);
}

/**
 * Builds the best seq_vector pointer from seq_data, seq_linear or seq_log
 * based on the contents of the ublas vector.
 */
seq_vector::self_type* seq_vector::build_best(boost::numeric::ublas::vector<double> data) {
 
    double* data_array = new double[data.size()-1];
    size_t index = 0;
    BOOST_FOREACH(double value, data) {
        data_array[index] = value;
         ++index;
    }
    self_type* seq = build_best(data_array, index);
    delete[] data_array;
    return seq;
}

/**
 * Builds the best seq_vector pointer from seq_data, seq_linear or seq_log
 * based on the contents of the std::vector.
 */
seq_vector::self_type* seq_vector::build_best(std::vector<double> data) {

    double* data_array = new double[data.size()-1];
    size_t index = 0;
    BOOST_FOREACH(double value, data) {
        data_array[index] = value;
        ++index;
    }
    self_type* seq = build_best(data_array, index);
    delete[] data_array;
    return seq;
}
