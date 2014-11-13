/**
 * @file ublas_special.h
 * The following functionals and operations are unique to usml.
 * They utilize the already in place properties of the vector/matrix
 * expression nature of ublas vector/matrices and perform specialized
 * tasks pertinent to usml.
 */

#ifndef USML_UBLAS_UBLAS_SPECIAL_H
#define USML_UBLAS_UBLAS_SPECIAL_H

#include <usml/ublas/math_traits.h>

namespace usml {
namespace ublas {

/** FUNCTIONALS **/

    /**
     * Used to take the modular of a vector.
     */
    template<class T1, class T2>
    struct scalar_fmod{
        typedef T1	argument_type1 ;
        typedef T2  argument_type2 ;
        typedef argument_type1	result_type ;

        static BOOST_UBLAS_INLINE
        result_type apply(argument_type1 t1, argument_type2 t2) {
            return fmod( t1, t2 ) ;
        }
    };

    /**
     * Used to take the element-wise product of two
     * vector of vectors.
     */
    template<class E1, class E2>
    struct vector_prod{
        typedef E1			argument_type1 ;
        typedef E2			argument_type2 ;
        typedef E1			result_type ;

        static BOOST_UBLAS_INLINE
        result_type apply(argument_type1 t1, argument_type2 t2) {
            return element_prod( t1, t2 ) ;
        }
    };

    /**
     * Used to take the element-wise product of two
     * vector of vectors.
     */
    template<class E1, class E2>
    struct vector_div{
        typedef E1			argument_type1 ;
        typedef E2			argument_type2 ;
        typedef E1			result_type ;

        static BOOST_UBLAS_INLINE
        result_type apply(argument_type1 t1, argument_type2 t2) {
            return element_div( t1, t2 ) ;
        }
    };


    /**
     * Used to take the element-wise product of two
     * vector of matrices.
     */
    template<class E1, class E2>
    struct matrix_prod{
        typedef E1			argument_type1 ;
        typedef E2			argument_type2 ;
        typedef E2			result_type ;           /// need to figure out how to return the correct size of matrix. does it matter?

        static BOOST_UBLAS_INLINE
        result_type apply(argument_type1 t1, argument_type2 t2) {
            return prod( t1, t2 ) ;
        }
    };

    /**
     * Used to take the exponent of a vector of vectors.
     */
    template<class E>
    struct element_exp{
        typedef E	argument_type ;
        typedef E	result_type ;

        static BOOST_UBLAS_INLINE
        result_type apply(argument_type t) {
            return exp( t ) ;
        }
    };

    /**
     * Used to take the exponent of a vector of vectors.
     */
    template<class E>
    struct element_trans{
        typedef E	argument_type ;
        typedef E	result_type ;

        static BOOST_UBLAS_INLINE
        result_type apply(argument_type t) {
            return trans( t ) ;
        }
    };

    /**
     * Used to produce the determinant of a matrix. The input matrix
     * must be a 2x2.
     */
    template<class E>
    struct element_determinant{
        typedef E			argument_type ;
        typedef typename E::value_type			result_type ;

        static BOOST_UBLAS_INLINE
        result_type apply(argument_type t) {
            return t(0,0)*t(1,1) - t(0,1)*t(1,0) ;
        }
    };

    /**
     * Returns the inverse of a 2x2 matrix. ONLY for 2x2 matricies
     */
    template<class E>
    struct matrix_inverse {
        typedef E		argument_type ;
        typedef argument_type	result_type ;
        typedef typename E::value_type	value_type ;

        static
        result_type apply(argument_type t) {
            argument_type tmp(t) ;
            value_type d = t(0,0)*t(1,1) - t(0,1)*t(1,0) ;
            tmp(0,0) = t(1,1) / d ;
            tmp(1,0) = -t(1,0) / d ;
            tmp(0,1) = -t(0,1) / d ;
            tmp(1,1) = t(0,0) / d ;
            return tmp ;
        }
    };

