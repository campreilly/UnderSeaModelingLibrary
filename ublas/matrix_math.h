/**
 * @ingroup vecmath
 * @{
 * @file matrix_math.h
 *
 * Matrix extensions to the uBLAS math library that create new math 
 * algorithms for scientific applications.  The new math operations are:
 * 
 *    - scalar addition: matrix +/- scalar, scalar +/- matrix
 *    - division of scalar by matrix
 *    - limiting functions: max(), min(), floor(), ceil()
 *    - algebraic functions: abs(), abs2(), arg(), sqrt(), copysign()
 *    - trigonometric functions: cos(), cosh(), sin(), sinh(), tan(), tanh()
 *    - inverse trig functions: acos(), acosh(), asin(), asinh(), 
 *              atan(), atan2(), atanh()
 *    - exponential functions: exp(), log(), log10(), pow()
 *    - signal processing functions: signal(), asignal()
 * 
 * These operations apply to real and complex matrices in either 
 * double or single precision format.  The details of the mathematics
 * are hidden inside of the scalar functors defined in scalar_math.h
 * and the math traits defined in math_traits.h.
 */
#ifndef USML_UBLAS_MATRIX_MATH_H
#define USML_UBLAS_MATRIX_MATH_H

#include <usml/ublas/scalar_math.h>

namespace usml {
namespace ublas {

    //**********************************************************
    // scalar addition

    /**
     * Add matrix (v) and scalar (t) such that (v+t)[i] = v[i] + t.
     * Assumes that the scalar type is the same as that stored in the matrix.
     */
    template<class E> inline
        typename matrix_binary_scalar2_traits<E, typename E::value_type,
    	scalar_plus<typename E::value_type, typename E::value_type> 
    	>::result_type
    operator+( const matrix_expression<E> &e1, typename E::value_type e2 ) {
        typedef typename matrix_binary_scalar2_traits<E,typename E::value_type,
	    scalar_plus<typename E::value_type, typename E::value_type> 
	    >::expression_type expression_type;
        return expression_type (e1(), e2);
    }

    /**
     * Add scalar (t) and matrix (v) such that (t+v)[i] = t + v[i].
     * Assumes that the scalar type is the same as that stored in the matrix.
     */
    template<class E> inline
        typename matrix_binary_scalar1_traits<typename E::value_type, E,
    	scalar_plus<typename E::value_type, typename E::value_type> 
    	>::result_type
    operator+( typename E::value_type e1, const matrix_expression<E> &e2 ) {
        typedef typename matrix_binary_scalar1_traits<typename E::value_type,E,
	    scalar_plus<typename E::value_type, typename E::value_type> 
		>::expression_type expression_type;
        return expression_type (e1, e2());
    }

    /**
     * Subtract matrix (v) and scalar (t) such that (v-t)[i] = v[i] - t.
     * Assumes that the scalar type is the same as that stored in the matrix.
     */
    template<class E> inline
        typename matrix_binary_scalar2_traits<E, typename E::value_type,
    	scalar_minus<typename E::value_type, typename E::value_type> 
    	>::result_type
    operator-( const matrix_expression<E> &e1, typename E::value_type e2 ) {
        typedef typename matrix_binary_scalar2_traits<E,typename E::value_type,
	    scalar_minus<typename E::value_type, typename E::value_type> 
	    >::expression_type expression_type;
        return expression_type (e1(), e2);
    }

    /**
     * Subtract scalar (t) and matrix (v) such that (t-v)[i] = t - v[i].
     * Assumes that the scalar type is the same as that stored in the matrix.
     */
    template<class E> inline
       typename matrix_binary_scalar1_traits<typename E::value_type, E,
	   scalar_minus<typename E::value_type, typename E::value_type> 
	   >::result_type
    operator-( typename E::value_type e1, const matrix_expression<E> &e2 ) {
        typedef typename matrix_binary_scalar1_traits<typename E::value_type,E,
            scalar_minus<typename E::value_type, typename E::value_type> 
            >::expression_type expression_type;
        return expression_type (e1, e2());
    }

    //**********************************************************
    // scalar division

