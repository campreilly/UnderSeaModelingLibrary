/**
 * @file wavefront_notifier.cc
 * Manage wavefront listeners and distribute wavefront updates.
 */

#include <usml/wavegen/wavefront_listener.h>
#include <usml/wavegen/wavefront_notifier.h>

using namespace usml::wavegen;

/**
 * Add an wavefront listener to this object.
 */
void wavefront_notifier::add_wavefront_listener(wavefront_listener* listener) {
    _listeners.insert(listener);
}

/**
 * Remove a wavefront listener from this object.
 */
void wavefront_notifier::remove_wavefront_listener(
    wavefront_listener* listener) {
    _listeners.erase(listener);
}

/**
 * Distribute wavefront updates to all listeners.
 */
void wavefront_notifier::notify_wavefront_listeners(
    const sensor_model* sensor, const eigenray_collection::csptr& eigenrays,
    const eigenverb_collection::csptr& eigenverbs) {
    for (wavefront_listener* listener : _listeners) {
        listener->update_wavefront_data(sensor, eigenrays, eigenverbs);
    }
}
