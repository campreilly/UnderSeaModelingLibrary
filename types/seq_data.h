/**
 * @file seq_data.h
 * Sequence defined by an unevenly spaced vector of points.
 */
#pragma once

#include <usml/types/seq_vector.h>

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
 * The find_index() routine in this implementation tries to speed up the
 * search by using the last search as the initial guess for the next search.
 */
class USML_DECLSPEC seq_data : public seq_vector {

    //**************************************************
    // type definisions

    typedef seq_data self_type;
public:
    typedef const vector_reference<const self_type> const_closure_type;
    typedef vector_reference<self_type> closure_type;

protected:

    /** index number from the last search */
    size_type _index;

    /** Value from the last search.  Speeds up redundant searches. */
    value_type _value;

    /** axis value that corresponds to _index */
    value_type _index_data;

    /** Sign value is 1 if the sequence is increasing, -1 if decreasing. */
    value_type _sign;

    //**************************************************
    // private functions

    /**
     *
     * @param  data		          Set of data elements to use.
     * @param  size               Number of elements in this set.
     * @throws invalid_argument   If series not monotonic
     */
    void init( const double* data, size_t size ) {
    }

    //**************************************************
    // virtual functions

public:

    /**
     * Search for a value in this sequence. If the value is outside of the
     * legal range, the index for the nearest endpoint will be returned,
     * unless the nearest endpoint is that last index, in which the second
     * to last index will be returned. This is to garuntee that there is
     * always an index to the right of the returned index.
     *
     * @param   value       Value of the element to find.
     * @return              Index of the largest value that is not greater
     *                      than the argument.
     */
    virtual size_type find_index(value_type value) {

        // Special case of data sets of size one
        if (_max_index == 0) {
            return 0;
        }

        // check for a redundant search
        if (value == _value) {
            return _index;
        }
        _value = value;

        // search backwards (toward the front)?

        value *= _sign;
        if (_index_data > value) {

            // Looping ends when the beginning of the list is hit or when
            // then new point is less than, or equal to, the search value.

            for (difference_type n = _index - 1; n >= 0 && _index_data > value; --n) {
                _index_data = _data(--_index) * _sign;
            }

            // search forwards (toward the back)?

        } else if (_index_data < value) {

            // Looping ends when the end of the list is hit or when
            // new point is greater than, or equal to, the search value.

            difference_type N = size() - 2;
            for (difference_type n = _index + 1; n <= N && _index_data < value; ++n) {
                _index_data = _data(++_index) * _sign;
            }

            // If new point is greater than the search value,
            // we've gone too far and we need to back up by one.

            if (_index_data > value && _index > 0) {
                _index_data = _data(--_index) * _sign;
            }

        }
        return _index;
    }


    //***************************************************************
    // constructors and destructors

    /** Virtual destructor. */
    virtual ~seq_data() {
    }

    /**
     * Initialize sequence sub-class using number of elements.
     *
     * @param  size		  Length of the sequence to create.
     */
    seq_data(size_type size) : seq_vector(size) {
    }

    /**
     * Construct sequence from a standard C array.
     * Ensures that the sequence is a monotonic sequence.
     *
     * @param  data		  Set of data elements to use.
     * @param  size               Number of elements in data.
     * @throws invalid_argument   If series not monotonic
     */
    seq_data( const double* data, size_t size ) : seq_vector( size ) {
        _index = 0 ;
        _value = data[0] ;
        _sign = 1.0;
        _data[0] = _value;
        if (_max_index == 0) {
            _increment[0] = 0.0 ;
        }
        else if (_max_index > 0) {

            // process first element

            value_type left = data[1] - data[0] ;
            if (left < 0) {
                _sign = -1.0;
            }
            _increment[0] = left;

            // process middle elements

            for ( size_type n = 1; n < _max_index; ++n ) {
                value_type right = data[n+1] - data[n] ;
                if (left * right <= 0) { // detect change of sign
                    throw std::invalid_argument("series not monotonic");
                }
                left = right;
                _data[n] = data[n];
                _increment[n] = left;
            }

            // process last element

            _data[ _max_index ] = data[ _max_index ];
            _increment[ _max_index ] = left;
        }
        _index_data = _data[_index] * _sign;
    }

    /**
     * Construct sequence from a uBLAS vector.
     * Ensures that the sequence is a monotonic sequence.
     *
     * @param  data		  Set of data elements to use.
     * @throws invalid_argument   If series not monotonic
     */
    template<class T, class A> seq_data( const vector<T,A> &data )
        : seq_vector( data.size() )
    {
        _index = 0 ;
        _value = data[0] ;
        _sign = 1.0;
        if (_max_index == 0) {
            _data[0] = data[0] ;
            _increment[0] = 0.0 ;
        }
        else if (_max_index > 0) {

            // process first element

            value_type left = data[1] - data[0] ;
            if (left < 0) {
                _sign = -1.0;
            }
            _data[0] = data[0] ;
            _increment[0] = left;

            // process middle elements

            for ( size_type n = 1; n < _max_index; ++n ) {
                value_type right = data[n+1] - data[n] ;
                if (left * right <= 0) { // detect change of sign
                    throw std::invalid_argument("series not monotonic");
                }
                left = right;
                _data[n] = data[n];
                _increment[n] = left;
            }

            // process last element

            _data[ _max_index ] = data[ _max_index ];
            _increment[ _max_index ] = left;
        }
        _index_data = _data[_index] * _sign;
    }

    /**
     * Copies data from another seq_data object.
     *
     * @param  copy         The object to be copied.
     */
    seq_data( const seq_data & copy ) :
        seq_vector(copy),
        _index(copy._index), _value(copy._value),
        _index_data(copy._index_data), _sign(copy._sign)
    {
    }

    /** Create a copy using a reference to the base class. */
    virtual seq_vector* clone() const {
        return new seq_data(*this);
    }

}; // end of class

/// @}
} // end of namespace types
} // end of namespace usml
