/**
 * @file reflect_loss_model.h
 * Generic interface for reflection loss models.
 */
#pragma once

#include <usml/types/types.h>
#include <usml/ublas/ublas.h>

namespace usml {
namespace ocean {

using namespace usml::ublas;
using namespace usml::types;

using boost::numeric::ublas::vector;

/// @ingroup boundaries
/// @{

/**
 * A reflection loss model computes the changes in intensity and phase
 * that result from the reflection of a ray from a boundary.
 * Reflection is calculated on a ray-by-ray basis instead of across the
 * wavefront because only selected rays interact with the boundary at any
 * given time step. These models compute their results as a function
 * of frequency to support broadband acoustics.
 */
class USML_DECLSPEC reflect_loss_model {
   public:
    /// Shared pointer to constant version of this class.
    typedef std::shared_ptr<const reflect_loss_model> csptr;

    /**
     * Computes the broadband reflection loss and phase change for a
     * single location.
     *
     * @param location      Location at which to compute reflection loss.
     * @param frequencies   Frequencies over which to compute loss. (Hz)
     * @param angle         Grazing angle relative to the interface (radians).
     * @param amplitude     Change in ray intensity in dB (output).
     * @param phase         Change in ray phase in radians (output).
     *                      Phase change not computed if this is nullptr.
     */
    virtual void reflect_loss(const wposition1& location,
                              seq_vector::csptr frequencies, double angle,
                              vector<double>* amplitude,
                              vector<double>* phase = nullptr) const = 0;

    /**
     * Virtual destructor
     */
    virtual ~reflect_loss_model() {}
};

/// @}
}  // end of namespace ocean
}  // end of namespace usml
