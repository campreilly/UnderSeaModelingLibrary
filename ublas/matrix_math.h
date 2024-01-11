/**
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
#pragma once

#include <usml/ublas/scalar_math.h>

#include <boost/core/enable_if.hpp>
#include <boost/numeric/ublas/detail/config.hpp>
#include <boost/numeric/ublas/expression_types.hpp>
#include <boost/numeric/ublas/functional.hpp>
#include <boost/numeric/ublas/matrix_expression.hpp>
#include <boost/type_traits/is_convertible.hpp>

namespace usml {
namespace ublas {

/// @ingroup vecmath
/// @{

//**********************************************************
// scalar addition

/**
 * Add matrix (v) and scalar (t) such that (v+t)[i] = v[i] + t.
 * Requires that the scalar type be convertible to the one stored in the matrix.
 */
template <class E1, class T2>
BOOST_UBLAS_INLINE typename boost::enable_if<
    boost::is_convertible<T2, typename E1::value_type>,
    typename matrix_binary_scalar2_traits<
        E1, const T2,
        scalar_plus<typename E1::value_type, T2> >::result_type>::type
operator+(const matrix_expression<E1> &e1, const T2 &e2) {
    typedef typename matrix_binary_scalar2_traits<
        E1, const T2,
        scalar_plus<typename E1::value_type, T2> >::expression_type
        expression_type;
    return expression_type(e1(), e2);
}

/**
 * Add scalar (t) and matrix (v) such that (t+v)[i] = t + v[i].
 * Requires that the scalar type be convertible to the one stored in the matrix.
 */
template <class T1, class E2>
BOOST_UBLAS_INLINE typename boost::enable_if<
    boost::is_convertible<T1, typename E2::value_type>,
    typename matrix_binary_scalar1_traits<
        const T1, E2,
        scalar_plus<T1, typename E2::value_type> >::result_type>::type
operator+(const T1 &e1, const matrix_expression<E2> &e2) {
    typedef typename matrix_binary_scalar1_traits<
        const T1, E2,
        scalar_plus<T1, typename E2::value_type> >::expression_type
        expression_type;
    return expression_type(e1, e2());
}

/**
 * Subtract matrix (v) and scalar (t) such that (v-t)[i] = v[i] - t.
 * Requires that the scalar type be convertible to the one stored in the matrix.
 */
template <class E1, class T2>
BOOST_UBLAS_INLINE typename boost::enable_if<
    boost::is_convertible<T2, typename E1::value_type>,
    typename matrix_binary_scalar2_traits<
        E1, const T2,
        scalar_minus<typename E1::value_type, T2> >::result_type>::type
operator-(const matrix_expression<E1> &e1, const T2 &e2) {
    typedef typename matrix_binary_scalar2_traits<
        E1, const T2,
        scalar_minus<typename E1::value_type, T2> >::expression_type
        expression_type;
    return expression_type(e1(), e2);
}

/**
 * Subtract scalar (t) and matrix (v) such that (t-v)[i] = t - v[i].
 * Requires that the scalar type be convertible to the one stored in the matrix.
 */
template <class T1, class E2>
BOOST_UBLAS_INLINE typename boost::enable_if<
    boost::is_convertible<T1, typename E2::value_type>,
    typename matrix_binary_scalar1_traits<
        const T1, E2,
        scalar_minus<T1, typename E2::value_type> >::result_type>::type
operator-(const T1 &e1, const matrix_expression<E2> &e2) {
    typedef typename matrix_binary_scalar1_traits<
        const T1, E2,
        scalar_minus<T1, typename E2::value_type> >::expression_type
        expression_type;
    return expression_type(e1, e2());
}

//**********************************************************
// scalar division

/**
 * Divide scalar (t) and matrix (v) such that (t/v)[i] = t / v[i].
 * Requires that the scalar type be convertible to the one stored in the matrix.
 */
template <class T1, class E2>
BOOST_UBLAS_INLINE typename boost::enable_if<
    boost::is_convertible<T1, typename E2::value_type>,
    typename matrix_binary_scalar1_traits<
        const T1, E2,
        scalar_divides<T1, typename E2::value_type> >::result_type>::type
operator/(const T1 &e1, const matrix_expression<E2> &e2) {
    typedef typename matrix_binary_scalar1_traits<
        const T1, E2,
        scalar_divides<T1, typename E2::value_type> >::expression_type
        expression_type;
    return expression_type(e1, e2());
}

