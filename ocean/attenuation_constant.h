/**
 * @file attenuation_constant.h
 * Models attenuation loss as a constant factor.
 */
#pragma once

#include <usml/ocean/attenuation_model.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

namespace usml {
namespace ocean {

using namespace usml::types;

/// @ingroup profiles
/// @{

/**
 * Models attenuation loss as a constant factor that multiplies
 * distance and frequency.
 */
class USML_DECLSPEC attenuation_constant : public attenuation_model {
   public:
    /**
     * Initialize model with a constant factor.
     *
     * @param   coefficient     Attenuation coefficient (dB/m/Hz).
     */
    attenuation_constant(double coefficient) : _coefficient(coefficient) {}

    /**
     * Computes the broadband absorption loss of sea water.
     *
     * @param location      Location at which to compute attenuation.
     * @param frequencies   Frequencies over which to compute loss. (Hz)
     * @param distance      Distance travelled through the water (meters).
     * @param attenuation   Absorption loss of sea water in dB (output).
     */
    void attenuation(const wposition& location, seq_vector::csptr frequencies,
                     const matrix<double>& distance,
                     matrix<vector<double> >* attenuation) const override;

   private:
    /** Holds the attenuation coefficient dB/m/Hz. */
    double _coefficient;
};

/// @}
}  // end of namespace ocean
}  // end of namespace usml
