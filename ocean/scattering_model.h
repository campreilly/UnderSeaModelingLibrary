/**
 * @file scattering_model.h
 */

#ifndef USML_OCEAN_SCATTERING_MODEL_H
#define USML_OCEAN_SCATTERING_MODEL_H

#include <usml/ublas/ublas.h>
#include <usml/types/types.h>

namespace usml {
namespace ocean {

using namespace usml::ublas ;
using namespace usml::types ;

using boost::numeric::ublas::vector;

/**
 * Scattering strength abstract class. All scattering strength models
 * produce a specular reflection of energy when interacting with the
 * interface. The model then returns an amount of energy, given
 * incident and scattering angles, reflected in that direction.
 */
class USML_DECLSPEC scattering_model {

    public:

        /**
         * Computes the broadband reflection loss and phase change.
         *
         * @param location      Location at which to compute attenuation.
         * @param frequencies   Frequencies over which to compute loss. (Hz)
         * @param angleI        Depression incident angle (radians).
         * @param angleS        Depression scattered angle (radians).
         * @param azI           Azimuthal incident angle (radians).
         * @param azS           Azimuthal scattered angle (radians).
         * @param amplitude     Change in ray strength in dB (output).
         * @param phase         Change in ray phase in radians (output).
         *                      Phase change not computed if this is NULL.
         *
         * NOTE: All angles are relative to the scattering interface.
         */
        virtual void scattering_strength( const wposition1& location,
            const seq_vector& frequencies, double angleI, double angleS,
            double azI, double azS, vector<double>* amplitude,
            vector<double>* phase=NULL ) = 0 ;

        /**
         * Computes the broadband reflection loss and phase change.
         *
         * @param location      Locations at which to compute attenuation.
         * @param frequencies   Frequencies over which to compute loss. (Hz)
         * @param angleI        vector of Depression incident angle (radians).
         * @param angleS        vector of Depression scattered angle (radians).
         * @param azI           vector of Azimuthal incident angle (radians).
         * @param azS           vector of Azimuthal scattered angle (radians).
         * @param amplitude     vector of the change in ray strength in
         *                      linear units (output).
         * @param phase         Change in ray phase in radians (output).
         *                      Phase change not computed if this is NULL.
         *
         * NOTE: All angles are relative to the scattering interface.
         */
        virtual void scattering_strength( const wposition& location,
            const seq_vector& frequencies, const vector<double>& angleI,
            const vector<double>& angleS, const vector<double>& azI,
            const vector<double>& azS, vector<vector<double> >* amplitude,
            vector<vector<double> >* phase=NULL ) = 0 ;

        /**
         * Virtual destructor
         */
        virtual ~scattering_model() {}

};

}   // end namespace ocean
}   // end namespace usml

#endif
