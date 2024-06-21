/**
 * @file bp_omni.h
 * Omni-directional beam pattern.
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
 * Models an omni-directional beam pattern.
 */
class USML_DECLSPEC bp_omni : public bp_model {
   public:
    void beam_level(const bvector& arrival,
                    const seq_vector::csptr& frequencies, vector<double>* level,
                    const bvector& steering = bvector(1.0, 0.0, 0.0),
                    double sound_speed = 1500.0) const override;

    void directivity(const seq_vector::csptr& frequencies,
                     vector<double>* level,
                     const bvector& steering = bvector(1.0, 0.0, 0.0),
                     double sound_speed = 1500.0) const override;
};

/// @}
}  // namespace beampatterns
}  // namespace usml