    /**
     * Divide scalar (t) and matrix (v) such that (t/v)[i] = t / v[i].
     * Assumes that the scalar type is the same as that stored in the matrix.
     */
    template<class T1, class E2> inline
        typename matrix_binary_scalar1_traits<const T1, E2, 
        scalar_divides<T1, typename E2::value_type> >::result_type
    operator / (const T1 &e1, const matrix_expression<E2> &e2) {
        typedef typename matrix_binary_scalar1_traits<const T1, E2, 
            scalar_divides<T1, typename E2::value_type> 
            >::expression_type expression_type;
        return expression_type (e1, e2 ());
    }

    //**********************************************************
    // limiting functions

    /**
     * Maximum between a matrix and a scalar.
     * Has the effect of clipping low values out of the matrix.
     * Not defined for complex matrices.
     */
    template<class E1, class E2> inline
        typename matrix_binary_scalar2_traits<E1, E2,
    	scalar_max<typename E1::value_type, E2> >::result_type
    max( const matrix_expression<E1> &e1, const E2 &e2 ) {
        typedef typename matrix_binary_scalar2_traits<E1, E2,
	    scalar_max<typename E1::value_type, E2> >::expression_type
	    expression_type;
        return expression_type (e1(), e2);
    }

    /**
     * Minimum between a matrix and a scalar.
     * Has the effect of clipping high values out of the matrix.
     * Not defined for complex matrices.
     */
    template<class E1, class E2> inline
        typename matrix_binary_scalar2_traits<E1, E2,
    	scalar_min<typename E1::value_type, E2> >::result_type
    min( const matrix_expression<E1> &e1, const E2 &e2 ) {
        typedef typename matrix_binary_scalar2_traits<E1, E2,
	    scalar_min<typename E1::value_type, E2> >::expression_type
	    expression_type;
        return expression_type (e1(), e2);
    }

    /**
     * Rounding elements of a matrix down to the nearest integer.
     * Not defined for complex matrices.
     */
    template<class E> inline
        typename matrix_unary1_traits<E,
        scalar_floor<typename E::value_type> >::result_type
    floor(const matrix_expression<E> &e) {
        typedef typename matrix_unary1_traits<E,
	    scalar_floor<typename E::value_type> >::expression_type
            expression_type;
        return expression_type( e() );
    }

    /**
     * Rounding elements of a matrix up to the nearest integer.
     * Not defined for complex matrices.
     */
    template<class E> inline
        typename matrix_unary1_traits<E,
    	scalar_ceil<typename E::value_type> >::result_type
    ceil(const matrix_expression<E> &e) {
        typedef typename matrix_unary1_traits<E,
	    scalar_ceil<typename E::value_type> >::expression_type
            expression_type;
        return expression_type( e() );
    }

    //**********************************************************
    // conversion functions

    /**
     * Convert angle value in radians to degrees.
     */
    template<class E> inline
        typename matrix_unary1_traits<E,
    	scalar_to_degrees<typename E::value_type> >::result_type
    to_degrees(const matrix_expression<E> &e) {
        typedef typename matrix_unary1_traits<E,
	    scalar_to_degrees<typename E::value_type> >::expression_type
            expression_type;
        return expression_type( e() );
    }

    /**
     * Convert angle value in degrees to radians.
     */
    template<class E> inline
        typename matrix_unary1_traits<E,
    	scalar_to_radians<typename E::value_type> >::result_type
    to_radians(const matrix_expression<E> &e) {
        typedef typename matrix_unary1_traits<E,
	    scalar_to_radians<typename E::value_type> >::expression_type
            expression_type;
        return expression_type( e() );
    }

    /**
     * Convert colatitude (radians from north) value to 
     * latitude (degrees from equator).
     */
    template<class E> inline
        typename matrix_unary1_traits<E,
    	scalar_to_latitude<typename E::value_type> >::result_type
    to_latitude(const matrix_expression<E> &e) {
        typedef typename matrix_unary1_traits<E,
	    scalar_to_latitude<typename E::value_type> >::expression_type
            expression_type;
        return expression_type( e() );
    }