    /**
     * Allows access to elements of a matrix within a vector.
     */
    template<class E, std::size_t I1, std::size_t I2>
    struct nested_index {
        typedef E			argument_type ;
        typedef typename E::value_type		result_type ;

        static
        result_type apply(argument_type t) {
            return t(I1,I2) ;
        }
    };

    /**
     * Plus assign for nested vectors to another vector.
     */
    template<class E1, class E2>
    struct nested_plus_assign {
        typedef E1		argument_type1 ;
        typedef E2		argument_type2 ;
        typedef argument_type1	result_type ;
        typedef typename E1::size_type	size_type ;

        static BOOST_UBLAS_INLINE
        result_type apply(argument_type1 t1, argument_type2 t2) {
            size_type size( t2.size() ) ;
            for(size_type i=0; i<size; ++i)
                t1 += t2(i) ;
            return t1 ;
        }

    };

    /**
     * Allows direct calls of nested functors that apply to a vector and
     * vector<vector> or vector<matrix>.
     */
    template<template<class E1, class E2>class F, class V, class E>
    void nested_vector_assign( V& v, const vector_expression<E>& e) {
        typedef F<V,E>		functor_type ;
        typedef typename V::size_type size_type ;
        v = functor_type::apply( v, e () ) ;
    }

/** OPERATIONS and SPECIALIZED CLASSES **/

    /**
     * Allows for the exponentiation of a vector of vectors.
     */
    template<class E> BOOST_UBLAS_INLINE
    typename vector_unary_traits<E, element_exp<E> >::result_type
    nested_exp( const vector_expression<E>& e ) {
        typedef typename vector_unary_traits<E, element_exp<E> >
            ::expression_type expression_type ;
        return expression_type( e() ) ;
    }

    /**
     * Allows for the transposition of a vector of matrices.
     */
    template<class E> BOOST_UBLAS_INLINE
    typename vector_unary_traits<E, element_trans<E> >::result_type
    nested_trans( const vector_expression<E>& e ) {
        typedef typename vector_unary_traits<E, element_trans<E> >
            ::expression_type expression_type ;
        return expression_type( e() ) ;
    }

    /**
     * Produces the inverse matrix within a vector of matricies
     */
    template<class E>
    typename vector_unary_traits<E, matrix_inverse<typename E::value_type> >::result_type
    inverse( const vector_expression<E>& e ) {
        typedef typename vector_unary_traits<E, matrix_inverse<typename E::value_type> >::expression_type
            expression_type ;
        return expression_type( e() ) ;
    }

    /**
     * Allows for the fmod of a vector.
     */
    template<class E, class T> BOOST_UBLAS_INLINE
    typename vector_binary_scalar2_traits<E, T, scalar_fmod<typename E::value_type, T> >::result_type
    vector_fmod( const vector_expression<E>& e, const T& t ) {
        typedef typename vector_binary_scalar2_traits<E, T, scalar_fmod<
            typename E::value_type, T> >::expression_type expression_type ;
        return expression_type( e(), t ) ;
    }

    /**
     * Allows for the fmod of a vector.
     */
    template<class E, class T> BOOST_UBLAS_INLINE
    typename vector_binary_scalar2_traits<E, T, scalar_multiplies<typename E::value_type, T> >::result_type
    nested_scalar_prod( const vector_expression<E>& e, const T& t ) {
        typedef typename vector_binary_scalar2_traits<E, T, scalar_multiplies<
            typename E::value_type, T> >::expression_type expression_type ;
        return expression_type( e(), t ) ;
    }

    /**
     * Allows for the product of two vector of vectors.
     */
    template<class E1, class E2> BOOST_UBLAS_INLINE
    typename vector_binary_traits<E1, E2, vector_prod<typename E1::value_type,
        typename E2::value_type> >::result_type
    nested_vector_prod( const vector_expression<E1>& e1, const vector_expression<E2>& e2 ) {
        typedef typename vector_binary_traits<E1, E2, vector_prod<
            typename E1::value_type, typename E2::value_type> >::expression_type expression_type ;
        return expression_type( e1(), e2() ) ;
    }

