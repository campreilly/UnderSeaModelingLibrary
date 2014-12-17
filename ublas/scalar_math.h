/** 
 * @file scalar_math.h
 * Expression template functors to create new math algorithms for scientific
 * applications. 
 */
#pragma once

#include <usml/ublas/math_traits.h>

namespace usml {
namespace ublas {

    /**
     * @internal
     * Expression template functors to create new math algorithms for scientific
     * applications.  These routines use math_traits<> to invoke a generalized
     * version of transcendental functions of real and complex numbers.
     * These include:
     *
     *    - limiting functions: max(), min(), floor(), ceil()
     *    - algebraic functions: abs(), abs2(), arg(), sqrt(), copysign()
     *    - trigonometric functions: cos(), cosh(), sin(), sinh(), tan(), tanh()
     *    - inverse trig functions: acos(), acosh(), asin(), asinh(),
     *              atan(), atan2(), atanh()
     *    - exponential functions: exp(), log(), log10(), pow()
     *    - signal processing functions: signal(), asignal()
     *
     * Based on the design of the uBLAS scalar functor classes.
     *
     * Developers will not usually use these classes directly.
     * They are designed to evaluate the expression templates
     * in the vector_math.h and matrix_math.h headers.
     */

    //**********************************************************
    // limiting functions

    /**
     * @internal
     * Maximum value of two arguments.
     */
    template<class T1, class T2>
    struct scalar_max:
        public scalar_binary_functor<T1, T2> {
        typedef typename scalar_binary_functor<T1, T2>::argument1_type
			 argument1_type;
        typedef typename scalar_binary_functor<T1, T2>::argument2_type
			 argument2_type;
        typedef typename scalar_binary_functor<T1, T2>::result_type
			 result_type;

        static inline result_type apply(argument1_type t1, argument2_type t2) {
            return math_traits<result_type>::max(t1,t2) ;
        }
    };

    /**
     * @internal
     * Minimum value of two arguments.
     */
    template<class T1, class T2>
    struct scalar_min:
        public scalar_binary_functor<T1, T2> {
        typedef typename scalar_binary_functor<T1, T2>::argument1_type
			 argument1_type;
        typedef typename scalar_binary_functor<T1, T2>::argument2_type
			 argument2_type;
        typedef typename scalar_binary_functor<T1, T2>::result_type
			 result_type;

        static inline result_type apply(argument1_type t1, argument2_type t2) {
            return math_traits<result_type>::min(t1,t2) ;
        }
    };

    /**
     * @internal
     * Rounding a scalar down to the nearest integer.
     */
    template<class T>
    struct scalar_floor:
        public scalar_unary_functor<T> {
        typedef typename scalar_unary_functor<T>::argument_type argument_type;
        typedef typename scalar_unary_functor<T>::result_type result_type;

        static inline result_type apply(argument_type t) {
            return math_traits<result_type>::floor(t) ;
        }
    };

    /**
     * @internal
     * Rounding a scalar up to the nearest integer.
     */
    template<class T>
    struct scalar_ceil:
        public scalar_unary_functor<T> {
        typedef typename scalar_unary_functor<T>::argument_type argument_type;
        typedef typename scalar_unary_functor<T>::result_type result_type;

        static inline result_type apply(argument_type t) {
            return math_traits<result_type>::ceil(t) ;
        }
    };

    //**********************************************************
    // conversion functions

    /**
     * @internal
     * Convert angle value in radians to degrees.
     */
    template<class T>
    struct scalar_to_degrees:
        public scalar_real_unary_functor<T> {
        typedef typename scalar_real_unary_functor<T>::argument_type
			 argument_type;
        typedef typename scalar_real_unary_functor<T>::result_type
			 result_type;

        static inline result_type apply(argument_type t) {
            return math_traits<result_type>::to_degrees(t);
        }
    };

    /**
     * @internal
     * Convert angle value in degrees to radians.
     */
    template<class T>
    struct scalar_to_radians:
        public scalar_real_unary_functor<T> {
        typedef typename scalar_real_unary_functor<T>::argument_type
			 argument_type;
        typedef typename scalar_real_unary_functor<T>::result_type
			 result_type;

        static inline result_type apply(argument_type t) {
            return math_traits<result_type>::to_radians(t);
        }
    };

