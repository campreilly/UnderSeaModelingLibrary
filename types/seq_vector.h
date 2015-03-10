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
        typedef array_type::iterator iterator ;
        typedef array_type::reverse_iterator    reverse_iterator ;
        typedef array_type::difference_type  difference_type ;
        typedef const vector_reference<const self_type> const_closure_type ;
        typedef vector_reference<self_type> closure_type ;

        /**
         * Destructor
         */
        virtual ~seq_vector() {}

        /**
         * Clone function
         */
        virtual self_type* clone() const = 0 ;

        /**
         * Search for a value in the data container, or the nearest
         * endpoint.
         */
        virtual size_type find_index( value_type value ) = 0 ;

        /**
         * Element Accessors
         */
        reference operator[] ( size_type i ) const {
            return data () [i] ;
        }

        reference data( size_type i ) const {
            return data () [i] ;
        }

        const_reference operator () ( size_type i ) const {
            return _data[i];
        }

        reference increment( size_type i ) const {
            i = std::max( (size_type)0, std::min(_max_index,i) ) ;
            return _increment[i] ;
        }

        /**
         * size of data
         */
        size_type size() const {
            return data ().size() ;
        }

        /**
         * reference to the array for data
         */
        array_type data() const {
            return _data ;
        }

        /**
         * Iterators needed for BOOST_FOREACH
         */
        iterator begin() const {
            return data ().begin() ;
        }

        iterator end() const {
            return data ().end() ;
        }

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
         * Copy Constructor
         */
        seq_vector( const self_type& other )
            : _data(other._data), _increment(other._increment),
              _max_index(other._max_index)
        {}

        /**
         * Data cache
         */
        array_type _data ;
        array_type _increment ;
        size_type _max_index ;

}; // end of class

/// @}
} // end of namespace types
} // end of namespace usml