    /**
     * Convert latitude (degrees from equator) value to 
     * colatitude (radians from north).
     */
    template<class E> inline
        typename matrix_unary1_traits<E,
    	scalar_to_colatitude<typename E::value_type> >::result_type
    to_colatitude(const matrix_expression<E> &e) {
        typedef typename matrix_unary1_traits<E,
	    scalar_to_colatitude<typename E::value_type> >::expression_type
            expression_type;
        return expression_type( e() );
    }

    //**********************************************************
    // algebraic functions

    /**
     * Magnitude of a complex matrix.
     */
    template<class E> inline
        typename matrix_unary1_traits<E,
    	scalar_abs<typename E::value_type> >::result_type
    abs(const matrix_expression<E> &e) {
        typedef typename matrix_unary1_traits<E,
	    scalar_abs<typename E::value_type> >::expression_type
            expression_type;
        return expression_type( e() );
    }

    /**
     * Magnitude squared of a complex matrix.
     */
    template<class E> inline
        typename matrix_unary1_traits<E,
    	scalar_abs2<typename E::value_type> >::result_type
    abs2(const matrix_expression<E> &e) {
        typedef typename matrix_unary1_traits<E,
	    scalar_abs2<typename E::value_type> >::expression_type
            expression_type;
        return expression_type( e() );
    }

    /**
     * Phase of a complex matrix.
     */
    template<class E> inline
        typename matrix_unary1_traits<E,
    	scalar_arg<typename E::value_type> >::result_type
    arg(const matrix_expression<E> &e) {
        typedef typename matrix_unary1_traits<E,
	    scalar_arg<typename E::value_type> >::expression_type
            expression_type;
        return expression_type( e() );
    }

    /**
     * Square root of a matrix.
     */
    template<class E> inline
        typename matrix_unary1_traits<E,
    	scalar_sqrt<typename E::value_type> >::result_type
    sqrt(const matrix_expression<E> &e) {
        typedef typename matrix_unary1_traits<E,
	    scalar_sqrt<typename E::value_type> >::expression_type
            expression_type;
        return expression_type( e() );
    }
    
    /**
     * Copy sign of matrix elements. Each element in the result has a value 
     * whose absolute value matches that of the first argument, but whose sign
     * matches that of the second argument.  Not defined for complex matrices.
     */
    template<class E1, class E2> inline
        typename matrix_binary_traits<E1, E2, scalar_copysign< 
    	typename E1::value_type, typename E2::value_type> >::result_type
    copysign( const matrix_expression<E1> &u, const matrix_expression<E2> &v ) {
        typedef typename matrix_binary_traits<E1, E2, 
	    scalar_copysign< typename E1::value_type, typename E2::value_type> 
	    >::expression_type expression_type;
        return expression_type( u(), v() );
    }


    //**********************************************************
    // standard trigonometric functions

    /**
     * Cosine of a matrix.
     */
    template<class E> inline
        typename matrix_unary1_traits<E,
    	scalar_cos<typename E::value_type> >::result_type
    cos(const matrix_expression<E> &e) {
        typedef typename matrix_unary1_traits<E,
	    scalar_cos<typename E::value_type> >::expression_type
            expression_type;
        return expression_type( e() );
    }

    /**
     * Hyperbolic cosine of a matrix.
     */
    template<class E> inline
        typename matrix_unary1_traits<E,
    	scalar_cosh<typename E::value_type> >::result_type
    cosh(const matrix_expression<E> &e) {
        typedef typename matrix_unary1_traits<E,
	    scalar_cosh<typename E::value_type> >::expression_type
            expression_type;
        return expression_type( e() );
    }

    /**
     * Sine of a matrix.
     */
    template<class E> inline
        typename matrix_unary1_traits<E,
    	scalar_sin<typename E::value_type> >::result_type
    sin(const matrix_expression<E> &e) {
        typedef typename matrix_unary1_traits<E,
	    scalar_sin<typename E::value_type> >::expression_type
            expression_type;
        return expression_type( e() );
    }