//**********************************************************
// element multiplication

/**
 * Define operator*() as an alias for element_prod().
 */
template <class E1, class E2>
BOOST_UBLAS_INLINE typename boost::enable_if<
    boost::is_convertible<E1, E2>,
    typename matrix_binary_traits<
        E1, E2,
        scalar_multiplies<typename E1::value_type,
                          typename E2::value_type> >::result_type>::type
operator*(const matrix_expression<E1> &u, const matrix_expression<E2> &v) {
    return element_prod(u(), v());
}

/**
 * Define operator/() as an alias for element_div().
 */
template <class E1, class E2>
BOOST_UBLAS_INLINE typename boost::enable_if<
    boost::is_convertible<E1, E2>,
    typename matrix_binary_traits<
        E1, E2,
        scalar_divides<typename E1::value_type,
                       typename E2::value_type> >::result_type>::type
operator/(const matrix_expression<E1> &u, const matrix_expression<E2> &v) {
    return element_div(u(), v());
}

//**********************************************************
// limiting functions

/**
 * Maximum between a matrix and a scalar.
 * Has the effect of clipping low values out of the matrix.
 * Requires that the scalar type be convertible to the one stored in the matrix.
 * Not defined for complex matrixs.
 */
template <class E1, class T2>
BOOST_UBLAS_INLINE typename boost::enable_if<
    boost::is_convertible<T2, typename E1::value_type>,
    typename matrix_binary_scalar2_traits<
        E1, const T2,
        scalar_max<typename E1::value_type, T2> >::result_type>::type
max(const matrix_expression<E1> &e1, const T2 &e2) {
    typedef typename matrix_binary_scalar2_traits<
        E1, const T2, scalar_max<typename E1::value_type, T2> >::expression_type
        expression_type;
    return expression_type(e1(), e2);
}

/**
 * Minimum between a matrix and a scalar.
 * Has the effect of clipping high values out of the matrix.
 * Requires that the scalar type be convertible to the one stored in the matrix.
 * Not defined for complex matrixs.
 */
template <class E1, class T2>
BOOST_UBLAS_INLINE typename boost::enable_if<
    boost::is_convertible<T2, typename E1::value_type>,
    typename matrix_binary_scalar2_traits<
        E1, const T2,
        scalar_min<typename E1::value_type, T2> >::result_type>::type
min(const matrix_expression<E1> &e1, const T2 &e2) {
    typedef typename matrix_binary_scalar2_traits<
        E1, const T2, scalar_min<typename E1::value_type, T2> >::expression_type
        expression_type;
    return expression_type(e1(), e2);
}

/**
 * Rounding elements of a matrix down to the nearest integer.
 * Not defined for complex matrixs.
 */
template <class E>
BOOST_UBLAS_INLINE typename matrix_unary1_traits<
    E, scalar_floor<typename E::value_type> >::result_type
floor(const matrix_expression<E> &e) {
    typedef typename matrix_unary1_traits<
        E, scalar_floor<typename E::value_type> >::expression_type
        expression_type;
    return expression_type(e());
}

/**
 * Rounding elements of a matrix up to the nearest integer.
 * Not defined for complex matrixs.
 */
template <class E>
BOOST_UBLAS_INLINE typename matrix_unary1_traits<
    E, scalar_ceil<typename E::value_type> >::result_type
ceil(const matrix_expression<E> &e) {
    typedef
        typename matrix_unary1_traits<E, scalar_ceil<typename E::value_type> >::
            expression_type expression_type;
    return expression_type(e());
}

//**********************************************************
// conversion functions

/**
 * Convert angle value in radians to degrees.
 */
template <class E>
BOOST_UBLAS_INLINE typename matrix_unary1_traits<
    E, scalar_to_degrees<typename E::value_type> >::result_type
to_degrees(const matrix_expression<E> &e) {
    typedef typename matrix_unary1_traits<
        E, scalar_to_degrees<typename E::value_type> >::expression_type
        expression_type;
    return expression_type(e());
}

/**
 * Convert angle value in degrees to radians.
 */
template <class E>
BOOST_UBLAS_INLINE typename matrix_unary1_traits<
    E, scalar_to_radians<typename E::value_type> >::result_type
to_radians(const matrix_expression<E> &e) {
    typedef typename matrix_unary1_traits<
        E, scalar_to_radians<typename E::value_type> >::expression_type
        expression_type;
    return expression_type(e());
}

