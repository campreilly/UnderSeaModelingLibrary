/**
 * @file bp_gaussian.h
 * Solid angle (aka cookie-cutter) beam pattern with a Gaussian taper.
 */
#pragma once

#include <usml/beampatterns/bp_model.h>
#include <usml/beampatterns/bp_solid.h>
#include <usml/types/bvector.h>
#include <usml/types/seq_vector.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/vector.hpp>

namespace usml {
namespace beampatterns {

/// @ingroup beampatterns
/// @{

/**
 * Models a solid angle (aka cookie-cutter) beam pattern with a Gaussian taper.
 * The beam pattern is 1.0 at the peak and it tapers down at the angles defined
 * by its vertical and horizontal beam width. Inherits the directivity gain
 * (DG) calculation from bp_solid.
 */
class USML_DECLSPEC bp_gaussian : public bp_solid {
   public:
    /**
     * Constructs a solid angle beam pattern.
     *
     * @param vert_width    Vertical beam half width (deg).
     * @param horz_width    Horizontal beam half width (deg).
     */
    bp_gaussian(double vert_width, double horz_width = 360.0)
        : bp_solid(vert_width, horz_width) {}

    void beam_level(const bvector& arrival,
                    const seq_vector::csptr& frequencies, vector<double>* level,
                    const bvector& steering = bvector(1.0, 0.0, 0.0),
                    double sound_speed = 1500.0) const override;
};

/// @}
}  // namespace beampatterns
}  // namespace usml