    /**
     * @internal
     * Convert colatitude (radians from north) value to 
     * latitude (degrees from equator).
     */
    template<class T>
    struct scalar_to_latitude:
        public scalar_real_unary_functor<T> {
        typedef typename scalar_real_unary_functor<T>::argument_type
			 argument_type;
        typedef typename scalar_real_unary_functor<T>::result_type
			 result_type;

        static inline result_type apply(argument_type t) {
            return math_traits<result_type>::to_latitude(t);
        }
    };

    /**
     * @internal
     * Convert latitude (degrees from equator) value to 
     * colatitude (radians from north).
     */
    template<class T>
    struct scalar_to_colatitude:
        public scalar_real_unary_functor<T> {
        typedef typename scalar_real_unary_functor<T>::argument_type
			 argument_type;
        typedef typename scalar_real_unary_functor<T>::result_type
			 result_type;

        static inline result_type apply(argument_type t) {
            return math_traits<result_type>::to_colatitude(t);
        }
    };

    //**********************************************************
    // algebraic functions

    /**
     * @internal
     * Magnitude of a complex scalar.
     */
    template<class T>
    struct scalar_abs:
        public scalar_real_unary_functor<T> {
        typedef typename scalar_real_unary_functor<T>::argument_type
			 argument_type;
        typedef typename scalar_real_unary_functor<T>::result_type
			 result_type;

        static inline result_type apply(argument_type t) {
            return math_traits<result_type>::abs(t);
        }
    };

    /**
     * @internal
     * Magnitude squared of a complex scalar.
     */
    template<class T>
    struct scalar_abs2:
        public scalar_real_unary_functor<T> {
        typedef typename scalar_real_unary_functor<T>::argument_type
			 argument_type;
        typedef typename scalar_real_unary_functor<T>::result_type
			 result_type;

        static inline result_type apply(argument_type t) {
            return math_traits<result_type>::abs2(t);
        }
    };

    /**
     * @internal
     * Phase of a complex scalar.
     */
    template<class T>
    struct scalar_arg:
        public scalar_real_unary_functor<T> {
        typedef typename scalar_real_unary_functor<T>::argument_type
			 argument_type;
        typedef typename scalar_real_unary_functor<T>::result_type
			 result_type;

        static inline result_type apply(argument_type t) {
            return math_traits<result_type>::arg(t);
        }
    };

    /**
     * @internal
     * Square root of a scalar.
     */
    template<class T>
    struct scalar_sqrt:
        public scalar_unary_functor<T> {
        typedef typename scalar_unary_functor<T>::argument_type argument_type;
        typedef typename scalar_unary_functor<T>::result_type result_type;

        static inline result_type apply(argument_type t) {
            return math_traits<result_type>::sqrt(t);
        }
    };

    /**
     * @internal
     * Copy sign of a number.
     */
    template<class T1, class T2>
    struct scalar_copysign:
        public scalar_binary_functor<T1, T2> {
        typedef typename scalar_binary_functor<T1, T2>::argument1_type
			 argument1_type;
        typedef typename scalar_binary_functor<T1, T2>::argument2_type
			 argument2_type;
        typedef typename scalar_binary_functor<T1, T2>::result_type
			 result_type;

        static inline result_type apply(argument1_type t1, argument2_type t2) {
            return math_traits<result_type>::copysign(t1,t2) ;
        }
    };

    //**********************************************************
    // trigonometric functions

    /**
     * @internal
     * Cosine of a scalar.
     */
    template<class T>
    struct scalar_cos:
        public scalar_unary_functor<T> {
        typedef typename scalar_unary_functor<T>::argument_type argument_type;
        typedef typename scalar_unary_functor<T>::result_type result_type;

        static inline result_type apply(argument_type t) {
            return math_traits<result_type>::cos(t);
        }
    };