/**
 * Convert colatitude (radians from north) value to
 * latitude (degrees from equator).
 */
template <class E>
BOOST_UBLAS_INLINE typename matrix_unary1_traits<
    E, scalar_to_latitude<typename E::value_type> >::result_type
to_latitude(const matrix_expression<E> &e) {
    typedef typename matrix_unary1_traits<
        E, scalar_to_latitude<typename E::value_type> >::expression_type
        expression_type;
    return expression_type(e());
}

/**
 * Convert latitude (degrees from equator) value to
 * colatitude (radians from north).
 */
template <class E>
BOOST_UBLAS_INLINE typename matrix_unary1_traits<
    E, scalar_to_colatitude<typename E::value_type> >::result_type
to_colatitude(const matrix_expression<E> &e) {
    typedef typename matrix_unary1_traits<
        E, scalar_to_colatitude<typename E::value_type> >::expression_type
        expression_type;
    return expression_type(e());
}

//**********************************************************
// algebraic functions

/**
 * Magnitude of a complex matrix.
 */
template <class E>
BOOST_UBLAS_INLINE typename matrix_unary1_traits<
    E, scalar_abs<typename E::value_type> >::result_type
abs(const matrix_expression<E> &e) {
    typedef
        typename matrix_unary1_traits<E, scalar_abs<typename E::value_type> >::
            expression_type expression_type;
    return expression_type(e());
}

/**
 * Magnitude squared of a complex matrix.
 */
template <class E>
BOOST_UBLAS_INLINE typename matrix_unary1_traits<
    E, scalar_abs2<typename E::value_type> >::result_type
abs2(const matrix_expression<E> &e) {
    typedef
        typename matrix_unary1_traits<E, scalar_abs2<typename E::value_type> >::
            expression_type expression_type;
    return expression_type(e());
}

/**
 * Phase of a complex matrix.
 */
template <class E>
BOOST_UBLAS_INLINE typename matrix_unary1_traits<
    E, scalar_arg<typename E::value_type> >::result_type
arg(const matrix_expression<E> &e) {
    typedef
        typename matrix_unary1_traits<E, scalar_arg<typename E::value_type> >::
            expression_type expression_type;
    return expression_type(e());
}

/**
 * Square root of a matrix.
 */
template <class E>
BOOST_UBLAS_INLINE typename matrix_unary1_traits<
    E, scalar_sqrt<typename E::value_type> >::result_type
sqrt(const matrix_expression<E> &e) {
    typedef
        typename matrix_unary1_traits<E, scalar_sqrt<typename E::value_type> >::
            expression_type expression_type;
    return expression_type(e());
}

/**
 * Copy sign of matrix elements. Each element in the result has a value
 * whose absolute value matches that of the first argument, but whose sign
 * matches that of the second argument.  Not defined for complex matrices.
 */
template <class E1, class E2>
BOOST_UBLAS_INLINE typename matrix_binary_traits<
    E1, E2,
    scalar_copysign<typename E1::value_type,
                    typename E2::value_type> >::result_type
copysign(const matrix_expression<E1> &u, const matrix_expression<E2> &v) {
    typedef typename matrix_binary_traits<
        E1, E2,
        scalar_copysign<typename E1::value_type, typename E2::value_type> >::
        expression_type expression_type;
    return expression_type(u(), v());
}

//**********************************************************
// standard trigonometric functions

/**
 * Cosine of a matrix.
 */
template <class E>
BOOST_UBLAS_INLINE typename matrix_unary1_traits<
    E, scalar_cos<typename E::value_type> >::result_type
cos(const matrix_expression<E> &e) {
    typedef
        typename matrix_unary1_traits<E, scalar_cos<typename E::value_type> >::
            expression_type expression_type;
    return expression_type(e());
}

/**
 * Hyperbolic cosine of a matrix.
 */
template <class E>
BOOST_UBLAS_INLINE typename matrix_unary1_traits<
    E, scalar_cosh<typename E::value_type> >::result_type
cosh(const matrix_expression<E> &e) {
    typedef
        typename matrix_unary1_traits<E, scalar_cosh<typename E::value_type> >::
            expression_type expression_type;
    return expression_type(e());
}

/**
 * Sine of a matrix.
 */
template <class E>
BOOST_UBLAS_INLINE typename matrix_unary1_traits<
    E, scalar_sin<typename E::value_type> >::result_type
sin(const matrix_expression<E> &e) {
    typedef
        typename matrix_unary1_traits<E, scalar_sin<typename E::value_type> >::
            expression_type expression_type;
    return expression_type(e());
}

