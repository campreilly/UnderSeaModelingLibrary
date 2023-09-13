/**
 * @file reflect_loss_beckmann.h
 * Models ocean surface reflection loss using Beckmann-Spizzichino model.
 */
#pragma once

#include <usml/ocean/reflect_loss_model.h>
#include <usml/ocean/wave_height_pierson.h>

namespace usml {
namespace ocean {

/// @ingroup boundaries
/// @{

/**
 * Models ocean surface reflection loss using Beckmann-Spizzichino model.
 * Jones et. al. has shown that this model can be broken into high and low
 * frequency components. The high frequency component is given by:
 * \f[
 *       RL_{high} = -20 \: log_{10} \left( \sqrt{1-v_3} \right)
 * \f]\f[
 *       v_3 = max \left( \frac{1}{2} sin \theta, \left[ 1 -
 *       \frac{ exp(-a \theta^2 / 4 ) }{ \sqrt{ \pi a \theta^2 } } \right] sin
 * \theta \right) \f] where \f$ a = \frac{1}{ 2 ( 0.003 + 5.1x10^{-3} w ) } \f$,
 *         \f$ w \f$ = wind speed (m/sec), and
 *         \f$ v_3 \f$ is limited to a 0.99 value.
 * Note that the high frequency component is frequency independent.
 * The low frequency component is given by:
 * \f[
 *       RL_{1ow} = -20 \: log_{10}
 *           \left( 0.3 + \frac{0.7}{1+6.0x10^{-11} w^4 f^2 } \right)
 * \f]
 * where
 *         \f$ f \f$ = signal frequency (Hz).
 * Note that the low frequency component is grazing angle independent.
 * The total reflection loss is the sum of these two terms in dB.
 *
 * @xref Adrian D. Jones, Janice Sendt, Alec J. Duncan, Paul A. Clarke and
 * Amos Maggi, "Modelling the acoustic reflection loss at the rough
 * ocean surface," Proceedings of ACOUSTICS 2009, Australian Acoustical Society,
 * 23-25 November 2009, Adelaide, Australia.
 */
class USML_DECLSPEC reflect_loss_beckmann : public reflect_loss_model {
   public:
    /**
     * Initializes ocean surface reflection loss using using
     * Beckmann-Spizzichino model.
     *
     * @param wind_speed    Wind_speed used to develop rough seas (m/s).
     */
    reflect_loss_beckmann(double wind_speed) : _wind_speed(wind_speed) {}

    /**
     * Computes the broadband reflection loss and phase change.
     *
     * @param location      Location at which to compute reflection loss.
     * @param frequencies   Frequencies over which to compute loss. (Hz).
     * @param angle         Grazing angle relative to the interface (radians).
     * @param amplitude     Change in ray intensity in dB (output).
     * @param phase         Change in ray phase in radians (output).
     *                         Hard-coded to a value of PI for this model.
     *                      Phase change not computed if this is nullptr.
     */
    void reflect_loss(const wposition1& location,
                      const seq_vector::csptr& frequencies, double angle,
                      vector<double>* amplitude,
                      vector<double>* phase = nullptr) const override;

   private:
    /** Wind speed (m/sec). */
    const double _wind_speed;
};

/// @}
}  // end of namespace ocean
}  // end of namespace usml
