/**
 * @file reflection_notifier.h
 * Manages reflection listeners and distributes reflection updates.
 */
#pragma once

#include <usml/types/wposition1.h>
#include <usml/usml_config.h>
#include <usml/waveq3d/reflection_listener.h>

#include <cstddef>
#include <set>

namespace usml {
namespace waveq3d {

using namespace usml::types;

/// @ingroup waveq3d
/// @{

/**
 * Manages reflection listeners and distributes reflection updates.
 */
class USML_DECLSPEC reflection_notifier {
   public:
    /**
     * Add an reflection listener to this object.
     */
    void add_reflection_listener(reflection_listener* listener);

    /**
     * Remove an reflection listener from this object.
     */
    void remove_reflection_listener(reflection_listener* listener);

    /**
     * Distribute an reflection update to all listeners.
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
    void notify_reflection_listeners(double time, size_t de, size_t az,
                                     double dt, double grazing, double speed,
                                     const wposition1& position,
                                     const wvector1& ndirection,
                                     size_t type) const;

    /**
     * Determines if any listeners exist
     * @return true when listeners exist, false otherwise.
     */
    bool has_reflection_listeners() const { return _listeners.size() > 0; }

   private:
    /**
     * List of active reflection listeners.
     */
    std::set<reflection_listener*> _listeners;
};

/// @}
}  // end of namespace waveq3d
}  // end of namespace usml
