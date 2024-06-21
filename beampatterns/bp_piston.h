/**
 * @file bp_piston.h
 * Piston hydrophone model oriented along the front/forward axis
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
 * Piston hydrophone model oriented along the front/forward axis.
 * \f[
 *      P = [\frac{2*besselj(1,pi*D/\lambda*sin(\phi))}
 *          {pi*D/\lambda*sin(\phi)}]^2
 * \f]
 * The directivity gain for the piston array has an analytic form.
 * \f[
 *      DG =\left[ \frac{\lambda}{\pi D} \right]^2
 * \f]
 * where D is the diameter of the piston. This beam pattern can not be steered.
 */
class USML_DECLSPEC bp_piston : public bp_model {
   public:
    /**
     * Constructs a piston beam pattern.
     *
     * @param diameter      Piston diameter (meters).
     * @param back_baffle   Set gain to zero in backplane when true.
     */
    bp_piston(double diameter, bool back_baffle = false)
        : _diameter(diameter), _back_baffle(back_baffle) {}

    void beam_level(const bvector& arrival,
                    const seq_vector::csptr& frequencies, vector<double>* level,
                    const bvector& steering = bvector(1.0, 0.0, 0.0),
                    double sound_speed = 1500.0) const override;

    void directivity(const seq_vector::csptr& frequencies,
                     vector<double>* level,
                     const bvector& steering = bvector(1.0, 0.0, 0.0),
                     double sound_speed = 1500.0) const override;

   private:
    /// piston diameter in meters
    double _diameter;

    /// Set gain to zero in backplane when true.
    const bool _back_baffle;
};

/// @}
}  // namespace beampatterns
}  // namespace usml
