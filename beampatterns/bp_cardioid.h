/**
 * @file bp_cardioid.h
 * A frequency independent cardioid beam pattern.
 */
#pragma once

#include <usml/beampatterns/bp_model.h>
#include <usml/types/bvector.h>
#include <usml/types/seq_vector.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/vector.hpp>

namespace usml {
namespace beampatterns {

/// @ingroup beampatterns
/// @{

/**
 * Models a frequency independent cardioid beam pattern.
 * The steering is fixed relative to the reference axis of the sensor,
 * but the reference axis can be changed by rotating the orientation of
 * the sensor.
 * \f[
 *      B(\vec{u}) = \left[ \frac{1+N\vec{u} \cdot \vec{u}_{ref} }{1+N}
 * \right]^2 \f] where
 * - \f$ \vec{u} \f$   = wavefront direction (out from array),
 * - \f$ \vec{u}_{ref} \f$ = array reference axis (forward), and
 * - N = the cardioid factor.
 *
 * Cardioid beampatterns can be formed from adding the voltage outputs of
 * a dipole sensor, to those of an omni-directional sensor. Different scaling
 * factors yield different cardioid patterns.
 *
 * - The cardioid factor is 1 for the true cardioid beam pattern function.
 *   This pattern has a null at 180, is 6 dB down at 90, has a 3 dB down
 *   beam width of 131 and a DI of 4.8 dB, the same DI as the dipole.
 * - The cardioid factor is \f$\sqrt3 \approx 1.7\f$ for the super-cardioid beam
 *   pattern function. This case has a front-to-back ratio of 11.7 dB, is
 *   8.6 dB down at 90, and it has a beam width of 115 and a DI of 5.7 dB.
 * - The cardioid factor is 3 for the hyper-cardioid beam pattern function.
 *   It has the highest DI possible with one omni and one dipole sensor
 *   yielding a DI of 6.0 dB, is 12 dB down at 90, has a beam width of
 *   only 105 but a front-to-back ratio of only 6 dB.
 *
 * The directivity gain of the cardioid beam pattern has an analytic form.
 * \f[
 *      DG = \frac{1}{4\pi} \int_{-\pi/2}^{\pi/2} \int_0^{2 \pi}
 *      \left[\frac{1+N\cos\theta\cos\phi}{1+N}\right]^2 \cos\theta d\theta
 * d\phi = \frac{3+N^2}{3(1+N)^2} \f]
 *
 *
 * @xref John L. Butler, Charles H. Sherman, Transducers and Arrays for
 * Underwater Sound, Second Edition 2016, Chapter 6.5.6 Summed Scalar and
 * Vector Sensors.
 */
class USML_DECLSPEC bp_cardioid : public bp_model {
   public:
    /**
     * Constructs cardioid beam pattern with adjustable null.
     *
     * @param factor    Cardioid factor (1 for true cardioid, sqrt(3) for
     *                  super-cardioid, 3 for hyper-cardioid).
     */
    bp_cardioid(double factor = 1.0)
        : _factor(factor),
          _directivity((3.0 + factor * factor) /
                       (3.0 * (1.0 + factor) * (1.0 + factor))) {}

    void beam_level(const bvector& arrival,
                    const seq_vector::csptr& frequencies, vector<double>* level,
                    const bvector& steering = bvector(1.0, 0.0, 0.0),
                    double sound_speed = 1500.0) const override;

    void directivity(const seq_vector::csptr& frequencies,
                     vector<double>* level,
                     const bvector& steering = bvector(1.0, 0.0, 0.0),
                     double sound_speed = 1500.0) const override;

   private:
    /// Cardioid factor (1 for true cardioid, sqrt(3) for super-cardioid,
    /// 3 for hyper-cardioid).
    const double _factor;

    /// Array gain against noise.
    const double _directivity;
};

/// @}
}  // namespace beampatterns
}  // namespace usml
