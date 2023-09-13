/**
 * @file reflect_loss_rayleigh_grid.h
 * Builds rayleigh models for an imported data_grid bottom types.
 */
#pragma once

#include <stddef.h>
#include <usml/ocean/reflect_loss_model.h>
#include <usml/ocean/reflect_loss_rayleigh.h>
#include <usml/types/data_grid.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition1.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/vector.hpp>
#include <vector>

namespace usml {
namespace ocean {

using boost::numeric::ublas::vector;

/// @ingroup boundaries
/// @{

/**
 * The reflect_loss_rayleigh_grid object ingests a data_grid of Rayleigh
 * bottom type data and creates a reflect_loss_rayleigh object to
 * compute reflection loss using the type number at each location.
 */
class USML_DECLSPEC reflect_loss_rayleigh_grid : public reflect_loss_model {
   public:
    /**
     * Creates a reflection_loss model lookup table.
     * Information stored in "data" member is set to a double with the value
     * from 0 to 8 representing different Rayleigh bottom types.
     *
     * @param type_grid data_grid of bottom_types for locations
     */
    reflect_loss_rayleigh_grid(const data_grid<2>::sptr& type_grid);

    /**
     * Gets a Rayleigh bottom type value at a specific location then
     * computes the broadband reflection loss and phase change.
     *
     * @param location      Location at which to compute attenuation.
     * @param frequencies   Frequencies over which to compute loss. (Hz)
     * @param angle         Reflection angle relative to the normal (radians).
     * @param amplitude     Change in ray strength in dB (output).
     * @param phase         Change in ray phase in radians (output).
     *                      Phase change not computed if this is nullptr.
     */
    void reflect_loss(const wposition1& location,
                      const seq_vector::csptr& frequencies, double angle,
                      vector<double>* amplitude,
                      vector<double>* phase = nullptr) const override;

   private:
    /**
     * Stored Rayleigh models for bottom reflections
     */
    std::vector<reflect_loss_rayleigh::csptr> _rayleigh;

    /**
     * Data grid that stores all of the bottom province information.
     */
    data_grid<2>::csptr _bottom_grid;
};

/// @}
}  // end of namespace ocean
}  // end of namespace usml
