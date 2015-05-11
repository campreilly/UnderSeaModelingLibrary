/**
 * @file matrix_test_support.h
 * Helper functions for the matrix tests
 */
#include <usml/ublas/math_traits.h>
#include <boost/foreach.hpp>

using namespace usml::ublas ;
using namespace boost ;

#define USML_MATRIX_REAL_TESTER( R, M, F ) \
    matrix_test::matrix_real_helper( R, M, F )

#define USML_MATRIX_COMPLEX_TESTER( R, M, F ) \
    matrix_test::matrix_complex_helper(R,M,F)

#define USML_MATRIX_CR_TESTER( R, M, F ) \
    matrix_test::matrix_cr_helper(R,M,F)

#define USML_MATRIX_POW_TESTER( R, T1, T2 ) \
    matrix_test::matrix_power_helper(R,T1,T2)

#define USML_MATRIX_POW_SCALAR1_TESTER( R, S, M ) \
    matrix_test::matrix_power_scalar1_helper(R,S,M)

#define USML_MATRIX_POW_SCALAR2_TESTER( R, M, S ) \
    matrix_test::matrix_power_scalar2_helper(R,M,S)

namespace matrix_test {
/**
 * Various helper functions to allow for ease of comparisions
 */
template<class T>
T abs2_helper( const T t ) {
    return t*t ;
}

template<class T>
T abs_helper( const T t ) {
    return std::sqrt( t.real()*t.real() + t.imag()*t.imag() ) ;
}

template<>
complex<double> abs2_helper( const complex<double> t ) {
    return abs_helper(t)*abs_helper(t) ;
}

template<class T>
T floor_helper( const T t ) {
    return math_traits<T>::floor(t+2.1) ;
}

template<class T>
T ceil_helper( const T t ) {
    return math_traits<T>::ceil(t+2.1) ;
}

template<class T>
T max_helper( const T t ) {
    return math_traits<T>::max(t,3.0) ;
}

template<class T>
T min_helper( const T t ) {
    return math_traits<T>::min(t,3.0) ;
}

template<class T>
T divide_helper( const T t1 ) {
    return t1 / t1 ;
}

template<class T>
T multiply_helper( const T t1 ) {
    return t1 * t1 ;
}

template<class T>
T add_helper( const T t1 ) {
    return t1 + t1 ;
}

template<class T>
T scalar1_divide_helper( const T t ) {
    return 2.1 / t ;
}

template<class T>
T scalar1_minus_helper( const T t ) {
    return 2.1 - t ;
}

template<class T>
T scalar2_add_helper( const T t ) {
    return t + 2.1 ;
}

template<class T>
T identity( const T t ) {
    return t ;
}

template<class T>
T signal_helper( const T t ) {
    return math_traits<T>::sin(t) ;
}

template<class T>
complex<T> asignal_helper( const T t ) {
    return complex<T>( math_traits<T>::sin(t), math_traits<T>::cos(t) ) ;
}

template<class T>
T arg_helper( const complex<T> t ) {
    return std::atan2( t.imag(), t.real() );
}

template<class T>
T zero_helper(const T t) {
	return 0.0;
}

template<class T>
void check_helper( const T t1, const T t2 ) {
    BOOST_CHECK_CLOSE( t1, t2, 1e-10 );
}

template<class T>
void check_helper( const complex<T> t1, const complex<T> t2 ) {
    BOOST_CHECK_CLOSE( t1.real(), t2.real(), 1e-10 );
    BOOST_CHECK_CLOSE( t1.imag(), t2.imag(), 1e-10 );
}

/**
 * Checker used for matrices that are real valued
 */
template<class Functor>
void matrix_real_helper(
    const matrix<double>& R,
    const matrix<double>& M, Functor f)
{
    for(size_t i=0; i<R.size1(); ++i) {
        for(size_t j=0; j<R.size2(); ++j) {
            std::complex<double> tmp = f(M(i,j)) ;
            check_helper( R(i,j), tmp.real() );
        }
    }
}

/**
 * Checker for matrices that are complex valued
 */
template<class Functor>
void matrix_complex_helper(
    const matrix<complex<double> >& result,
    const matrix<complex<double> >& origin, Functor f)
{
    for(size_t i=0; i<result.size1(); ++i) {
        for(size_t j=0; j<result.size2(); ++j) {
            std::complex<double> value = f(origin(i,j)) ;
            check_helper( result(i,j), value ) ;
        }
    }
}

template<class Functor>
void matrix_cr_helper(
    const matrix<complex<double> >& result,
    const matrix<double>& origin, Functor f)
{
    for(size_t i=0; i<result.size1(); ++i) {
        for(size_t j=0; j<result.size2(); ++j) {
            std::complex<double> value = f(origin(i,j)) ;
            check_helper( result(i,j), value ) ;
        }
    }
}

/**
 * Tests the power functionality of matrices
 */
template<class R, class M1, class M2>
void matrix_power_helper(
    const R& result, const M1& m1, const M2& m2 )
{
    typedef typename R::value_type  result_type ;
    for(size_t i=0; i<result.size1(); ++i) {
        for(size_t j=0; j<result.size2(); ++j) {
            result_type value = std::pow(m1(i,j),m2(i,j)) ;
            check_helper( result(i,j), value ) ;
        }
    }
}

template<class R, class S, class M>
void matrix_power_scalar1_helper(
    const R& result, const S& s, const M& m )
{
    typedef typename R::value_type  result_type ;
    for(size_t i=0; i<result.size1(); ++i) {
        for(size_t j=0; j<result.size2(); ++j) {
            result_type value = std::pow(s,m(i,j)) ;
            check_helper( result(i,j), value ) ;
        }
    }
}

template<class R, class M, class S>
void matrix_power_scalar2_helper(
    const R& result, const M& m, const S& s )
{
    typedef typename R::value_type  result_type ;
    for(size_t i=0; i<result.size1(); ++i) {
        for(size_t j=0; j<result.size2(); ++j) {
            result_type value = std::pow(m(i,j),s) ;
            check_helper( result(i,j), value ) ;
        }
    }
}

}   // end of namespace matrix_test
