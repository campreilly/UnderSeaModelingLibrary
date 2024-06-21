/**
 * @file bp_solid.h
 * Solid angle (aka cookie-cutter) beam pattern.
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
 * Models a solid angle (aka cookie-cutter) beam pattern.  The beam pattern
 * has a gain of 1.0 inside the cone defined by its vertical and horizontal
 * beam width. It is zero everywhere else.  The directivity gain (DG) is
 * defined as the ratio of the solid angles covered by vertical and horizontal
 * beam width divided by the total number of available solid angles.
 * Directivity gain is diminished if the edge of the vertical beam pattern
 * exceeds the range [-90,90].
 */
class USML_DECLSPEC bp_solid : public bp_model {
   public:
    /**
     * Constructs a solid angle beam pattern.
     *
     * @param vert_width    Vertical beam half width (deg).
     * @param horz_width    Horizontal beam half width (deg).
     */
    bp_solid(double vert_width, double horz_width = 360.0)
        : _vert_half(0.5 * vert_width), _horz_half(0.5 * horz_width) {}

    void beam_level(const bvector& arrival,
                    const seq_vector::csptr& frequencies, vector<double>* level,
                    const bvector& steering = bvector(1.0, 0.0, 0.0),
                    double sound_speed = 1500.0) const override;

    void directivity(const seq_vector::csptr& frequencies,
                     vector<double>* level,
                     const bvector& steering = bvector(1.0, 0.0, 0.0),
                     double sound_speed = 1500.0) const override;

   protected:
    const double _vert_half;  ///< Vertical beam half width (deg).
    const double _horz_half;  ///< Horizontal beam half width (deg).
};

/// @}
}  // namespace beampatterns
}  // namespace usml
