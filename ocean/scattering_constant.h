/**
 * @file scattering_constant.h
 */

#ifndef USML_OCEAN_SCATTERING_CONSTANT_H
#define USML_OCEAN_SCATTERING_CONSTANT_H

#include <usml/ocean/scattering_model.h>

namespace usml {
namespace ocean {

using namespace usml::ublas;
using namespace usml::types;

using boost::numeric::ublas::vector;

/// @ingroup boundaries
/// @{

/**
 * Models reverberation scattering strength as a constant factor that
 * is independent of grazing angle and frequency.
 */
class USML_DECLSPEC scattering_constant: public scattering_model {

public:

	/**
	 * Initialize model with a constant factors.
	 *
	 * @param amplitude     Reflection amplitude change (dB).
	 */
	scattering_constant( double amplitude=-300.0 ) :
		_amplitude( pow(10.0,amplitude/10.0) ) {}

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
	virtual void scattering(const wposition1& location,
			const seq_vector& frequencies, double de_incident,
			double de_scattered, double az_incident, double az_scattered,
			vector<double>* amplitude)
	{
		*amplitude = scalar_vector<double>( frequencies.size(), _amplitude );
			// fast assignment of scalar to vector
	}

private:

    /** Holds the interface scattering strength (dB). */
    double _amplitude ;

};

/// @}
}	// end of namespace ocean
}   // end of namespace usml

#endif
