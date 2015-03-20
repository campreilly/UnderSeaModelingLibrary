/** 
 * @file math_traits.h
 * Traits implementation of new math algorithms for scientific applications.
 */
#pragma once

#include<cmath>
#include<complex>
#include<boost/numeric/ublas/vector.hpp>
#include<boost/numeric/ublas/matrix.hpp>
#include<boost/numeric/ublas/io.hpp>
#include <usml/usml_config.h>

# define TWO_PI     (2.0*M_PI)

namespace usml {
namespace ublas {

using std::cout;
using std::operator<<;
using std::endl;

using std::complex;
using std::size_t;

using std::max;
using std::min;
using std::floor;
using std::ceil;

using std::abs;
using std::arg;
using std::sqrt;

using std::cos;
using std::cosh;
using std::sin;
using std::sinh;
using std::tan;
using std::tanh;

using std::acos;
using std::asin;
using std::atan;
using std::atan2;

using std::exp;
using std::log;
using std::log10;
using std::pow;

using namespace boost::numeric::ublas;

const complex<double> DOUBLE_I = complex<double> (0.0, 1.0);
const complex<float> FLOAT_I = complex<float> (0.0, 1.0);

/**
 * @internal
 * Traits implementation of new math algorithms for scientific
 * applications.  Defines scalar prototypes for all of the
 * transcendental functions supported by the complex class.
 * The vector and matrix functionals use these traits to
 * evaluate individual elements. Based on the design of
 * the uBLAS type_traits<> class.
 *
 * This template is parameterized by the type of each function's
 * output.  Therefore, functions such as abs() that always return
 * a real value will be part of the double specialization of this
 * template.
 *
 * Developers will not usually use these classes directly.
 * They are designed to evaluate the expression templates
 * in the vector_math.h and matrix_math.h headers.
 */
template<class T> struct math_traits
{
    typedef math_traits<T> self_type;
    typedef T value_type;
    typedef const T & const_reference;
    typedef T & reference;
    typedef T real_type;

    //*********************************************************
    // conversion functions

    static inline value_type to_degrees(const_reference t);
    static inline value_type to_radians(const_reference t);
    static inline value_type to_latitude(const_reference t);
    static inline value_type to_colatitude(const_reference t);

    //*********************************************************
    // algebraic functions

    static inline value_type sqrt(const_reference t);

    //*********************************************************
    // trigonometric functions

    static inline value_type cos(const_reference t);
    static inline value_type cosh(const_reference t);
    static inline value_type sin(const_reference t);
    static inline value_type sinh(const_reference t);
    static inline value_type tan(const_reference t);
    static inline value_type tanh(const_reference t);

    //*********************************************************
    // inverse trigonometric functions

    static inline value_type acos(const_reference t);
    static inline value_type acosh(const_reference t);
    static inline value_type asin(const_reference t);
    static inline value_type asinh(const_reference t);
    static inline value_type atan(const_reference t);
    static inline value_type atan2(const_reference y, const_reference x);
    static inline value_type atanh(const_reference t);

    //*********************************************************
    // exponential functions

    static inline value_type exp(const_reference t);
    static inline value_type log(const_reference t);
    static inline value_type log10(const_reference t);
    static inline value_type pow(const_reference t, int e);
    static inline value_type pow(const_reference t, const_reference e);
};

/**
 * @internal
 * Specialization of math_traits<> for functions that return double values.
 * Passes all work to the cmath library using static inline methods.
 */
template<> struct math_traits<double>
{
    typedef math_traits<double> self_type;
    typedef double value_type;
    typedef const double & const_reference;
    typedef double & reference;
    typedef double real_type;

    //*********************************************************
    // limiting functions

    static inline value_type max(const_reference t1, const_reference t2)
    {
        return std::max(t1, t2);
    }
    static inline value_type min(const_reference t1, const_reference t2)
    {
        return std::min(t1, t2);
    }
    static inline value_type floor(const_reference t)
    {
        return std::floor(t);
    }
    static inline value_type ceil(const_reference t)
    {
        return std::ceil(t);
    }

    //*********************************************************
    // conversion functions

    static inline value_type to_degrees(const_reference t)
    {
        return t * (180.0 / M_PI);
    }

    static inline value_type to_radians(const_reference t)
    {
        return t * (M_PI / 180.0);
    }

    static inline value_type to_latitude(const_reference t)
    {
        return 90.0 - to_degrees(t);
    }

    static inline value_type to_colatitude(const_reference t)
    {
        return to_radians(90.0 - t);
    }

    //*********************************************************
    // algebraic functions

    static inline value_type sqrt(const_reference t)
    {
        return std::sqrt(t);
    }
    static inline value_type copysign(const_reference t1, const_reference t2)
    {
		return ( t2 < 0.0 ) ? -t1 : t1 ;
    }

    //*********************************************************
    // trigonometric functions

    static inline value_type cos(const_reference t)
    {
        return std::cos(t);
    }
    static inline value_type cosh(const_reference t)
    {
        return std::cosh(t);
    }
    static inline value_type sin(const_reference t)
    {
        return std::sin(t);
    }
    static inline value_type sinh(const_reference t)
    {
        return std::sinh(t);
    }
    static inline value_type tan(const_reference t)
    {
        return std::tan(t);
    }
    static inline value_type tanh(const_reference t)
    {
        return std::tanh(t);
    }

    //*********************************************************
    // inverse trigonometric functions
    // @xref A. Jeffery, Handbook of Math Formulas
    // and Integrals, pp. 124-127.

    static inline value_type acos(const_reference t)
    {
        return std::acos(t);
    }
    static inline value_type acosh(const_reference t)
    {
        return log(t + sqrt(t * t - 1.0));
    }
    static inline value_type asin(const_reference t)
    {
        return std::asin(t);
    }
    static inline value_type asinh(const_reference t)
    {
        return log(t + sqrt(t * t + 1.0));
    }
    static inline value_type atan(const_reference t)
    {
        return std::atan(t);
    }
    static inline value_type atan2(const_reference y, const_reference x)
    {
        return std::atan2(y, x);
    }
    static inline value_type atanh(const_reference t)
    {
        return 0.5 * log((1.0 + t) / (1.0 - t));
    }

    //*********************************************************
    // exponential functions

    static inline value_type exp(const_reference t)
    {
        return std::exp(t);
    }
    static inline value_type log(const_reference t)
    {
        return std::log(t);
    }
    static inline value_type log10(const_reference t)
    {
        return std::log10(t);
    }
    static inline value_type pow(const_reference t, int e)
    {
        return std::pow(t, e);
    }
    static inline value_type pow(const_reference t, const_reference e)
    {
        return std::pow(t, e);
    }

    //*********************************************************
    // functions specific to double return values.

    static inline real_type abs(const_reference t)
    {
        return std::abs(t);
    }
    static inline real_type arg(const_reference t)
    {
        return 0.0;
    }
    static inline real_type abs2(const_reference t)
    {
        return t * t;
    }

    static inline real_type abs(const complex<value_type> &t)
    {
        return std::abs(t);
    }
    static inline real_type arg(const complex<value_type> &t)
    {
        return std::arg(t);
    }
    static inline real_type abs2(const complex<value_type> &t)
    {
        return t.real() * t.real() + t.imag() * t.imag();
    }
};

/**
 * @internal
 * Specialization of math_traits<> for functions that return float values.
 * Passes all work to the cmath library using static inline methods.
 */
template<> struct math_traits<float>
{
    typedef math_traits<float> self_type;
    typedef float value_type;
    typedef const float & const_reference;
    typedef float & reference;
    typedef float real_type;

    //*********************************************************
    // limiting functions

    static inline value_type max(const_reference t1, const_reference t2)
    {
        return std::max(t1, t2);
    }
    static inline value_type min(const_reference t1, const_reference t2)
    {
        return std::min(t1, t2);
    }
    static inline value_type floor(const_reference t)
    {
        return std::floor(t);
    }
    static inline value_type ceil(const_reference t)
    {
        return std::ceil(t);
    }

    //*********************************************************
    // conversion functions

    static inline value_type to_degrees(const_reference t)
    {
        return t * (180.0 / M_PI);
    }

    static inline value_type to_radians(const_reference t)
    {
        return t * (M_PI / 180.0);
    }

    static inline value_type to_latitude(const_reference t)
    {
        return 90.0 - to_degrees(t);
    }

    static inline value_type to_colatitude(const_reference t)
    {
        return to_radians(90.0 - t);
    }

    //*********************************************************
    // algebraic functions

    static inline value_type sqrt(const_reference t)
    {
        return std::sqrt(t);
    }
    static inline value_type copysign(const_reference t1, const_reference t2)
    {
		return ( t2 < 0.0 ) ? -t1 : t1 ;
    }

    //*********************************************************
    // trigonometric functions

    static inline value_type cos(const_reference t)
    {
        return std::cos(t);
    }
    static inline value_type cosh(const_reference t)
    {
        return std::cosh(t);
    }
    static inline value_type sin(const_reference t)
    {
        return std::sin(t);
    }
    static inline value_type sinh(const_reference t)
    {
        return std::sinh(t);
    }
    static inline value_type tan(const_reference t)
    {
        return std::tan(t);
    }
    static inline value_type tanh(const_reference t)
    {
        return std::tanh(t);
    }

    //*********************************************************
    // inverse trigonometric functions
    // @xref A. Jeffery, Handbook of Math Formulas
    // and Integrals, pp. 124-127.

    static inline value_type acos(const_reference t)
    {
        return std::acos(t);
    }
    static inline value_type acosh(const_reference t)
    {
        return log(t + sqrt(t * t - 1.0f));
    }
    static inline value_type asin(const_reference t)
    {
        return std::asin(t);
    }
    static inline value_type asinh(const_reference t)
    {
        return log(t + sqrt(t * t + 1.0f));
    }
    static inline value_type atan(const_reference t)
    {
        return std::atan(t);
    }
    static inline value_type atan2(const_reference y, const_reference x)
    {
        return std::atan2(y, x);
    }
    static inline value_type atanh(const_reference t)
    {
        return 0.5 * log((1.0f + t) / (1.0f - t));
    }

    //*********************************************************
    // exponential functions

    static inline value_type exp(const_reference t)
    {
        return std::exp(t);
    }
    static inline value_type log(const_reference t)
    {
        return std::log(t);
    }
    static inline value_type log10(const_reference t)
    {
        return std::log10(t);
    }
    static inline value_type pow(const_reference t, int e)
    {
        return std::pow(t, e);
    }
    static inline value_type pow(const_reference t, const_reference e)
    {
        return std::pow(t, e);
    }

    //*********************************************************
    // functions specific to float return values.

    static inline real_type abs(const_reference t)
    {
        return std::abs(t);
    }
    static inline real_type arg(const_reference t)
    {
        return 0.0;
    }
    static inline real_type abs2(const_reference t)
    {
        return t * t;
    }

    static inline real_type abs(const complex<value_type> &t)
    {
        return std::abs(t);
    }
    static inline real_type arg(const complex<value_type> &t)
    {
        return std::arg(t);
    }
    static inline real_type abs2(const complex<value_type> &t)
    {
        return t.real() * t.real() + t.imag() * t.imag();
    }
};

/**
 * @internal
 * Specialization of math_traits<> for functions that return
 * complex<double> values.
 * Passes all work to the std:complex class using static inline methods.
 */
template<> struct math_traits<complex<double> >
{
    typedef math_traits<complex<double> > self_type;
    typedef complex<double> value_type;
    typedef const complex<double> & const_reference;
    typedef complex<double> & reference;
    typedef double real_type;

    //*********************************************************
    // complex functions

    static inline value_type sqrt(const_reference t)
    {
        return std::sqrt(t);
    }
    //*********************************************************
    // trigonometric functions

    static inline value_type cos(const_reference t)
    {
        return std::cos(t);
    }
    static inline value_type cosh(const_reference t)
    {
        return std::cosh(t);
    }
    static inline value_type sin(const_reference t)
    {
        return std::sin(t);
    }
    static inline value_type sinh(const_reference t)
    {
        return std::sinh(t);
    }
    static inline value_type tan(const_reference t)
    {
        return std::tan(t);
    }
    static inline value_type tanh(const_reference t)
    {
        return std::tanh(t);
    }

    //*********************************************************
    // inverse trigonometric functions
    // @xref A. Jeffery, Handbook of Math Formulas
    // and Integrals, pp. 124-127.

    static inline value_type acos(const_reference t)
    {
        return DOUBLE_I * log(t + sqrt(t * t - 1.0));
    }
    static inline value_type acosh(const_reference t)
    {
        return log(t + sqrt(t * t - 1.0));
    }
    static inline value_type asin(const_reference t)
    {
        return -DOUBLE_I * log(DOUBLE_I * t + sqrt(1.0 - t * t));
    }
    static inline value_type asinh(const_reference t)
    {
        return log(t + sqrt(t * t + 1.0));
    }
    static inline value_type atan(const_reference t)
    {
        return log((1.0 + DOUBLE_I * t) / (1.0 - DOUBLE_I * t)) / (2.0
                * DOUBLE_I);
    }
    static inline value_type atan2(const_reference y, const_reference x)
    {
        return atan(y / x);
    }
    static inline value_type atanh(const_reference t)
    {
        return 0.5 * log((1.0 + t) / (1.0 - t));
    }

    //*********************************************************
    // exponential functions

    static inline value_type exp(const_reference t)
    {
        return std::exp(t);
    }
    static inline value_type log(const_reference t)
    {
        return std::log(t);
    }
    static inline value_type log10(const_reference t)
    {
        return std::log10(t);
    }
    static inline value_type pow(const_reference t, int e)
    {
        return std::pow(t, e);
    }
    static inline value_type pow(const_reference t, const_reference e)
    {
        return std::pow(t, e);
    }

    //*********************************************************
    // functions specific to complex<double> return values.

    static inline value_type pow(const_reference t, real_type e)
    {
        return std::pow(t, e);
    }
    static inline value_type pow(real_type t, const_reference e)
    {
        return std::pow(t, e);
    }

};

/**
 * @internal
 * Specialization of math_traits<> for functions that return
 * complex<float> values.
 * Passes all work to the std:complex class using static inline methods.
 */
template<> struct math_traits<complex<float> >
{
    typedef math_traits<complex<float> > self_type;
    typedef complex<float> value_type;
    typedef const complex<float> & const_reference;
    typedef complex<float> & reference;
    typedef float real_type;

    //*********************************************************
    // complex functions

    static inline value_type sqrt(const_reference t)
    {
        return std::sqrt(t);
    }
    //*********************************************************
    // trigonometric functions

    static inline value_type cos(const_reference t)
    {
        return std::cos(t);
    }
    static inline value_type cosh(const_reference t)
    {
        return std::cosh(t);
    }
    static inline value_type sin(const_reference t)
    {
        return std::sin(t);
    }
    static inline value_type sinh(const_reference t)
    {
        return std::sinh(t);
    }
    static inline value_type tan(const_reference t)
    {
        return std::tan(t);
    }
    static inline value_type tanh(const_reference t)
    {
        return std::tanh(t);
    }

    //*********************************************************
    // inverse trigonometric functions
    // @xref A. Jeffery, Handbook of Math Formulas
    // and Integrals, pp. 124-127.

    static inline value_type acos(const_reference t)
    {
        return FLOAT_I * log(t + sqrt(t * t - 1.0f));
    }
    static inline value_type acosh(const_reference t)
    {
        return log(t + sqrt(t * t - 1.0f));
    }
    static inline value_type asin(const_reference t)
    {
        return -FLOAT_I * log(FLOAT_I * t + sqrt(1.0f - t * t));
    }
    static inline value_type asinh(const_reference t)
    {
        return log(t + sqrt(t * t + 1.0f));
    }
    static inline value_type atan(const_reference t)
    {
        return log((1.0f + FLOAT_I * t) / (1.0f - FLOAT_I * t)) / (2.0f
                * FLOAT_I);
    }
    static inline value_type atan2(const_reference y, const_reference x)
    {
        return atan(y / x);
    }
    static inline value_type atanh(const_reference t)
    {
        return 0.5f * log((1.0f + t) / (1.0f - t));
    }

    //*********************************************************
    // exponential functions

    static inline value_type exp(const_reference t)
    {
        return std::exp(t);
    }
    static inline value_type log(const_reference t)
    {
        return std::log(t);
    }
    static inline value_type log10(const_reference t)
    {
        return std::log10(t);
    }
    static inline value_type pow(const_reference t, int e)
    {
        return std::pow(t, e);
    }
    static inline value_type pow(const_reference t, const_reference e)
    {
        return std::pow(t, e);
    }

    //*********************************************************
    // functions specific to complex<float> return values.

    static inline value_type pow(const_reference t, real_type e)
    {
        return std::pow(t, e);
    }
    static inline value_type pow(real_type t, const_reference e)
    {
        return std::pow(t, e);
    }

};

//*********************************************************
// conversion functions specializations for double

inline math_traits<double>::value_type to_degrees(
        math_traits<double>::const_reference t)
{
    return math_traits<double>::to_degrees(t);
}

inline math_traits<double>::value_type to_radians(
        math_traits<double>::const_reference t)
{
    return math_traits<double>::to_radians(t);
}

inline math_traits<double>::value_type to_latitude(
        math_traits<double>::const_reference t)
{
    return math_traits<double>::to_latitude(t);
}

inline math_traits<double>::value_type to_colatitude(
        math_traits<double>::const_reference t)
{
    return math_traits<double>::to_colatitude(t);
}

inline math_traits<double>::value_type copysign(
        math_traits<double>::const_reference t,
        math_traits<double>::const_reference v )
{
    return math_traits<double>::copysign(t,v);
}

//*********************************************************
// add GNU C++ math functions to Visual C++

#ifdef _MSC_VER     // Microsoft Visual C++
    #ifndef NAN 
        #define NAN std::numeric_limits<double>::quiet_NaN()
    #endif
    inline int isnan(double x) { return _isnan(x); }

#if (_MSC_VER < 1800 )  // Visual Sudio eariler than 2013

	inline int round(double x) { return floor(x + 0.5); }

	inline math_traits<double>::value_type acosh(
        math_traits<double>::const_reference t)
    {
        return math_traits<double>::acosh(t);
    }
    inline math_traits<double>::value_type asinh(
        math_traits<double>::const_reference t)
    {
        return math_traits<double>::asinh(t);
    }
    inline math_traits<double>::value_type atanh(
        math_traits<double>::const_reference t)
    {
        return math_traits<double>::atanh(t);
    }

#endif

#endif
} // end of ublas namespace
} // end of usml namespace
