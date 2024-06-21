/**
 * @file seq_vector.h
 * A read-only, monotonic sequence of values.
 */
#pragma once

#include <usml/ublas/ublas.h>

#include <boost/numeric/ublas/vector.hpp>

namespace usml {
namespace types {

using namespace usml::ublas;

/// @ingroup data_grid
/// @{

/**
 * Base class for read-only, monotonic sequence of values. Designed to be used
 * as an interpolation axis for a multi-dimensional data sets.
 * Fast interpolation algorithms require an ability to quickly
 * lookup an axis index appropriate given a floating point axis value.
 *
 * This class is modeled after ublas::vector and as such can be used
 * in a similar fashion to other ublas::vectors.
 */
class USML_DECLSPEC seq_vector : public vector_container<seq_vector> {
   public:
    typedef seq_vector self_type;
    typedef double value_type;
    typedef value_type reference;
    typedef const reference const_reference;
    typedef unbounded_array<value_type> array_type;
    typedef array_type::size_type size_type;
    typedef array_type::difference_type difference_type;
    typedef dense_tag storage_category;
    typedef indexed_const_iterator<self_type, dense_random_access_iterator_tag>
        const_iterator;
    typedef const_iterator iterator;
    typedef reverse_iterator_base<const_iterator> const_reverse_iterator;
    typedef const vector_reference<const self_type> const_closure_type;
    typedef vector_reference<self_type> closure_type;

    /// Shared pointer to constant version of this class.
    typedef std::shared_ptr<const seq_vector> csptr;

    /**
     * Virtual destructor
     */
    virtual ~seq_vector() {}

    /**
     * Quickly search for the interpolation grid index for a value.
     * Normally, this is the index of the sequence member less than or
     * equal to provided value. For sequences with negative increments,
     * it is the sequence member greater than or equal to provided value.
     * Legal values are limited to the range [0,size-2] to ensure that
     * there is always a sequence member to the "right" of the returned index.
     *
     * This fast lookup is the principle feature that distinguishes
     * seq_vector objects from ordinary vectors.
     *
     * @param   value       Value of the element to find.
     * @return              Index of the largest value that is not greater
     *                      than the argument.
     */
    virtual size_type find_index(value_type value) const = 0;

    /**
     * Search for a value in this sequence. If the value is outside of the
     * legal range, the index for the nearest endpoint will be returned.
     *
     * @param   value       Value of the element to find.
     * @return              Index of the value that is closest.
     */
    size_type find_nearest(value_type value) const;

    /**
     * Retrieves the value at a specified index in the sequence in the fastest
     * way possible. Problems will occur if the index is outside of the
     * range [0,size-1],.
     *
     * @param   index       The element number to retrieve (zero indexed).
     * @return              The value between the "index" element.
     */
    inline const_reference operator[](size_type index) const {
        return _data[index];
    }

    /**
     * Retrieves the value at a specified index in the sequence in the safest
     * way possible. If the index is outside of the range [0,size-1], the value
     * for the nearest endpoint will be returned.
     *
     * @param  index        The index of element to retrieve (zero indexed).
     * @return              The value at the indexed element.
     */
    inline const_reference operator()(size_type index) const {
        index = max((size_type)0, min(_max_index, index));
        return _data[index];
    }

    /**
     * Checks if the contents of seq_vector's lhs and rhs are equal,
     * that is, whether lhs.size() == rhs.size() and each element
     * in lhs compares equal with the element in rhs at the same position.
     *
     * @param  rhs  The reference to a seq_vector on rhs hand side.
     * @return true if the contents of the containers are equal, false otherwise
     */
    inline bool operator==(const self_type& rhs) const {
        if (size() != rhs.size()) {
            return false;
        }
        // Compare each data value in each seq_vector
        return (std::equal(begin(), end(), rhs.begin()));
    }

    /**
     * Contrapositive of operator==
     *
     * @param  rhs  The reference to a seq_vector on rhs hand side.
     * @return true if the contents of the containers are unequal, false
     * otherwise
     */
    inline bool operator!=(const self_type& rhs) const {
        return !operator==(rhs);
    }

    /**
     * Retrieves the increment between two elements in this sequence.
     * If the index is outside of the range [0,size-1], the value for
     * the nearest endpoint will be returned.
     *
     * @param   index       The element number to retrieve (zero indexed).
     * @return              The difference between the element at "index"
     *                      and the element at "index+1".
     */
    const_reference increment(size_type index) const {
        index = std::max((size_type)0, std::min(_max_index, index));
        return _increment[index];
    }

    /**
     * Returns the number of elements in this sequence.
     */
    size_type size() const { return _data.size(); }

    /**
     * Convert sequence into a normal C++ array
     */
    array_type data() const { return _data; }

    /**
     * Create const_iterator equivalent of an array index number.
     */
    const_iterator find(size_type i) const { return const_iterator(*this, i); }

    /**
     * Return iterator to start of sequence.
     */
    const_iterator begin() const { return find(0); }

    /**
     * Return iterator to end of sequence.
     */
    const_iterator end() const { return find(_data.size()); }

    /**
     * Return reverse iterator to end of sequence.
     */
    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(rbegin());
    }

    /**
     * Return reverse iterator to start of sequence.
     */
    const_reverse_iterator rend() const {
        return const_reverse_iterator(rend());
    }

    /**
     * Clips the current seq_vector based on the intersection of the min
     * and max values provided. Creates and returns new seq_vector using
     * the build_best method. Caller is responsible to delete the returned
     * pointer.
     *
     * @param   min       The element number to retrieve (zero indexed).
     * @param   max       The element number to retrieve (zero indexed).
     * @return  pointer to a new clipped seq_vector.
     */
    seq_vector::csptr clip(double min, double max) const;

    /**
     * Builds a new seq_vector from the contents of an array.
     * Selects the type of seq_vector sub-class that best matches the
     * structure of the data.
     *
     * @param   data      Pointer to an array of doubles
     * @param   count     Size of the array
     * @return  new instance of a seq_data, seq_linear, or seq_log
     */
    static seq_vector::csptr build_best(const double* data, size_t count);

    /**
     * Builds a new seq_vector from the contents of a uBLAS vector.
     * Selects the type of seq_vector sub-class that best matches the
     * structure of the data.
     *
     * @param   data      uBLAS vector for this data
     * @return  new instance of a seq_data, seq_linear, or seq_log
     */
    static seq_vector::csptr build_best(
        const boost::numeric::ublas::vector<double>& data);

    /**
     * Builds a new seq_vector from the contents of a standard library vector.
     * Selects the type of seq_vector sub-class that best matches the
     * structure of the data.
     *
     * @param   data      standard library vector for this data
     * @return  new instance of a seq_data, seq_linear, or seq_log
     */
    static seq_vector::csptr build_best(const std::vector<double>& data);

   protected:
    /**
     * Initializes data container.
     */
    seq_vector(size_type size)
        : vector_container<self_type>(),
          _data(size),
          _increment(size),
          _max_index(size - 1) {}

    /**
     * Copies data from another seq_vector object.
     *
     * @param  other         The object to be copied.
     */
    seq_vector(const self_type& other)
        : vector_container<self_type>(),
          _data(other._data),
          _increment(other._increment),
          _max_index(other._max_index) {}

    /// Cache of sequence values.
    array_type _data;

    /// Cache of increment values.
    array_type _increment;

    /// Largest valid index number (one less than size() of data).
    size_type _max_index;

};  // end of class

/// @}
}  // end of namespace types
}  // end of namespace usml
