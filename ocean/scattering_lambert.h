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
 * is constant at all observed angle for ideal diffuse reflectors.
 * Mathematically, this leads to the relationship below.
 * \f[
 *   \frac{I_{scat}}{I_{inc}} = \mu \: sin(\gamma_{scat}) * sin(\gamma_{inc}) \: \delta A
 * \f]
 * where
 * 		\f$ \gamma \f$ = grazing angle,
 * 		\f$ \mu \f$ = scattering strength coefficient.
 *
 * In underwater acoustics, this is often referred to as the Mackenzie model,
 * based on at-sea measurements which indicated that indicated that
 * Lambert's Law was also a good fit for ocean bottom backscattering strength.
 *
 * @xref Mackenzie K. V., "Bottom reverberation for 530 and 1030 cps
 * Sound in Deep Water," J. Acoust. Soc. Am. 33:1596 (1961).
 */
class USML_DECLSPEC scattering_lambert : public scattering_model {

public:

    /**
     * Initializes scattering strength model with a Mackenzie coefficient.
     * Defaults to a value of the scattering strength coefficient
     * such that \f$10 log(\mu) = -27 dB\f$.
     *
     * @param	bss		Bottom scattering strength coefficient (dB)
     */
    scattering_lambert( double bss = -27.0 ) :
    	_coeff(pow(10.0,bss/10.0)) {}

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
	virtual void scattering( const wposition1& location,
		const seq_vector& frequencies, double de_incident, double de_scattered,
		double az_incident, double az_scattered, vector<double>* amplitude )
	{
		*amplitude = scalar_vector<double>( frequencies.size(),
				abs( _coeff * sin( de_incident ) * sin( de_scattered ) ) ) ;
				// fast assignment of scalar to vector
	}

private:

	/**
	 * Bottom scattering strength coefficient in linear units.
	 */
	double _coeff ;
};

/// @}
}   // end of namespace ocean
}   // end of namespace usml

#endif