    /**
     * Hyperbolic sine of a matrix.
     */
    template<class E> inline
        typename matrix_unary1_traits<E,
    	scalar_sinh<typename E::value_type> >::result_type
    sinh(const matrix_expression<E> &e) {
        typedef typename matrix_unary1_traits<E,
	    scalar_sinh<typename E::value_type> >::expression_type
            expression_type;
        return expression_type( e() );
    }

    /**
     * Tangent of a matrix.
     */
    template<class E> inline
        typename matrix_unary1_traits<E,
    	scalar_tan<typename E::value_type> >::result_type
    tan(const matrix_expression<E> &e) {
        typedef typename matrix_unary1_traits<E,
	    scalar_tan<typename E::value_type> >::expression_type
            expression_type;
        return expression_type( e() );
    }

    /**
     * Hyperbolic tangent of a matrix.
     */
    template<class E> inline
        typename matrix_unary1_traits<E,
    	scalar_tanh<typename E::value_type> >::result_type
    tanh(const matrix_expression<E> &e) {
        typedef typename matrix_unary1_traits<E,
	    scalar_tanh<typename E::value_type> >::expression_type
            expression_type;
        return expression_type( e() );
    }

    //**********************************************************
    // inverse trigonometric functions

    /**
     * Inverse cosine of a matrix.
     */
    template<class E> inline
        typename matrix_unary1_traits<E,
    	scalar_acos<typename E::value_type> >::result_type
    acos(const matrix_expression<E> &e) {
        typedef typename matrix_unary1_traits<E,
	    scalar_acos<typename E::value_type> >::expression_type
            expression_type;
        return expression_type( e() );
    }

    /**
     * Inverse hyperbolic cosine of a matrix.
     */
    template<class E> inline
        typename matrix_unary1_traits<E,
    	scalar_acosh<typename E::value_type> >::result_type
    acosh(const matrix_expression<E> &e) {
        typedef typename matrix_unary1_traits<E,
	    scalar_acosh<typename E::value_type> >::expression_type
            expression_type;
        return expression_type( e() );
    }

    /**
     * Inverse sine of a matrix.
     */
    template<class E> inline
        typename matrix_unary1_traits<E,
    	scalar_asin<typename E::value_type> >::result_type
    asin(const matrix_expression<E> &e) {
        typedef typename matrix_unary1_traits<E,
	    scalar_asin<typename E::value_type> >::expression_type
            expression_type;
        return expression_type( e() );
    }

    /**
     * Inverse hyperbolic sine of a matrix.
     */
    template<class E> inline
        typename matrix_unary1_traits<E,
    	scalar_asinh<typename E::value_type> >::result_type
    asinh(const matrix_expression<E> &e) {
        typedef typename matrix_unary1_traits<E,
	    scalar_asinh<typename E::value_type> >::expression_type
            expression_type;
        return expression_type( e() );
    }

    /**
     * Inverse tangent of a matrix.
     */
    template<class E> inline
        typename matrix_unary1_traits<E, 
    	scalar_atan<typename E::value_type> >::result_type
    atan(const matrix_expression<E> &e) {
        typedef typename matrix_unary1_traits<E,
	    scalar_atan<typename E::value_type> >::expression_type
            expression_type;
        return expression_type( e() );
    }

    /**
     * Inverse tangent of matrix y / matrix x.
     */
    template<class E1, class E2> inline
        typename matrix_binary_traits<E1, E2, scalar_atan2< 
    	typename E1::value_type, typename E2::value_type> >::result_type
    atan2( const matrix_expression<E1> &y, const matrix_expression<E2> &x ) {
        typedef typename matrix_binary_traits<E1, E2, 
	    scalar_atan2< typename E1::value_type, typename E2::value_type> 
	    >::expression_type expression_type;
        return expression_type( y(), x() );
    }

    /**
     * Inverse hyperbolic tangent of a matrix.
     */
    template<class E> inline
        typename matrix_unary1_traits<E,
    	scalar_atanh<typename E::value_type> >::result_type
    atanh(const matrix_expression<E> &e) {
        typedef typename matrix_unary1_traits<E,
	    scalar_atanh<typename E::value_type> >::expression_type
            expression_type;
        return expression_type( e() );
    }

    //**********************************************************
    // standard exponential functions

