/**
 * @file randgen.cc
 * Random generator that uses a common seed across multiple instantiations.
 */
#include <usml/ublas/randgen.h>

#include <algorithm>
#include <iterator>

using namespace usml::ublas;
using namespace boost::numeric::ublas;

/**
 * Build a new random number generator with random seed.
 */
randgen::randgen() : randgen(std::random_device{}()) {}

/**
 * Build a new random number generator with specific seed.
 */
randgen::randgen(unsigned seed)
    : _generator(seed), _uniform_dist(0.0, 1.0), _gaussian_dist(0.0, 1.0) {}

/**
 * Update the random number generator with a specific seed.
 */
void randgen::seed(unsigned seed) { _generator.seed(seed); }

//******************************************************************
// Scalar random numbers

/**
 * Generate a single random number from a Uniform distribution.
 */
double randgen::uniform() { return _uniform_dist(_generator); }

/**
 * Generate a single random number from a Gaussian distribution.
 */
double randgen::gaussian() { return _gaussian_dist(_generator); }

/**
 * Generate a single random number from a complex Gaussian distribution.
 */
std::complex<double> randgen::cgaussian() {
    return {_gaussian_dist(_generator), _gaussian_dist(_generator)};
}

//******************************************************************
// Vector random numbers

/**
 * Generate a vector of random numbers from a Uniform distribution.
 */
vector<double> randgen::uniform(int N) {
    vector<double> result(N);
    auto gen = [this]() { return _uniform_dist(_generator); };
    std::generate(begin(result), end(result), gen);
    return result;
}

/**
 * Generate a vector of random numbers from a Gaussian distribution.
 */
vector<double> randgen::gaussian(int N) {
    vector<double> result(N);
    auto gen = [this]() { return _gaussian_dist(_generator); };
    std::generate(begin(result), end(result), gen);
    return result;
}

/**
 * Generate a vector of complex random numbers from a
 * Gaussian distribution.
 */
vector<std::complex<double>> randgen::cgaussian(int N) {
    vector<std::complex<double>> result(N);
    auto gen = [this]() {
        return std::complex<double>(_gaussian_dist(_generator),
                                    _gaussian_dist(_generator));
    };
    std::generate(begin(result), end(result), gen);
    return result;
}

//******************************************************************
// Matrix random numbers

/**
 * Generate a matrix of random numbers from a Uniform distribution.
 */
matrix<double> randgen::uniform(int N, int M) {
    matrix<double> result(N, M);
    auto gen = [this]() { return _uniform_dist(_generator); };
    std::generate(begin(result.data()), end(result.data()), gen);
    return result;
}

/**
 * Generate a matrix of random numbers from a Gaussian distribution.
 */
matrix<double> randgen::gaussian(int N, int M) {
    matrix<double> result(N, M);
    auto gen = [this]() { return _gaussian_dist(_generator); };
    std::generate(begin(result.data()), end(result.data()), gen);
    return result;
}

/**
 * Generate a matrix of complex random numbers from a
 * Gaussian distribution.
 */
matrix<std::complex<double>> randgen::cgaussian(int N, int M) {
    matrix<std::complex<double>> result(N, M);
    auto gen = [this]() {
        return std::complex<double>(_gaussian_dist(_generator),
                                    _gaussian_dist(_generator));
    };
    std::generate(begin(result.data()), end(result.data()), gen);
    return result;
}
