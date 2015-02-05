/**
 * @file seq_vector.h
 * A read-only, monotonic sequence of values.
 */
#pragma once

#include <usml/ublas/ublas.h>

namespace usml {
namespace types {

using namespace usml::ublas;

/// @ingroup data_grid
/// @{

/**
 * A read-only, monotonic sequence of values.  Designed to be used
 * as an interpolation axis for a multi-dimensional data sets.
 * Fast interpolation algorithms require an ability to quickly
 * lookup an axis index appropriate given a floating point axis value.
 *
 * This class caches data values and increments in ublas::vector<> objects.
 * This allow frequently used operations (ex: operator[] and increment())
 * access to inlined methods and cached values instead of computed results.
 * Sub-classes often store an additional representation of this data
 * that is used to implement fast versions of the find_index() method.
 *
 * This design uses zero_vector<T> as a model of how to implement a read-only
 * uBLAS vector.  Implements subclass methods as virtual functions,
 * instead of traits, so that they can be resolved at run-time.  Run-time
 * resolution is critical to reading sequences from netCDF data files.
 */
class USML_DECLSPEC seq_vector: public vector_container<seq_vector>
{
    //**************************************************
    // type definitions

    typedef seq_vector self_type;
public:
    typedef double value_type;
    typedef vector<value_type>::size_type size_type;
    typedef vector<value_type>::difference_type difference_type;
    typedef vector<value_type>::const_reference const_reference;
    typedef vector<value_type>::const_reference reference;
    typedef vector<value_type>::array_type array_type;

    typedef const vector_reference<const self_type> const_closure_type;
    typedef vector_reference<self_type> closure_type;
    typedef vector<value_type>::storage_category storage_category;

    typedef vector<value_type>::const_iterator const_iterator;
    typedef reverse_iterator_base<const_iterator> const_reverse_iterator;

    //**************************************************
    // data property

protected:
    /**
     * Cache of sequence values.
     */
    vector<value_type> _data;

public:
    /**
     * Retrieves the value at a specified index in the sequence in the fastest
     * way possible. Problems will occur if the index is outside of the
     * range [0,size-1),
     *
     * @param   index       The element number to retrieve (zero indexed).
     * @return              The value between the "index" element.
     */
    inline value_type operator[](size_type index) const  {
        return _data[index];
    }

    /**
     * Retrieves the value at a specified index in the sequence in the safest
     * way possible. If the index is outside of the range [0,size), the value
     * for the nearest endpoint will be returned.
     *
     * @param  index        The index of element to retrieve (zero indexed).
     * @return              The value at the indexed element.
     */
    virtual value_type operator()(size_type index) const {
        index = max( (size_type) 0, min(_max_index,index) );
        return _data[index] ;
    }

    /**
     * Convert sequence into a normal C++ array,
     */
    const array_type &data () const {
        return _data.data() ;
    }

    //**************************************************
    // increment property

protected:
    /**
     * Cache of increment values.
     */
    vector<value_type> _increment;

public:

    /**
     * Retrieves the increment between two elements in this sequence.
     * If the index is outside of the range [0,size-1), the value for
     * the nearest endpoint will be returned.
     *
     * @param   index       The element number to retrieve (zero indexed).
     * @return              The difference between the element at "index"
     *                      and the element at "index+1".
     */
    inline value_type increment(size_type index) const {
        index = max( (size_type) 0, min(_max_index,index) );
        return _increment[index] ;
    }

    //**************************************************
    // size property

protected:
    /** Number of elements in the sequence. */
    size_type _size;

    /** Largest valid index number (one less than _size). */
    size_type _max_index ;

public:
    /**
     * Returns the number of elements in this sequence.
     */
    inline size_type size() const {
        return _size ;
    }

    //**************************************************
    // iterators

    /**
     * Create iterator for the beginning of sequence.
     * Required for inheritance from vector_container.
     */
    inline const_iterator begin() const {
        return _data.begin() ;
    }

    /**
     * Create iterator for the end of sequence.
     * Required for inheritance from vector_container.
     */
    inline const_iterator end() const {
        return _data.end() ;
    }

    /**
     * Create iterator for the beginning of reverse sequence.
     * Required for inheritance from vector_container.
     */
    inline const_reverse_iterator rbegin() const {
        return _data.rbegin() ;
    }

    /**
     * Create iterator for the end of reverse sequence.
     * Required for inheritance from vector_container.
     */
    inline const_reverse_iterator rend() const {
        return _data.rend() ;
    }

    /**
     * Create iterator for a random position in the sequence.
     * Required for inheritance from vector_container.
     */
    inline const_iterator find(size_type index) const {
        return _data.find(index) ;
    }

    //**************************************************
    // virtual functions

    /**
     * Search for a value in this sequence. If the value is outside of the
     * legal range, the index for the nearest endpoint will be returned.
     *
     * This reverse lookup is the principle feature that distinguishes
     * seq_vector objects from ordinary vectors.
     *
     * @param   value       Value of the element to find.
     * @return              Index of the largest value that is not greater
     *                      than the argument.
     */
    virtual size_type find_index(value_type value) = 0;

    //**************************************************
    // constructors and destructors

protected:
    /**
     * Initialize data caches.
     */
    seq_vector( size_type size ) :
        _data(size), _increment(size), _size(size), _max_index(size-1)
    {
    }

    /**
     * Copies data from another seq_vector object.
     *
     * @param  copy         The object to be copied.
     */
    seq_vector( const seq_vector & copy ) :
        _data(copy._data), _increment(copy._increment),
        _size(copy._size), _max_index(copy._max_index)
    {
    }

public:
    /** Virtual destructor. */
    virtual ~seq_vector()
    {
    }

    /** Create a copy using a reference to the base class. */
    virtual seq_vector* clone() const = 0;

}; // end of class

/// @}
} // end of namespace types
} // end of namespace usml