    /**
     * Exponential of a matrix.
     */
    template<class E> inline
        typename matrix_unary1_traits<E,
    	scalar_exp<typename E::value_type> >::result_type
    exp(const matrix_expression<E> &e) {
        typedef typename matrix_unary1_traits<E,
	    scalar_exp<typename E::value_type> >::expression_type
            expression_type;
        return expression_type( e() );
    }

    /**
     * Natural logarithm of a matrix.
     */
    template<class E> inline
        typename matrix_unary1_traits<E,
    	scalar_log<typename E::value_type> >::result_type
    log(const matrix_expression<E> &e) {
        typedef typename matrix_unary1_traits<E,
	    scalar_log<typename E::value_type> >::expression_type
            expression_type;
        return expression_type( e() );
    }

    /**
     * Base 10 logarithm of a matrix.
     */
    template<class E> inline
        typename matrix_unary1_traits<E,
    	scalar_log10<typename E::value_type> >::result_type
    log10(const matrix_expression<E> &e) {
        typedef typename matrix_unary1_traits<E,
	    scalar_log10<typename E::value_type> >::expression_type
            expression_type;
        return expression_type( e() );
    }

    /**
     * Raise matrix (v) to a scalar (t) power such that (v^t)[i] = v[i] ^ t.
     * Assumes that the scalar type is the same as that stored in the matrix.
     */
    template<class E> inline
        typename matrix_binary_scalar2_traits<E, typename E::value_type,
    	scalar_pow<typename E::value_type, typename E::value_type> 
    	>::result_type
    pow( const matrix_expression<E> &e1, typename E::value_type e2 ) {
        typedef typename matrix_binary_scalar2_traits<E,typename E::value_type,
	    scalar_pow<typename E::value_type, typename E::value_type> 
	    >::expression_type expression_type;
        return expression_type (e1(), e2);
    }

    /**
     * Raise scalar (t) to a matrix (v) power such that (t^v)[i] = t ^ v[i].
     * Assumes that the scalar type is the same as that stored in the matrix.
     */
    template<class E> inline
        typename matrix_binary_scalar1_traits<typename E::value_type, E,
    	scalar_pow<typename E::value_type, typename E::value_type> 
    	>::result_type
    pow( typename E::value_type e1, const matrix_expression<E> &e2 ) {
        typedef typename matrix_binary_scalar1_traits<typename E::value_type,E,
	    scalar_pow<typename E::value_type, typename E::value_type> 
		>::expression_type expression_type;
        return expression_type (e1, e2());
    }

    /**
     * Raise a matrix (u) to a matrix power (v), element by element, 
     * such that (u^v)[i] = u[i] ^ v[i].
     */
    template<class E1, class E2> inline
        typename matrix_binary_traits<E1, E2, scalar_pow< 
    	typename E1::value_type, typename E2::value_type> >::result_type
    pow( const matrix_expression<E1> &u, const matrix_expression<E2> &v ) {
        typedef typename matrix_binary_traits<E1, E2, 
	    scalar_pow< typename E1::value_type, typename E2::value_type> 
	    >::expression_type expression_type;
        return expression_type( u(), v() );
    }

    //**********************************************************
    // signal processing functions

    /**
     * Converts a real phase angle into a real valued signal.
     * The sin(t) form is used to make the phase zero at t=0.
     */
    template<class E> inline
        typename matrix_unary1_traits<E,
        scalar_signal<typename E::value_type> >::result_type
    signal(const matrix_expression<E> &e) {
        typedef typename matrix_unary1_traits<E,
            scalar_signal<typename E::value_type> >::expression_type
            expression_type;
        return expression_type( e() );
    }

    /**
     * Converts a real phase angle into a complex analytic signal.
     * The real part of this matrix is equivalent to signal(t).
     */
    template<class E> inline
        typename matrix_unary1_traits<E,
        scalar_asignal<typename E::value_type> >::result_type
    asignal(const matrix_expression<E> &e) {
        typedef typename matrix_unary1_traits<E,
            scalar_asignal<typename E::value_type> >::expression_type
            expression_type;
        return expression_type( e() );
    }

} // end of ublas namespace
} // end of usml namespace
/// }

#endif
