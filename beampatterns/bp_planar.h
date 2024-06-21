/**
 * @file bp_planar.h
 * Models the closed form solution of a planar array.
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
 * Models the closed form solution of a planar array as the product of
 * a horizontal and vertical line array. See bp_line for more details.
 * The result is a planar array whose elements are in the up/right plane.
 * \f[
 *      b_{planar}(e,u,\theta_s,\phi_s) =
 *      b_{vla}(e,\phi_s) b_{vla}(u,\theta_s)
 * \f]
 * Even though one of the arrays is horizontal, we model is we a VLA because
 * this is the form that is steered relative to the normal of the array.
 *
 * The directivity gain for planar array can be estimated using:
 * \f[
 * DG = \frac{3 \lambda^2}{
 * 2\pi^2 \cos(\phi_s) d_x d_y \sqrt{(N^2_x-1)(N^2_y-1)}  }
 * \f]
 * where \f$d_x\f$, \f$d_y\f$ are the spacing between elements,
 * \f$N_x\f$, \f$N_y\f$ are the number of elements, and
 * \f$\phi_s\f$ is the steering angle relative to the array normal.
 *
 * When the beam pattern is computed at its design frequency
 * (where \f$\alpha=1\f$), the DI is inversely proportional to the
 * number of elements at all steerings.  But, the full equation includes
 * frequency dependent effects, including increases in DI near end-fire.
 *
 * @xref Albert H. Nuttall, Benjamin A. Cray, Approximations to
 * Directivity for Linear, Planar, and Volumetric Apertures and Arrays,
 * NUWC-NPT Technical Report 10,798, 25 July 1997, equation 41.
 *
 */
class USML_DECLSPEC bp_planar : public bp_model {
   public:
    /**
     * Constructs a vertical or horizontal beam pattern.
     *
     * @param num_elem_up    Number of elements in the up/down direction.
     * @param spacing_up     Element spacing in the up/down direction (m).
     * @param num_elem_right Number of elements in the right/left direction.
     * @param spacing_right  Element spacing in the right/left direction (m).
     * @param back_baffle    Set gain to zero in backplane when true.
     */
    bp_planar(unsigned num_elem_up, double spacing_up, unsigned num_elem_right,
              double spacing_right, bool back_baffle = false);

    void beam_level(const bvector& arrival,
                    const seq_vector::csptr& frequencies, vector<double>* level,
                    const bvector& steering = bvector(1.0, 0.0, 0.0),
                    double sound_speed = 1500.0) const override;

    void directivity(const seq_vector::csptr& frequencies,
                     vector<double>* level,
                     const bvector& steering = bvector(1.0, 0.0, 0.0),
                     double sound_speed = 1500.0) const override;

   private:
    /// Number of elements in the up/down direction.
    const double _num_elem_up;

    /// Element spacing in the up/down direction (m).
    const double _spacing_up;

    /// Number of elements in the right/left direction.
    const double _num_elem_right;

    /// Element spacing in the right/left direction (m).
    const double _spacing_right;

    /// Constant parts of directivity gain.
    const double _directivity;

    /// Set gain to zero in backplane when true.
    const bool _back_baffle;
};

/// @}
}  // namespace beampatterns
}  // namespace usml
