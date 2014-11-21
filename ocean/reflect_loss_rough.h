/**
 * @file reflect_loss_rough.h
 * Models plane wave reflection loss from a rough ocean surface.
 */
#pragma once

#include <usml/ocean/reflect_loss_model.h>

namespace usml {
namespace ocean {

using boost::numeric::ublas::vector;

/// @ingroup boundaries
/// @{

/**
 * Models ocean surface reflection loss from a rough surface.
 * \f[
 *   	R = exp[ - k H sin( \theta ) ]
 * \f]
 * where
 * 		\f$ k = \frac{2 \pi f}{c} \f$ = wave number (1/m),
 * 		\f$ H \f$ = RMS height of wave spectrum (m),
 * 		\f$ \theta \f$ = grazing angle (rad), and
 * 		\f$ R \f$ = reflection coefficient (ratio).
 *
 * Note that the significant wave height (SWH or Hs) is defined
 * as four times the rms height of wave spectrum.
 *
 * @xref Urick R. J., "Principles of Underwater Sound, Third Edition, 1983", pp. 129.
 */
class USML_DECLSPEC reflect_loss_rough: public reflect_loss_model {

public:

	/**
	 * Builds a rough ocean surface assuming full developed, wind driven seas.
	 * Uses the Pierson and Moskowitz model for computing wave height
	 * from wind speed.
	 *
	 * @param wind_speed	Wind_speed used to develop rough seas (m/s)
	 * @param sound_speed	Speed of sound in water used for wave number (m/s).
	 * 						Defaults to 1500 m/s when not specified.
	 */
	reflect_loss_rough(double wind_speed, double sound_speed = 1500.0) :
		_wave_height( wave_height_pierson(wind_speed) ),
		_sound_speed( sound_speed )
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
		*amplitude = exp( -TWO_PI / _sound_speed  * frequency * _wave_height
				   * abs(sin(angle)) ) ;
		if ( phase ) {
			noalias(*phase) = scalar_vector<double>(frequency.size(),M_PI);
		}
	}

private:

	/** RMS height of wave spectrum (m). */
	const double _wave_height;

	/** Speed of sound in water (m/s). */
	const double _sound_speed;
};

/// @}
}// end of namespace ocean
}  // end of namespace usml
