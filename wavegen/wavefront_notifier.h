/**
 * @file wavefront_notifier.h
 * Manages wavefront listeners and distributes wavefront updates.
 */
#pragma once

#include <usml/usml_config.h>
#include <usml/wavegen/wavefront_listener.h>

#include <set>

namespace usml {
namespace wavegen {

using namespace usml::sensors;

/// @ingroup wavegen
/// @{

/**
 * Manages wavefront listeners and distributes wavefront updates.
 */
class USML_DECLSPEC wavefront_notifier {
   public:
    /**
     * Add a wavefront listener to this object.
     */
    void add_wavefront_listener(wavefront_listener* listener);

    /**
     * Remove a wavefront listener to this object.
     */
    void remove_wavefront_listener(wavefront_listener* listener);

    /**
     * Distribute wavefront updates to all listeners.
     */
    void notify_wavefront_listeners(const sensor_model* sensor,
                                    const eigenray_collection::csptr& eigenrays,
                                    const eigenverb_collection::csptr& eigenverbs);

   private:
    /**
     * List of active wavefront listeners.
     */
    std::set<wavefront_listener*> _listeners;
};

/// @}
}  // end of namespace wavegen
}  // end of namespace usml