    /**
     * @internal
     * Hyperbolic cosine of a scalar.
     */
    template<class T>
    struct scalar_cosh:
        public scalar_unary_functor<T> {
        typedef typename scalar_unary_functor<T>::argument_type argument_type;
        typedef typename scalar_unary_functor<T>::result_type result_type;

        static inline result_type apply(argument_type t) {
            return math_traits<result_type>::cosh(t);
        }
    };

    /**
     * @internal
     * Sine of a scalar.
     */
    template<class T>
    struct scalar_sin:
        public scalar_unary_functor<T> {
        typedef typename scalar_unary_functor<T>::argument_type argument_type;
        typedef typename scalar_unary_functor<T>::result_type result_type;

        static inline result_type apply(argument_type t) {
            return math_traits<result_type>::sin(t);
        }
    };

    /**
     * @internal
     * Hyperbolic sine of a scalar.
     */
    template<class T>
    struct scalar_sinh:
        public scalar_unary_functor<T> {
        typedef typename scalar_unary_functor<T>::argument_type argument_type;
        typedef typename scalar_unary_functor<T>::result_type result_type;

        static inline result_type apply(argument_type t) {
            return math_traits<result_type>::sinh(t);
        }
    };

    /**
     * @internal
     * Tangent of a scalar.
     */
    template<class T>
    struct scalar_tan:
        public scalar_unary_functor<T> {
        typedef typename scalar_unary_functor<T>::argument_type argument_type;
        typedef typename scalar_unary_functor<T>::result_type result_type;

        static inline result_type apply(argument_type t) {
            return math_traits<result_type>::tan(t);
        }
    };

    /**
     * @internal
     * Hyperbolic tangent of a scalar.
     */
    template<class T>
    struct scalar_tanh:
        public scalar_unary_functor<T> {
        typedef typename scalar_unary_functor<T>::argument_type argument_type;
        typedef typename scalar_unary_functor<T>::result_type result_type;

        static inline result_type apply(argument_type t) {
            return math_traits<result_type>::tanh(t);
        }
    };

    //**********************************************************
    // inverse trigonometric functions

    /**
     * @internal
     * Inverse cosine of a scalar.
     */
    template<class T>
    struct scalar_acos:
        public scalar_unary_functor<T> {
        typedef typename scalar_unary_functor<T>::argument_type argument_type;
        typedef typename scalar_unary_functor<T>::result_type result_type;

        static inline result_type apply(argument_type t) {
            return math_traits<result_type>::acos(t);
        }
    };

    /**
     * @internal
     * Inverse hyperbolic cosine of a scalar.
     */
    template<class T>
    struct scalar_acosh:
        public scalar_unary_functor<T> {
        typedef typename scalar_unary_functor<T>::argument_type argument_type;
        typedef typename scalar_unary_functor<T>::result_type result_type;

        static inline result_type apply(argument_type t) {
            return math_traits<result_type>::acosh(t);
        }
    };

    /**
     * @internal
     * Inverse sine of a scalar.
     */
    template<class T>
    struct scalar_asin:
        public scalar_unary_functor<T> {
        typedef typename scalar_unary_functor<T>::argument_type argument_type;
        typedef typename scalar_unary_functor<T>::result_type result_type;

        static inline result_type apply(argument_type t) {
            return math_traits<result_type>::asin(t);
        }
    };

    /**
     * @internal
     * Inverse hyperbolic sine of a scalar.
     */
    template<class T>
    struct scalar_asinh:
        public scalar_unary_functor<T> {
        typedef typename scalar_unary_functor<T>::argument_type argument_type;
        typedef typename scalar_unary_functor<T>::result_type result_type;

        static inline result_type apply(argument_type t) {
            return math_traits<result_type>::asinh(t);
        }
    };

    /**
     * @internal
     * Inverse tangent of a scalar.
     */
    template<class T>
    struct scalar_atan:
        public scalar_unary_functor<T> {
        typedef typename scalar_unary_functor<T>::argument_type argument_type;
        typedef typename scalar_unary_functor<T>::result_type result_type;

        static inline result_type apply(argument_type t) {
            return math_traits<result_type>::atan(t);
        }
    };

