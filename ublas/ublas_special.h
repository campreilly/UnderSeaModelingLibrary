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
     * Used to take the element-wise product of two
     * vector of vectors.
     */
    template<class E1, class E2>
    struct component_prod{
        typedef E1			argument_type1 ;
        typedef E2			argument_type2 ;
        typedef E1			result_type ;

        static BOOST_UBLAS_INLINE
        result_type apply(argument_type1 t1, argument_type2 t2) {
            return element_prod( t1, t2 ) ;
        }
    };

    /**
     * Used to take the exponent of a vector of vectors.
     */
    template<class E>
    struct component_exp{
        typedef E	argument_type ;
        typedef E	result_type ;

        static BOOST_UBLAS_INLINE
        result_type apply(argument_type t) {
            return exp( t ) ;
        }
    };


    /**
     * Used to produce the determinant of a matrix. The input matrix
     * must be a 2x2.
     */
    template<class E>
    struct component_determinant{
        typedef E			argument_type ;
        typedef typename E::value_type			result_type ;

        static BOOST_UBLAS_INLINE
        result_type apply(argument_type t) {
            return t(0,0)*t(1,1) - t(0,1)*t(1,0) ;
        }
    };

/** OPERATIONS and SPECIALIZED CLASSES **/

    /**
     * Allows for the exponentiation of a vector of vectors.
     */
    template<class E> BOOST_UBLAS_INLINE
    typename vector_unary_traits<E, component_exp<E> >::result_type
    layer_exp( const vector_expression<E>& e ) {
        typedef typename vector_unary_traits<E, component_exp<E> >
            ::expression_type expression_type ;
        return expression_type( e() ) ;
    }

    /**
     * Allows for the product of two vector of vectors.
     */
    template<class E1, class E2> BOOST_UBLAS_INLINE
    typename vector_binary_traits<E1, E2, component_prod<typename E1::value_type,
        typename E2::value_type> >::result_type
    layer_prod( const vector_expression<E1>& e1, const vector_expression<E2>& e2 ) {
        typedef typename vector_binary_traits<E1, E2, component_prod<
            typename E1::value_type, typename E2::value_type> >::expression_type expression_type ;
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

            explicit
            vector_unary_special( const expression_type& e ) : __e(e) {}

            const expression_type& expression() const {
                return __e ;
            }

            const size_type size() const {
                return expression().size() ;
            }

            const_reference operator() (size_type i) const {
                return functor_type::apply( __e(i) ) ;
            }

            const_iterator begin() const {
                return expression().begin() ;
            }

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
    template<class E>
    typename vector_unary_special_traits<E,	component_determinant<typename E::value_type> >::result_type
    layer_determinant( const vector_expression<E>& e ) {
        typedef typename vector_unary_special_traits<E, component_determinant<
            typename E::value_type> >::expression_type expression_type ;
        return expression_type( e() ) ;
    }

}   // end of namespace ublas
}   // end of namespace usml

#endif