/**
 * Hyperbolic sine of a matrix.
 */
template <class E>
BOOST_UBLAS_INLINE typename matrix_unary1_traits<
    E, scalar_sinh<typename E::value_type> >::result_type
sinh(const matrix_expression<E> &e) {
    typedef
        typename matrix_unary1_traits<E, scalar_sinh<typename E::value_type> >::
            expression_type expression_type;
    return expression_type(e());
}

/**
 * Tangent of a matrix.
 */
template <class E>
BOOST_UBLAS_INLINE typename matrix_unary1_traits<
    E, scalar_tan<typename E::value_type> >::result_type
tan(const matrix_expression<E> &e) {
    typedef
        typename matrix_unary1_traits<E, scalar_tan<typename E::value_type> >::
            expression_type expression_type;
    return expression_type(e());
}

/**
 * Hyperbolic tangent of a matrix.
 */
template <class E>
BOOST_UBLAS_INLINE typename matrix_unary1_traits<
    E, scalar_tanh<typename E::value_type> >::result_type
tanh(const matrix_expression<E> &e) {
    typedef
        typename matrix_unary1_traits<E, scalar_tanh<typename E::value_type> >::
            expression_type expression_type;
    return expression_type(e());
}

//**********************************************************
// inverse trigonometric functions

/**
 * Inverse cosine of a matrix.
 */
template <class E>
BOOST_UBLAS_INLINE typename matrix_unary1_traits<
    E, scalar_acos<typename E::value_type> >::result_type
acos(const matrix_expression<E> &e) {
    typedef
        typename matrix_unary1_traits<E, scalar_acos<typename E::value_type> >::
            expression_type expression_type;
    return expression_type(e());
}

/**
 * Inverse hyperbolic cosine of a matrix.
 */
template <class E>
BOOST_UBLAS_INLINE typename matrix_unary1_traits<
    E, scalar_acosh<typename E::value_type> >::result_type
acosh(const matrix_expression<E> &e) {
    typedef typename matrix_unary1_traits<
        E, scalar_acosh<typename E::value_type> >::expression_type
        expression_type;
    return expression_type(e());
}

/**
 * Inverse sine of a matrix.
 */
template <class E>
BOOST_UBLAS_INLINE typename matrix_unary1_traits<
    E, scalar_asin<typename E::value_type> >::result_type
asin(const matrix_expression<E> &e) {
    typedef
        typename matrix_unary1_traits<E, scalar_asin<typename E::value_type> >::
            expression_type expression_type;
    return expression_type(e());
}

/**
 * Inverse hyperbolic sine of a matrix.
 */
template <class E>
BOOST_UBLAS_INLINE typename matrix_unary1_traits<
    E, scalar_asinh<typename E::value_type> >::result_type
asinh(const matrix_expression<E> &e) {
    typedef typename matrix_unary1_traits<
        E, scalar_asinh<typename E::value_type> >::expression_type
        expression_type;
    return expression_type(e());
}

/**
 * Inverse tangent of a matrix.
 */
template <class E>
BOOST_UBLAS_INLINE typename matrix_unary1_traits<
    E, scalar_atan<typename E::value_type> >::result_type
atan(const matrix_expression<E> &e) {
    typedef
        typename matrix_unary1_traits<E, scalar_atan<typename E::value_type> >::
            expression_type expression_type;
    return expression_type(e());
}

/**
 * Inverse tangent of matrix y / matrix x.
 */
template <class E1, class E2>
BOOST_UBLAS_INLINE typename matrix_binary_traits<
    E1, E2, scalar_atan2<typename E1::value_type, typename E2::value_type> >::
    result_type
    atan2(const matrix_expression<E1> &y, const matrix_expression<E2> &x) {
    typedef typename matrix_binary_traits<
        E1, E2,
        scalar_atan2<typename E1::value_type, typename E2::value_type> >::
        expression_type expression_type;
    return expression_type(y(), x());
}

/**
 * Inverse hyperbolic tangent of a matrix.
 */
template <class E>
BOOST_UBLAS_INLINE typename matrix_unary1_traits<
    E, scalar_atanh<typename E::value_type> >::result_type
atanh(const matrix_expression<E> &e) {
    typedef typename matrix_unary1_traits<
        E, scalar_atanh<typename E::value_type> >::expression_type
        expression_type;
    return expression_type(e());
}