    /**
     * @internal
     * Inverse tangent of scalars y / x.
     */
    template<class T1, class T2>
    struct scalar_atan2:
        public scalar_binary_functor<T1, T2> {
        typedef typename scalar_binary_functor<T1, T2>::argument1_type
                         argument1_type;
        typedef typename scalar_binary_functor<T1, T2>::argument2_type
                         argument2_type;
        typedef typename scalar_binary_functor<T1, T2>::result_type
                         result_type;

        static inline result_type apply(argument1_type y, argument2_type x) {
            return math_traits<result_type>::atan2(y,x) ;
        }
    };

    /**
     * @internal
     * Inverse hyperbolic tangent of a scalar.
     */
    template<class T>
    struct scalar_atanh:
        public scalar_unary_functor<T> {
        typedef typename scalar_unary_functor<T>::argument_type argument_type;
        typedef typename scalar_unary_functor<T>::result_type result_type;

        static inline result_type apply(argument_type t) {
            return math_traits<result_type>::atanh(t);
        }
    };

    //**********************************************************
    // exponential functions

    /**
     * @internal
     * Exponential of a scalar.
     */
    template<class T>
    struct scalar_exp:
        public scalar_unary_functor<T> {
        typedef typename scalar_unary_functor<T>::argument_type argument_type;
        typedef typename scalar_unary_functor<T>::result_type result_type;

        static inline result_type apply(argument_type t) {
            return math_traits<result_type>::exp(t);
        }
    };

    /**
     * @internal
     * Natural logarithm of a scalar.
     */
    template<class T>
    struct scalar_log:
        public scalar_unary_functor<T> {
        typedef typename scalar_unary_functor<T>::argument_type argument_type;
        typedef typename scalar_unary_functor<T>::result_type result_type;

        static inline result_type apply(argument_type t) {
            return math_traits<result_type>::log(t);
        }
    };

    /**
     * @internal
     * Base 10 logarithm of a scalar.
     */
    template<class T>
    struct scalar_log10:
        public scalar_unary_functor<T> {
        typedef typename scalar_unary_functor<T>::argument_type argument_type;
        typedef typename scalar_unary_functor<T>::result_type result_type;

        static inline result_type apply(argument_type t) {
            return math_traits<result_type>::log10(t);
        }
    };

    /**
     * @internal
     * Real or complex value raised to a power.
     */
    template<class T1, class T2>
    struct scalar_pow:
        public scalar_binary_functor<T1, T2> {
        typedef typename scalar_binary_functor<T1, T2>::argument1_type
			 argument1_type;
        typedef typename scalar_binary_functor<T1, T2>::argument2_type
			 argument2_type;
        typedef typename scalar_binary_functor<T1, T2>::result_type
			 result_type;

        static inline result_type apply(argument1_type t1, argument2_type t2) {
            return math_traits<result_type>::pow(t1,t2) ;
        }
    };

    //**********************************************************
    // signal processing functions

    /**
     * @internal
     * Converts a real phase angle into a real valued signal.
     * The sin(t) form is used to make the phase zero at t=0.
     */
    template<class T>
    struct scalar_signal:
        public scalar_real_unary_functor<T> {
        typedef typename scalar_real_unary_functor<T>::argument_type
			 argument_type;
        typedef typename scalar_real_unary_functor<T>::result_type
			 result_type;

        static inline result_type apply(argument_type t) {
            return math_traits<result_type>::sin(t);
        }
    };

    /**
     * @internal
     * Converts a real phase angle into a complex analytic signal.
     * The real part of this result is equivalent to scalar_signal(t).
     */
    template<class T>
    struct scalar_asignal {
        typedef T argument_type;
        typedef complex<T> result_type ;

        static inline result_type apply(argument_type t) {
            return math_traits<result_type>::sin(t);
            return result_type( math_traits<T>::sin(t),
			        math_traits<T>::cos(t) ) ;
        }
    };

} // end of ublas namespace
} // end of usml namespace
