/**
 * @file reflection_listener.h
 * Abstract interface for alerting listeners to interface reflections.
 */
#pragma once

#include <usml/types/wposition1.h>
#include <usml/usml_config.h>

#include <cstddef>

namespace usml {
namespace waveq3d {

using namespace usml::types;

/// @ingroup waveq3d
/// @{

/**
 * Abstract interface for alerting listeners to interface reflections.
 */
class USML_DECLSPEC reflection_listener {
   public:
    /**
     * Virtual destructor.
     */
    virtual ~reflection_listener() {}

    /**
     * Pure virtual method to process reflection notifications
     *
     * @param time          Time of collision.
     * @param de            D/E angle index number.
     * @param az            AZ angle index number.
     * @param dt            Offset in time to collision with the boundary
     * @param grazing       The grazing angle at point of impact (rads)
     * @param speed         Speed of sound at the point of collision.
     * @param position      Location at which the collision occurs
     * @param ndirection    Normalized direction at the point of collision.
     * @param type          Interface number for the interface that generated
     *                      for this eigenverb.  See the eigenverb_collection
     *                      class header for documentation on interpreting
     *                      this number. For some layers, you can also use
     */
    virtual void reflect(double time, size_t de, size_t az, double dt,
                         double grazing, double speed,
                         const wposition1& position, const wvector1& ndirection,
                         size_t type) = 0;
};

/// @}
}  // end of namespace waveq3d
}  // end of namespace usml
