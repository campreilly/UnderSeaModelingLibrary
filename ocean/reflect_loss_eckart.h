/**
 * @file reflect_loss_eckart.h
 * Models ocean surface reflection loss using Eckart's model.
 */
#pragma once

#include <usml/ocean/reflect_loss_model.h>
#include <usml/ocean/wave_height_pierson.h>

namespace usml {
namespace ocean {

using boost::numeric::ublas::vector;

/// @ingroup boundaries
/// @{

/**
 * Models ocean surface reflection loss using Eckart's model.
 * \f[
 *   	RL = -20 \: log_{10} \left( exp[ - 0.5 \Gamma^2 ] \right)
 * \f]\f[
 *   	\Gamma = 2 k h sin(\theta)
 * \f]
 * where
 * 		\f$ k = \frac{2 \pi f}{c} \f$ = wave number (1/m),
 * 		\f$ f \f$ = signal frequency (Hz),
 * 		\f$ c \f$ = speed of sound (m/sec),
 * 		\f$ h \f$ = RMS height of wave spectrum (m),
 * 		\f$ \theta \f$ = grazing angle (rad),
 * 		\f$ \Gamma \f$ = Rayleigh roughness parameter, and
 * 		\f$ RL \f$ = reflection loss (dB).
 * Eckart's model arises from a Kirchhoff approximation to scattering
 * and the assumption of a Gaussian probability of surface elevations
 * of standard deviation h.
 *
 * Jones et. al. has shown that many of the terms in this expression can
 * be simplified if we assume that the speed of sound is 1500 m/s and
 * that the wave height is related to wind speed by a Pierson-Moskowitz
 * spectrum for fully developed, wind driven seas.
 * \f[
 *   	RL = 8.6x10^{-9} f^2 w^4 sin^2(\theta)
 * \f]
 * where
 * 		\f$ w \f$ = wind speed (m/sec).
 *
 * @xref Adrian D. Jones, Janice Sendt, Alec J. Duncan, Paul A. Clarke and
 * Amos Maggi, "Modelling the acoustic reflection loss at the rough
 * ocean surface," Proceedings of ACOUSTICS 2009, Australian Acoustical Society,
 * 23-25 November 2009, Adelaide, Australia.
 *
 * @xref C. Eckart, “The scattering of sound from the sea surface,”
 * J. Acoust. Soc. Am. 25, 560–570 (1953).
 */
class USML_DECLSPEC reflect_loss_eckart: public reflect_loss_model {

public:

	/**
	 * Initializes ocean surface reflection loss using Eckart's model.
	 *
	 * @param wind_speed	Wind_speed used to develop rough seas (m/s)
	 */
	reflect_loss_eckart( double wind_speed ) :
		_wind_speed2( wind_speed * wind_speed )
	{
	}

	/**
	 * Computes the broadband reflection loss and phase change.
	 *
	 * @param location      Location at which to compute reflection loss.
	 * @param frequencies   Frequencies over which to compute loss. (Hz)
	 * @param angle         Grazing angle relative to the interface (radians).
	 * @param amplitude     Change in ray intensity in dB (output).
	 * @param phase         Change in ray phase in radians (output).
	 * 						Hard-coded to a value of PI for this model.
	 *                      Phase change not computed if this is NULL.
	 */
	virtual void reflect_loss(const wposition1& location,
			const seq_vector& frequencies, double angle,
			vector<double>* amplitude, vector<double>* phase = NULL)
	{
		noalias(*amplitude) = frequencies ;	// copy sequence into vector
		*amplitude = 8.6e-9 * abs2( (*amplitude) * _wind_speed2 * sin(angle) );
		if ( phase ) {
			noalias(*phase) = scalar_vector<double>( frequencies.size(), M_PI );
		}
	}

private:

	/** Wind speed squared (m/sec)^2. */
	const double _wind_speed2;
};

/// @}
}  // end of namespace ocean
}  // end of namespace usml
