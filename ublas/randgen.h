/**
 * @file randgen.h
 * Random generator that uses a common seed across multiple instantiations.
 */
#pragma once

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <complex>
#include <random>

namespace usml {
namespace ublas {

using namespace boost::numeric::ublas;

/// @ingroup randgen
/// @{

/**
 * Random generator that uses a common seed across multiple instantiations.
 */
class randgen {
   private:
    /**
     * Master generator that underlies all distributions. This implementation
     * instantiates a shuffle order adaptor on a linear congruential engine.
     */
    std::knuth_b _generator;

    /// Uniform random number distribution for the range [0,1).
    std::uniform_real_distribution<double> _uniform_dist;

    /// Gaussian random number distribution with mean=0, sigma=1.
    std::normal_distribution<double> _gaussian_dist;

   public:
    /**
     * Build a new random number generator with random seed.
     */
    randgen();

    /**
     * Build a new random number generator with specific seed.
     *
     * @param seed  New seed for the random number generator.
     */
    randgen(unsigned seed);

    /**
     * Update the random number generator with a specific seed.
     *
     * @param seed  New seed for the random number generator.
     */
    void seed(unsigned seed);

    //******************************************************************
    // Scalar random numbers

    /**
     * Generate a single random number from a Uniform distribution.
     */
    double uniform();

    /**
     * Generate a single random number from a Gaussian distribution.
     */
    double gaussian();

    /**
     * Generate a single random number from a complex Gaussian distribution.
     */
    std::complex<double> cgaussian();

    //******************************************************************
    // Vector random numbers

    /**
     * Generate a vector of random numbers from a Uniform distribution.
     *
     * @param    N        Length of the output vector.
     */
    vector<double> uniform(int N);

    /**
     * Generate a vector of random numbers from a Gaussian distribution.
     *
     * @param    N        Length of the output vector.
     */
    vector<double> gaussian(int N);

    /**
     * Generate a vector of complex random numbers from a
     * Gaussian distribution.
     *
     * @param    N        Length of the output vector.
     */
    vector<std::complex<double>> cgaussian(int N);

    //******************************************************************
    // Matrix random numbers

    /**
     * Generate a matrix of random numbers from a Uniform distribution.
     *
     * @param    N        Rows of the output matrix.
     * @param    M        Columns of the output matrix.
     */
    matrix<double> uniform(int N, int M);

    /**
     * Generate a matrix of random numbers from a Gaussian distribution.
     *
     * @param    N        Rows of the output matrix.
     * @param    M        Columns of the output matrix.
     */
    matrix<double> gaussian(int N, int M);

    /**
     * Generate a matrix of complex random numbers from a
     * Gaussian distribution.
     *
     * @param    N        Rows of the output matrix.
     * @param    M        Columns of the output matrix.
     */
    matrix<std::complex<double>> cgaussian(int N, int M);
};

/// @}
}  // namespace ublas
}  // namespace usml
