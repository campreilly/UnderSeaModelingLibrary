/** 
 * @file randgen.h 
 * Integrates the Boost Random Number Library with uBLAS vectors and matrices. 
 */
#pragma once

#include <complex>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/random.hpp>
#include <usml/usml_config.h>

namespace usml {
namespace ublas {

    using std::cout ;
    using std::operator<< ;
    using std::endl ;
    using std::complex ;
    using namespace boost::numeric::ublas ;
    
    using boost::numeric::ublas::vector;

    /// @ingroup randgen
    /// @{

    /**
     * Singleton class for integrating the Boost Random Number Library 
     * with uBLAS vectors and matrices.  Implemented as a singleton to
     * ensure that all random numbers are created from the same 
     * pseudo-random number generation pool.  This is designed to prevent 
     * un-intentional correlation between random series in different 
     * parts of the application.
     *
     * Because all of the methods are declared static, the developer
     * never actually creates a class of this type.
     */
    class randgen {

      public:

        /**
         * Defines the class for the basic random number generator.
         * This implementation combines a linear congruential
         * generator with a shuffling algorithm.
         */
        typedef boost::kreutzer1986 random_type;

      private:

        /**
         * Master generator that underlies all distributions.
         * Seeded with the current system time at initialization.
         */
        static USML_DECLSPEC random_type master_gen;

        /** Uniform random number generator for the range [0,1). */
        static USML_DECLSPEC boost::uniform_01 < random_type, double > uniform_gen;

        /** variate_generator<> requires reference to abstract distribution. */
        static USML_DECLSPEC boost::normal_distribution<double> gaussian_dist ;

        /** Gaussian random number generator with sigma = 1. */
        static USML_DECLSPEC boost::variate_generator< random_type, 
    	    boost::normal_distribution<double> > gaussian_gen;

        //******************************************************************
        // Utility routines

        /**
         * Quickly fill a vector with random numbers.
         * Designed to handle both double and complex<double> vectors.
         *
         * @param    result	Vector of random numbers.
         * @param    rand	Random number generator.
         */
        template < class T, class Random >
        static inline void fill(vector< T > & result, Random & rand) {
            const int N = result.size();
            double * current = (double *) & result(0);
            double * end = (double *) & result(N - 1);
            while (current <= end) {
                * current++ = rand();
            }
        }

        /**
         * Quickly fill a matrix with random numbers.
         * Designed to handle both double and complex<double> matrices.
         *
         * @param    result	Matrix of random numbers.
         * @param    rand	Random number generator.
         */
        template < class T, class Random >
        static inline void fill(matrix< T > & result, Random & rand) {
            const int N = result.size1();
            const int M = result.size2();
            double * current = (double *) & result(0, 0);
            double * end = (double *) & result(N - 1, M - 1);
            while (current <= end) {
                * current++ = rand();
            }
        }

        /**
         * Don't allow users to construct this class.
         */
        randgen() { 
        }

      public:

        /**
         * Initialize the random number generator with a specific seed.
         * This allows the system to re-create random signals from
         * run-to-run.
         *
         * @param    value	New seed for the random number generator.
         */
        static inline void seed(random_type::result_type value) {
            master_gen.seed(value);
        }

        //******************************************************************
        // Scalar random numbers

        /** Generate a single random number from a Uniform distribution. */
        static inline double uniform() {
            return uniform_gen();
        }

        /** Generate a single random number from a Gaussian distribution. */
        static inline double gaussian() {
            return gaussian_gen();
        }

        //******************************************************************
        // Vector random numbers

        /**
         * Generate a vector of random numbers from a Uniform distribution.
         *
         * @param    N		Length of the output vector.
         */
        static inline vector<double> uniform(int N) {
            vector<double> result(N);
            fill(result, uniform_gen);
            return result;
        }

        /**
         * Generate a vector of random numbers from a Gaussian distribution.
         *
         * @param    N		Length of the output vector.
         */
        static inline vector<double> gaussian(int N) {
            vector<double> result(N);
            fill(result, gaussian_gen);
            return result;
        }

        /**
         * Generate a vector of complex random numbers from a
         * Gaussian distribution.
         *
         * @param    N		Length of the output vector.
         */
        static inline vector< complex<double> > noise(int N) {
            vector< complex<double> > result(N);
            fill(result, gaussian_gen);
            return result;
        }

        //******************************************************************
        // Matrix random numbers

        /**
         * Generate a matrix of random numbers from a Uniform distribution.
         *
         * @param    N		Rows of the output matrix.
         * @param    M		Columns of the output matrix.
         */
        static inline matrix<double> uniform(int N, int M) {
            matrix<double> result(N, M);
            fill(result, uniform_gen);
            return result;
        }

        /**
         * Generate a matrix of random numbers from a Gaussian distribution.
         *
         * @param    N		Rows of the output matrix.
         * @param    M		Columns of the output matrix.
         */
        static inline matrix<double> gaussian(int N, int M) {
            matrix<double> result(N, M);
            fill(result, gaussian_gen);
            return result;
        }

        /**
         * Generate a matrix of complex random numbers from a
         * Gaussian distribution.
         *
         * @param    N		Rows of the output matrix.
         * @param    M		Columns of the output matrix.
         */
        static inline matrix< complex<double> > noise(int N, int M) {
            matrix< complex<double> > result(N, M);
            fill(result, gaussian_gen);
            return result;
        }

    };

    /// @}
} // end of ublas namespace
} // end of usml namespace
