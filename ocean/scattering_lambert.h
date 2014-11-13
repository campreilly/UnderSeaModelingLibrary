/**
 * @file scattering_lambert.h
 */

#ifndef USML_OCEAN_SCATTERING_LAMBERT_H
#define USML_OCEAN_SCATTERING_LAMBERT_H

#include <usml/ublas/ublas.h>
#include <usml/types/types.h>
#include <usml/ocean/scattering_model.h>

namespace usml {
namespace ocean {

using namespace usml::ublas ;
using namespace usml::types ;

using boost::numeric::ublas::vector;

/**
 * Scattering model that follows Lambert's Law:
 *
 *\f[
 * \mathcal{I}_{scat} \propto \mathcal{I}_{inc} sin(\alpha_I)
 * sin(\alpha_S) \delta A
 *\f]
 */

class USML_DECLSPEC scattering_lambert : public scattering_model {

    public:

        /**
         * Constructor
         */
        scattering_lambert( double bss ) : _bss(bss) {}

        /**
         * Default Constructor
         */
        scattering_lambert() {
            _bss = pow(10.0,-2.7) ;
        }

        /**
         * Destructor
         */
        virtual ~scattering_lambert() {} ;

        /**
         * Computes the broadband reflection loss and phase change.
         *
         * @param location      NOT USED
         * @param frequencies   Frequencies over which to compute loss. (Hz)
         * @param angleI        Depression incident angle (radians).
         * @param angleS        Depression scattered angle (radians).
         * @param azI           NOT USED
         * @param azS           NOT USED.
         * @param amplitude     Change in ray strength in linear units (output).
         * @param phase         NOT USED
         *
         * NOTE: All angles are relative to the scattering interface.
         */
        virtual void scattering_strength( const wposition1& location,
            const seq_vector& frequencies, double angleI, double angleS,
            double azI, double azS, vector<double>* amplitude,
            vector<double>* phase=NULL ) ;

        /**
         * Computes the broadband reflection loss and phase change.
         *
         * @param location      NOT USED
         * @param frequencies   Frequencies over which to compute loss. (Hz)
         * @param angleI        vector of Depression incident angle (radians).
         * @param angleS        vector of Depression scattered angle (radians).
         * @param azI           NOT USED
         * @param azS           NOT USED.
         * @param amplitude     vector of the change in ray strength in
         *                      linear units (output).
         *                      Where vector<vector<double>(size locations)>(size freqs)
         * @param phase         NOT USED
         *
         * NOTE: All angles are relative to the scattering interface.
         */
        virtual void scattering_strength( const wposition& location,
            const seq_vector& frequencies, const vector<double>& angleI,
            const vector<double>& angleS, const vector<double>& azI,
            const vector<double>& azS, vector<vector<double> >* amplitude,
            vector<vector<double> >* phase=NULL ) ;

    private:

        /**
         * Back scattering strength
         */
        double _bss ;


};

}   // end of namespace ocean
}   // end of namespace usml

#endif
