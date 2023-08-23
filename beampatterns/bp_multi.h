/**
 * @file bp_multi.h
 * Combines the responses of multiple beampattern models into a single model.
 */
#pragma once

#include <usml/beampatterns/bp_model.h>
#include <usml/types/bvector.h>
#include <usml/types/seq_vector.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/vector.hpp>
#include <list>

namespace usml {
namespace beampatterns {

/// @ingroup beampatterns
/// @{

/// Type of combination to use: product or sum
enum class bp_multi_type {
    product = 0,  ///< product of beam patterns
    sum = 1       ///< sum of beampatterns
};

/**
 * Combines the responses of multiple beampattern models into a single model.
 * \f[
 *      b_{multi} = b_1 * b_2 ...
 * \f]
 * Beam patterns can products (the default) or sums (often used for baffles).
 */
class USML_DECLSPEC bp_multi : public bp_model {
   public:
    /**
     * Takes a list of beam patterns and stores them locally to be used
     * when requesting a beam level.
     *
     * @param bp_list	List of references to beam_pattern_models.
     * @param type      Type of combination to use: product or sum.
     */
    bp_multi(std::list<bp_model::csptr> bp_list,
             bp_multi_type type = bp_multi_type::product)
        : _bp_list(bp_list), _type(type) {}

    void beam_level(const bvector& arrival, const seq_vector::csptr& frequencies,
                    vector<double>* level,
                    const bvector& steering = bvector(1.0, 0.0, 0.0),
                    double sound_speed = 1500.0) const override;

   private:
    /// The list of beam patterns whose responses will be combined.
    const std::list<bp_model::csptr> _bp_list;

    /// Type of combination to use: product or sum
    const bp_multi_type _type;
};

/// @}
}  // namespace beampatterns
}  // namespace usml
