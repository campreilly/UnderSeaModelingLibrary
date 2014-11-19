/**
 * @file scattering_lambert.h
 */

#ifndef USML_OCEAN_SCATTERING_LAMBERT_H
#define USML_OCEAN_SCATTERING_LAMBERT_H

#include <usml/ocean/scattering_model.h>

namespace usml {
namespace ocean {

using namespace usml::ublas ;
using namespace usml::types ;

using boost::numeric::ublas::vector;

/// @ingroup boundaries
/// @{

/**
 * Lambert's Law defines the scattering strength from an ideal diffuse
 * reflector. In optics, the radiant intensity (power per unit solid angle)
 * is constant at all observed angle for diffuse reflectors. Mathematically,
 * this leads to the relationship.
 *\f[
 * \mathcal{I}_{scat} \propto \mathcal{I}_{inc} sin(\alpha_I)
 * sin(\alpha_S) \delta A
 *\f]
 * In acoustics, this is often referred to as the Mackenzie model, based on
 * at-sea measurements that indicated that indicated that Lambert's Law
 * was also a good fit for ocean bottom backscattering strength.
 *
 * @xref Mackenzie K. V., "Bottom reverberation for 530 and 1030 cps
 * Sound in Deep Water," J. Acoust. Soc. Am. 33:1596 (1961).
 */
class USML_DECLSPEC scattering_lambert : public scattering_model {

    public:

        /**
         * Initializes scattering strength model with a Mackenzie coefficient.
         * Defaults to -27 dB.
         *
         * @param	bss		Bottom scattering strength coefficent (dB)
         */
        scattering_lambert( double bss = -27.0 ) : _bss(pow(10.0,bss/10.0)) {}

        /**
         * Destructor
         */
        virtual ~scattering_lambert() {} ;

        /**
         * Computes the broadband scattering strength for a single location.
         *
         * @param location      Location at which to compute attenuation.
         * @param frequencies   Frequencies over which to compute loss. (Hz)
         * @param de_incident   Depression incident angle (radians).
         * @param de_scattered  Depression scattered angle (radians).
         * @param az_incident   Azimuthal incident angle (radians).
         * @param az_scattered  Azimuthal scattered angle (radians).
         * @param amplitude     Change in ray strength in dB (output).
         */
        virtual void scattering_strength( const wposition1& location,
            const seq_vector& frequencies, double de_incident, double de_scattered,
            double az_incident, double az_scattered, vector<double>* amplitude ) ;

    private:

        /**
         * Bottom scattering strength coefficient in linear units.
         */
        double _bss ;
};

/// @}
}   // end of namespace ocean
}   // end of namespace usml

#endif
