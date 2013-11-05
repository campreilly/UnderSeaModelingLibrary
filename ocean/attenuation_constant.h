/** 
 * @file attenuation_constant.h
 * Models attenuation loss as a constant factor.
 */
#ifndef USML_OCEAN_ATTENUATION_CONSTANT_H
#define USML_OCEAN_ATTENUATION_CONSTANT_H

#include <usml/ocean/attenuation_model.h>

namespace usml {
namespace ocean {

using boost::numeric::ublas::vector;

/// @ingroup profiles
/// @{

/**
 * Models attenuation loss as a constant factor that multiplies
 * distance and frequency.
 */
class USML_DECLSPEC attenuation_constant : public attenuation_model {

  private:

    /** Holds the attenuation coefficient dB/m/Hz. */
    double _coefficient;

  public:

    /**
     * Initialize model with a constant factor.
     *
     * @param   coefficient     Attenuation coefficient (dB/m/Hz).
     */
    attenuation_constant( double coefficient ) : _coefficient(coefficient) {}

    /**
     * Computes the broadband absorption loss of sea water.
     *
     * @param location      Location at which to compute attenuation.
     * @param frequencies   Frequencies over which to compute loss. (Hz)
     * @param distance      Distance travelled through the water (meters).
     * @param attenuation   Absorption loss of sea water in dB (output).
     */
    virtual void attenuation( 
        const wposition& location, 
        const seq_vector& frequencies,
        const matrix<double>& distance,
        matrix< vector<double> >* attenuation ) ;
        
} ;

/// @}
}  // end of namespace ocean
}  // end of namespace usml

#endif
