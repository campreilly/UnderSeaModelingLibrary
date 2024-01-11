/**
 * @file bp_grid.h
 * Constructs a beam pattern from a data grid.
 */
#pragma once

#include <usml/beampatterns/bp_model.h>
#include <usml/types/bvector.h>
#include <usml/types/data_grid.h>
#include <usml/types/seq_vector.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/vector.hpp>

namespace usml {
namespace beampatterns {

using namespace usml::types;

/// @ingroup beampatterns
/// @{

/**
 * Interpolate beam levels from data grid. This beam pattern can not be steered
 * or adjusted for local sound speed.  It is primarily used to implement beam
 * patterns built from measured data.
 *
 * To work around the problem that the data_grid.interpolate() function is
 * not const, this implementation uses a combination of a read_write_lock
 * and a const_cast<> to lock the data grid during interpolation.
 */
class USML_DECLSPEC bp_grid : public bp_model {
   public:
    /**
     * Constructs a beam pattern from a data grid.
     * DE angles range from -90 deg down to +90 deg up.
     * AZ angles range from -180 deg left to +180 deg right.
     *
     * @param data  Data grid for beam pattern. Dimension #0 is frequency (Hz),
     *              #1 is depression/elevation angle (deg), and
     *              #2 is azimuth angle (deg).
     */
    bp_grid(data_grid<3>::csptr data) : _data(data) {}

    virtual void beam_level(const bvector& arrival,
                            const seq_vector::csptr& frequencies,
                            vector<double>* level,
                            const bvector& steering = bvector(1.0, 0.0, 0.0),
                            double sound_speed = 1500.0) const;

   private:
    /**
     * Data grid for beam pattern. Dimension #0 is frequency (Hz),
     * #1 is depression/elevation angle (deg), and #2 is azimuth angle (deg).
     */
    data_grid<3>::csptr _data;
};

/// @}
}  // namespace beampatterns
}  // namespace usml
