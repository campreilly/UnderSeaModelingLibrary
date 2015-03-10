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
    public:

        typedef seq_vector self_type ;
        typedef double  value_type ;
        typedef value_type  reference ;
        typedef const reference const_reference ;
        typedef value_type* value_ptr ;
        typedef vector<value_type>  vector_type ;
        typedef vector_type::array_type array_type ;
        typedef vector_type::const_iterator  const_iterator ;
        typedef vector<value_type>::storage_category storage_category ;
        typedef array_type::size_type  size_type ;
        typedef const value_type* iterator ;
        typedef array_type::reverse_iterator    reverse_iterator ;
        typedef array_type::difference_type  difference_type ;
        typedef const vector_reference<const self_type> const_closure_type ;
        typedef vector_reference<self_type> closure_type ;

        /**
         * Virtual destructor
         */
        virtual ~seq_vector() {}

        /** Create a copy using a reference to the base class. */
        virtual self_type* clone() const = 0 ;

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
        virtual size_type find_index( value_type value ) = 0 ;

        /**
         * Retrieves the value at a specified index in the sequence in the fastest
         * way possible. Problems will occur if the index is outside of the
         * range [0,size-1),
         *
         * @param   index       The element number to retrieve (zero indexed).
         * @return              The value between the "index" element.
         */
        reference operator[] ( size_type index ) const {
            return data () [index] ;
        }

        /**
         * Retrieves the value at a specified index in the sequence in the safest
         * way possible. If the index is outside of the range [0,size), the value
         * for the nearest endpoint will be returned.
         *
         * @param  index        The index of element to retrieve (zero indexed).
         * @return              The value at the indexed element.
         */
        const_reference operator () ( size_type index ) const {
            index = max( (size_type) 0, min(_max_index,index) ) ;
            return _data[index];
        }

        /**
         * Retrieves the increment between two elements in this sequence.
         * If the index is outside of the range [0,size-1), the value for
         * the nearest endpoint will be returned.
         *
         * @param   index       The element number to retrieve (zero indexed).
         * @return              The difference between the element at "index"
         *                      and the element at "index+1".
         */
        reference increment( size_type index ) const {
            index = std::max( (size_type)0, std::min(_max_index,index) ) ;
            return _increment[index] ;
        }

        /**
         * Returns the number of elements in this sequence.
         */
        size_type size() const {
            return data ().size() ;
        }

        /**
         * Convert sequence into a normal C++ array
         */
        array_type data() const {
            return _data ;
        }

        /**
         * Iterators needed for BOOST_FOREACH
         */
        iterator begin() const {
            return _data.begin() ;
        }

        iterator end() const {
            return _data.end() ;
        }

        /**
         * Reverse iterators needed for data_grid _axes call
         */
        reverse_iterator rbegin() const {
            return data ().rbegin() ;
        }

        reverse_iterator rend() const {
            return data ().rend() ;
        }

    protected:

        /**
         * Initializes data container
         */
        seq_vector( size_type size )
            : vector_container<self_type> (),
              _data(size), _increment(size),
              _max_index(size-1)
        {}

        /**
         * Copies data from another seq_vector object.
         *
         * @param  copy         The object to be copied.
         */
        seq_vector( const self_type& other )
            : vector_container<self_type> (),
              _data(other._data), _increment(other._increment),
              _max_index(other._max_index)
        {}

        /**
         * Cache of sequence values.
         */
        array_type _data ;

        /**
         * Cache of increment values.
         */
        array_type _increment ;

        /** Largest valid index number (one less than size() ). */
        size_type _max_index ;

}; // end of class

/// @}
} // end of namespace types
} // end of namespace usml
