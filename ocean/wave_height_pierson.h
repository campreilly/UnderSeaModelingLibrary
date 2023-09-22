/**
 * @file wave_height_pierson.h
 * Pierson and Moskowitz model for computing wave height from wind speed.
 */
#pragma once

#include <usml/ublas/ublas.h>

namespace usml {
namespace ocean {

/// @ingroup boundaries
/// @{

/**
 * Pierson and Moskowitz model for computing wave height from wind speed.
 * This is an empirical model based on measurements of waves measured
 * on British weather ships in the north Atlantic.  It assumes that the waves
 * have come into equilibrium with the wind over a large area.
 * This is the concept is called a fully developed sea.
 *
 * \f[
 *       H_{1/3} \approx 0.22 \frac{ U_{10}^2 }{g}
 * \f]
 * \f[
 *       H_{rms} = \frac{1}{4} H_{1/3} \approx 0.0056 \: U_{10}^2
 * \f]
 * where
 * \f$ U_{10} \f$ = average wind speed measured at 10 meters above water (m/s),
 * \f$ g \approx 9.8 \: m/s^2 \f$ = acceleration due to gravity,
 * \f$ H_{1/3} \f$ = significant wave height (m), and
 * \f$ H_{rms} \f$ = root mean squared of wave height (m).
 *
 * @xref W. J. Pierson, L. Moskowitz, A proposed spectral form for fully
 * developed wind seas based on the similarity theory of S.A. Kitaigordskii,
 * Journal of Geophysical Research 69, pp. 5181–5190, 1964.
 *
 * @xref R. H. Stewart, "Introduction to Physical Oceanography," Open source
 * text book, https://open.umn.edu/opentextbooks/textbooks/20, 2008.
 *
 * @xref Ocean-Wave Spectra entry on WikiWaves site,
 * http://www.wikiwaves.org/Ocean-Wave_Spectra .
 *
 * @param  wind_speed   Average wind speed at height of 10 m above surface
 * (m/s).
 * @return                 RMS wave height (m).
 */
template <class T>
T wave_height_pierson(T wind_speed) {
    return (0.22 / 9.8 / 4) * wind_speed * wind_speed;
}

/// @}
}  // end of namespace ocean
}  // end of namespace usml
