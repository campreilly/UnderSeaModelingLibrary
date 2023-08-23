/**
 * @file reflect_loss_constant.h
 * Models reflection loss as a constant factor.
 */
#pragma once

#include <usml/ocean/reflect_loss_model.h>

namespace usml {
namespace ocean {

using boost::numeric::ublas::vector;

/// @ingroup boundaries
/// @{

/**
 * Models surface or bottom reflection loss as a constant factor that is
 * independent of grazing angle and frequency.
 */
class USML_DECLSPEC reflect_loss_constant : public reflect_loss_model {
   public:
    /**
     * Initialize model with a constant factors.
     *
     * @param amplitude     Reflection amplitude change (dB).
     * @param phase         Reflection phase change (radians).
     */
    reflect_loss_constant(double amplitude, double phase = 0.0)
        : _amplitude(amplitude), _phase(phase) {}

    /**
     * Computes the broadband reflection loss and phase change.
     *
     * @param location      Location at which to compute reflection loss.
     * @param frequencies   Frequencies over which to compute loss. (Hz)
     * @param angle         Grazing angle relative to the interface (radians).
     * @param amplitude     Change in ray intensity in dB (output).
     * @param phase         Change in ray phase in radians (output).
     *                      Phase change not computed if this is nullptr.
     */
    void reflect_loss(const wposition1& location, seq_vector::csptr frequencies,
                      double angle, vector<double>* amplitude,
                      vector<double>* phase = nullptr) const override {
        noalias(*amplitude) =
            scalar_vector<double>(frequencies->size(), _amplitude);
        if (phase) {
            noalias(*phase) =
                scalar_vector<double>(frequencies->size(), _phase);
        }
    }

   private:
    /** Holds the reflection amplitude change (dB). */
    double _amplitude;

    /** Holds the reflection phase change (radians). */
    double _phase;
};

/// @}
}  // end of namespace ocean
}  // end of namespace usml
