/**
 * @file window.h
 * Assortment of digital signal processing window functions.
 */
#pragma once

#include <usml/types/seq_linear.h>
#include <usml/types/seq_vector.h>
#include <usml/ublas/math_traits.h>
#include <usml/ublas/vector_math.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_expression.hpp>
#include <cmath>

namespace usml {
namespace transmit {

using namespace usml::types;

/// @ingroup transmit
/// @{

/**
 * Assortment of digital signal processing window functions.
 */
class USML_DECLSPEC window {
   public:
    /// Waveform shading window.
    enum class type {
        BOXCAR = 0,   ///< rectangular envelope
        HANN = 1,     ///< squared sine window by Julius von Hann
        HAMMING = 2,  ///< raised cosine window by Richard W. Hamming
        KAISER = 3    ///< Kaiser-Bessel approximation of DPSS window
    };

    /**
     * Shading defined by enumeration.
     *
     * @param   type    Shading window type.
     * @param	N		Number of points in window.
     * @param	beta	Shape factor.
     * @return			Vector of window samples.
     */
    static vector<double> any(window::type type, size_t N, double beta = 0.5) {
        switch (type) {
            case type::HANN:
                return hann(N);
            case type::HAMMING:
                return hamming(N);
            case type::KAISER:
                return kaiser(N, beta);
            default:
                return boxcar(N);
        }
    }

    /**
     * Uniform shading.
     *
     * @param	N	Number of points in window.
     * @return		Vector of window samples.
     */
    static vector<double> boxcar(size_t N) {
        return scalar_vector<double>(N, 1.0);
    }

    /**
     * Squared sine window by Julius von Hann.
     *
     * @param	N	Number of points in window.
     * @return		Vector of window samples.
     */
    static vector<double> hann(size_t N) {
        const double M = N - 1;
        const vector<double> x = seq_linear(0.0, 1.0, N).data();
        return 0.5 * (1.0 - cos((TWO_PI / M) * x));
    }

    /**
     * Raised cosine window by Richard W. Hamming.
     *
     * @param	N	Number of points in window.
     * @return		Vector of window samples.
     */
    static vector<double> hamming(size_t N) {
        const int M = N - 1;
        const vector<double> x = seq_linear(0.0, 1.0, N).data();
        return 0.54 - 0.46 * cos((TWO_PI / M) * x);
    }

    /**
     * Kaiser-Bessel approximation of DPSS window.
     *
     * @param	N		Number of points in window.
     * @param	beta	Shape factor.
     * @return			Vector of window samples.
     */
    static vector<double> kaiser(size_t N, double beta) {
        const double M = (N - 1.0) / 2.0;
        vector<double> x = seq_linear(0.0, 1.0, N).data();
        for (int n = 0; n < x.size(); ++n) {
            x[n] = std::cyl_bessel_i(0.0, beta * sqrt(1.0 - (x[n] - M) / M)) /
                   std::cyl_bessel_i(0.0, beta);
        }
        return x;
    }
};
/// @}
}  // namespace transmit
}  // namespace usml
