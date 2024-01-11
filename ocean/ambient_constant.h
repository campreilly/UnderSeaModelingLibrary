/**
 * @file ambient_constant.h
 * Constant model for ambient noise in the ocean.
 */
#pragma once

#include <usml/ocean/ambient_model.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition1.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/expression_types.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <cmath>

namespace usml {
namespace ocean {

using boost::numeric::ublas::vector;

/// @ingroup profiles
/// @{

/**
 * Ambient constant returns the same fixed value for all frequencies.
 * The constant value is set during construction.
 */
class USML_DECLSPEC ambient_constant : public ambient_model {
   public:
    /**
     * Initialize model with a constant factor.
     *
     * @param   coefficient   Ambient noise power spectral density (dB).
     */
    ambient_constant(double coefficient) {
        _coefficient = std::pow(10.0, 0.1 * coefficient);
    }

    /**
     * Computes the power spectral density of ambient noise. This implementation
     * returns the same power spectral density for all frequencies and
     * locations.
     *
     * @param location  Location at which to compute noise.
     * @param frequency Frequencies at which noise is calculated.
     * @param noise     Ambient noise power spectral density (output)
     */
    virtual void ambient(const wposition1 &location,
                         const seq_vector::csptr &frequency,
                         vector<double> *noise) const {
        *noise = scalar_vector<double>(frequency->size(), _coefficient);
    }

   private:
    /** Ambient noise power spectral density. */
    double _coefficient;
};

/// @}
}  // end of namespace ocean
}  // end of namespace usml
