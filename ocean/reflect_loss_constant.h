/** 
 * @file reflect_loss_constant.h
 * Models reflection loss as a constant factor.
 */
#ifndef USML_OCEAN_REFLECT_LOSS_CONSTANT_H
#define USML_OCEAN_REFLECT_LOSS_CONSTANT_H

#include <usml/ocean/reflect_loss_model.h>

namespace usml {
namespace ocean {

/// @ingroup boundaries
/// @{

/**
 * Models reflection loss as a constant factor that is independent
 * of grazing angle and frequency.
 */
class reflect_loss_constant : public reflect_loss_model {

  private:

    /** Holds the reflection amplitude change (dB). */
    double _amplitude ;

    /** Holds the reflection phase change (radians). */
    double _phase ;

  public:

    /**
     * Initialize model with a constant factors.
     *
     * @param amplitude     Reflection amplitude change (dB).
     * @param phase         Reflection phase change (radians).
     */
    reflect_loss_constant( double amplitude, double phase=0.0 ) : 
        _amplitude(amplitude), _phase(phase) {}

    /**
     * Computes the broadband reflection loss and phase change.
     *
     * @param location      Location at which to compute attenuation.
     * @param frequencies   Frequencies over which to compute loss. (Hz)
     * @param angle         Grazing angle relative to the interface (radians).
     * @param amplitude     Change in ray strength in dB (output).
     * @param phase         Change in ray phase in radians (output).
     *                      Phase change not computed if this is NULL.
     */
    virtual void reflect_loss( 
        const wposition1& location, 
        const seq_vector& frequencies, double angle,
        vector<double>* amplitude, vector<double>* phase=NULL ) ;
        
} ;

/// @}
}  // end of namespace ocean
}  // end of namespace usml

#endif
