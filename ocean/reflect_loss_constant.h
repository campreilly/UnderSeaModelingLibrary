/**
 * @file reflect_loss_constant.h
 * Models reflection loss as a constant factor.
 */
#ifndef USML_OCEAN_REFLECT_LOSS_CONSTANT_H
#define USML_OCEAN_REFLECT_LOSS_CONSTANT_H

#include <usml/ocean/reflect_loss_model.h>

namespace usml {
namespace ocean {

using boost::numeric::ublas::vector;

/// @ingroup boundaries
/// @{

/**
 * Models reflection loss as a constant factor that is independent
 * of grazing angle and frequency.
 */
class USML_DECLSPEC reflect_loss_constant : public reflect_loss_model {

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

        /**
         * Computes the broadband reflection loss and phase change for
         * multiple locations.
         *
         * @param location      Location at which to compute attenuation.
         * @param frequencies   Frequencies over which to compute loss. (Hz)
         * @param angle         Reflection angle relative to the normal (radians).
         * @param amplitude     Change in ray strength in dB (output).
         *                      Where vector<vector<double>(size locations)>(size freqs)
         * @param phase         Change in ray phase in radians (output).
         *                      Phase change not computed if this is NULL.
         * @param linear        returns the value back in linear or log units.
         * @TODO    add this implementation
         */
        virtual void reflect_loss( const wposition& location,
            const seq_vector& frequencies, vector<double>* angle,
            vector<vector<double> >* amplitude,
            vector<vector<double> >* phase=NULL, bool linear=false ) {}

    private:

        /** Holds the reflection amplitude change (dB). */
        double _amplitude ;

        /** Holds the reflection phase change (radians). */
        double _phase ;

} ;

/// @}
}  // end of namespace ocean
}  // end of namespace usml

#endif
