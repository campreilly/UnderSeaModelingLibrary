/**
 * @file reflection_notifier.cc
 * Manage reflection listeners and distribute reflection updates.
 */

#include <usml/waveq3d/reflection_notifier.h>

#include <utility>

using namespace usml::waveq3d;

/**
 * Add an reflection listener to this object.
 */
void reflection_notifier::add_reflection_listener(
    reflection_listener* listener) {
    _listeners.insert(listener);
}

/**
 * Remove an reflection listener from this object.
 */
void reflection_notifier::remove_reflection_listener(
    reflection_listener* listener) {
    _listeners.erase(listener);
}

/**
 * Distribute an reflection updates to all listeners.
 */
void reflection_notifier::notify_reflection_listeners(
    double time, size_t de, size_t az, double dt, double grazing, double speed,
    const wposition1& position, const wvector1& ndirection, size_t type) const {
    for (reflection_listener* listener : _listeners) {
        listener->reflect(time, de, az, dt, grazing, speed, position,
                          ndirection, type);
    }
}