    /**
     * Allows for the product of two vector of vectors.
     */
    template<class E1, class E2> BOOST_UBLAS_INLINE
    typename vector_binary_traits<E1, E2, vector_div<typename E1::value_type,
        typename E2::value_type> >::result_type
    nested_vector_div( const vector_expression<E1>& e1, const vector_expression<E2>& e2 ) {
        typedef typename vector_binary_traits<E1, E2, vector_div<
            typename E1::value_type, typename E2::value_type> >::expression_type expression_type ;
        return expression_type( e1(), e2() ) ;
    }

    /**
     * Allows for the product of two vector of matrices.
     */
    template<class E1, class E2> BOOST_UBLAS_INLINE
    typename vector_binary_traits<E1, E2, matrix_prod<E1,E2> >::result_type
    nested_matrix_prod( const vector_expression<E1>& e1, const vector_expression<E2>& e2 ) {
        typedef typename vector_binary_traits<E1, E2, matrix_prod<E1, E2> >
            ::expression_type expression_type ;
        return expression_type( e1(), e2() ) ;
    }


    /**
     * Specialized class similar to vector_unary, but the input class
     * and the output class differ. The input class is vector_expression<E>
     * while the output is vector_expression<typename E::value_type::value_type>.
     */
    template<class E, class F>
    class vector_unary_special :
        public vector_expression<vector_unary_special<E,F> >
    {
        public:
            typedef F	functor_type ;
            typedef E	expression_type ;
            typedef typename E::size_type	size_type ;
            typedef typename E::value_type::value_type	value_type ;
            typedef value_type	const_reference ;
            typedef typename E::iterator		iterator ;
            typedef typename E::const_iterator			const_iterator ;

            BOOST_UBLAS_INLINE
            explicit vector_unary_special( const expression_type& e ) :
                __e(e) {}

            BOOST_UBLAS_INLINE
            const expression_type& expression() const {
                return __e ;
            }

            BOOST_UBLAS_INLINE
            const size_type size() const {
                return expression().size() ;
            }

            BOOST_UBLAS_INLINE
            const_reference operator() (size_type i) const {
                return functor_type::apply( __e(i) ) ;
            }

            BOOST_UBLAS_INLINE
            const_iterator begin() const {
                return expression().begin() ;
            }

            BOOST_UBLAS_INLINE
            const_iterator end() const {
                return expression().end() ;
            }

        private:
            expression_type	__e ;
    };

    template<class E, class F>
    struct vector_unary_special_traits {
        typedef vector_unary_special<E,F>	result_type ;
        typedef result_type		expression_type ;
    };

    /**
     * Returns a vector that is the determinant of the vector of matrices that
     * this operation is called on.
     * determinant(v[i]) = v(determinant[i])
     */
    template<class E> BOOST_UBLAS_INLINE
    typename vector_unary_special_traits<E,	element_determinant<typename E::value_type> >::result_type
    nested_determinant( const vector_expression<E>& e ) {
        typedef typename vector_unary_special_traits<E, element_determinant<
            typename E::value_type> >::expression_type expression_type ;
        return expression_type( e() ) ;
    }

    /**
     * Returns a vector of the elements of the vector's entries' index I1,I2.
     * v[i] = v[i](I1,I2)
     */
    template<class E, std::size_t I1, std::size_t I2>
    typename vector_unary_special_traits<E, nested_index<typename E::value_type, I1, I2> >::result_type
    nested_access( const vector_expression<E>& e ) {
        typedef typename vector_unary_special_traits<E, nested_index<typename E::value_type,
            I1, I2> >::expression_type	expression_type ;
        return expression_type( e() ) ;
    }

}   // end of namespace ublas
}   // end of namespace usml

#endif
