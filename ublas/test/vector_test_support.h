/**
 * @file vector_test_support.h
 * Helper functions for the vector tests.
 */
#include <usml/ublas/math_traits.h>
#include <boost/foreach.hpp>

using namespace usml::ublas ;
using namespace boost ;

#define USML_VECTOR_REAL_TESTER( R, V, F ) \
    vector_test::vector_real_helper( R, V, F )

#define USML_VECTOR_COMPLEX_TESTER( R, V, F ) \
    vector_test::vector_complex_helper( R, V, F )

#define USML_VECTOR_CR_TESTER( R, V, F ) \
    vector_test::vector_cr_helper( R, V, F )

#define USML_VECTOR_POW_TESTER( R, T1, T2 ) \
    vector_test::vector_power_helper( R, T1, T2 )

#define USML_VECTOR_POW_SCALAR1_TESTER( R, S, V ) \
    vector_test::vector_power_scalar1_helper( R, S, V )

#define USML_VECTOR_POW_SCALAR2_TESTER( R, V, S ) \
    vector_test::vector_power_scalar2_helper( R, V, S )

namespace vector_test {
/**
 * Various helper functions to allow for ease of comparisons.
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
T scalar2_multiply_helper( const T t ) {
    return t * 2.1 ;
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
 * Checker used for vectors that are real valued
 */
template<class Functor>
void vector_real_helper(
    const vector<double>& result,
    const vector<double>& v, Functor f)
{
    for(size_t i=0; i<result.size(); ++i) {
        std::complex<double> tmp = f(v(i)) ;
        check_helper( result(i), tmp.real() );
    }
}

/**
 * Checker for vectors that are complex valued
 */
template<class Functor>
void vector_complex_helper(
    const vector<complex<double> >& result,
    const vector<complex<double> >& v, Functor f)
{
    for(size_t i=0; i<result.size(); ++i) {
        std::complex<double> value = f(v(i)) ;
        check_helper( result(i), value ) ;
    }
}

/**
 * Checker for vectors that are complex valued
 */
template<class Functor>
void vector_cr_helper(
    const vector<complex<double> >& result,
    const vector<double>& v, Functor f)
{
    for(size_t i=0; i<result.size(); ++i) {
        std::complex<double> value = f(v(i)) ;
        check_helper( result(i), value ) ;
    }
}


/**
 * Tests the power functionality of vectors
 */
template<class R, class V1, class V2>
void vector_power_helper(
    const R& result, const V1& v1, const V2& v2 )
{
    typedef typename R::value_type  result_type ;
    for(size_t i=0; i<result.size(); ++i) {
        result_type value = std::pow(v1(i),v2(i)) ;
        check_helper( result(i), value ) ;
    }
}

template<class R, class S, class V>
void vector_power_scalar1_helper(
    const R& result, const S& s, const V& v )
{
    typedef typename R::value_type  result_type ;
    for(size_t i=0; i<result.size(); ++i) {
        result_type value = std::pow(s,v(i)) ;
        check_helper( result(i), value ) ;
    }
}

template<class R, class V, class S>
void vector_power_scalar2_helper(
    const R& result, const V& v, const S& s )
{
    typedef typename R::value_type  result_type ;
    for(size_t i=0; i<result.size(); ++i) {
        result_type value = std::pow(v(i),s) ;
        check_helper( result(i), value ) ;
    }
}

}   //end of namespace vector_test