//**********************************************************
// standard exponential functions

/**
 * Exponential of a matrix.
 */
template <class E>
BOOST_UBLAS_INLINE typename matrix_unary1_traits<
    E, scalar_exp<typename E::value_type> >::result_type
exp(const matrix_expression<E> &e) {
    typedef
        typename matrix_unary1_traits<E, scalar_exp<typename E::value_type> >::
            expression_type expression_type;
    return expression_type(e());
}

/**
 * Natural logarithm of a matrix.
 */
template <class E>
BOOST_UBLAS_INLINE typename matrix_unary1_traits<
    E, scalar_log<typename E::value_type> >::result_type
log(const matrix_expression<E> &e) {
    typedef
        typename matrix_unary1_traits<E, scalar_log<typename E::value_type> >::
            expression_type expression_type;
    return expression_type(e());
}

/**
 * Base 10 logarithm of a matrix.
 */
template <class E>
BOOST_UBLAS_INLINE typename matrix_unary1_traits<
    E, scalar_log10<typename E::value_type> >::result_type
log10(const matrix_expression<E> &e) {
    typedef typename matrix_unary1_traits<
        E, scalar_log10<typename E::value_type> >::expression_type
        expression_type;
    return expression_type(e());
}

/**
 * Raise matrix (v) to a scalar (t) power such that (v^t)[i] = v[i] ^ t.
 * Requires that the scalar type be convertible to the one stored in the matrix.
 */
template <class E1, class T2>
BOOST_UBLAS_INLINE typename boost::enable_if<
    boost::is_convertible<T2, typename E1::value_type>,
    typename matrix_binary_scalar2_traits<
        E1, const T2,
        scalar_pow<typename E1::value_type, T2> >::result_type>::type
pow(const matrix_expression<E1> &e1, const T2 &e2) {
    typedef typename matrix_binary_scalar2_traits<
        E1, const T2, scalar_pow<typename E1::value_type, T2> >::expression_type
        expression_type;
    return expression_type(e1(), e2);
}

/**
 * Raise scalar (t) to a matrix (v) power such that (t^v)[i] = t ^ v[i].
 * Requires that the scalar type be convertible to the one stored in the matrix.
 */
template <class T1, class E2>
BOOST_UBLAS_INLINE typename boost::enable_if<
    boost::is_convertible<T1, typename E2::value_type>,
    typename matrix_binary_scalar1_traits<
        const T1, E2,
        scalar_pow<T1, typename E2::value_type> >::result_type>::type
pow(const T1 &e1, const matrix_expression<E2> &e2) {
    typedef typename matrix_binary_scalar1_traits<
        const T1, E2, scalar_pow<T1, typename E2::value_type> >::expression_type
        expression_type;
    return expression_type(e1, e2());
}

/**
 * Raise a matrix (u) to a matrix power (v), element by element,
 * such that (u^v)[i] = u[i] ^ v[i].
 */
template <class E1, class E2>
BOOST_UBLAS_INLINE typename matrix_binary_traits<
    E1, E2,
    scalar_pow<typename E1::value_type, typename E2::value_type> >::result_type
pow(const matrix_expression<E1> &u, const matrix_expression<E2> &v) {
    typedef typename matrix_binary_traits<
        E1, E2, scalar_pow<typename E1::value_type, typename E2::value_type> >::
        expression_type expression_type;
    return expression_type(u(), v());
}

//**********************************************************
// signal processing functions

/**
 * Converts a real phase angle into a real valued signal.
 * The sin(t) form is used to make the phase zero at t=0.
 */
template <class E>
BOOST_UBLAS_INLINE typename matrix_unary1_traits<
    E, scalar_signal<typename E::value_type> >::result_type
signal(const matrix_expression<E> &e) {
    typedef typename matrix_unary1_traits<
        E, scalar_signal<typename E::value_type> >::expression_type
        expression_type;
    return expression_type(e());
}

/**
 * Converts a real phase angle into a complex analytic signal.
 * The real part of this matrix is equivalent to signal(t).
 */
template <class E>
BOOST_UBLAS_INLINE typename matrix_unary1_traits<
    E, scalar_asignal<typename E::value_type> >::result_type
asignal(const matrix_expression<E> &e) {
    typedef typename matrix_unary1_traits<
        E, scalar_asignal<typename E::value_type> >::expression_type
        expression_type;
    return expression_type(e());
}

/// @}
}  // namespace ublas
}  // namespace usml
