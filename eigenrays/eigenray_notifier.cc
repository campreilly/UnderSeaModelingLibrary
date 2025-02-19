/**
 * @file eigenray_notifier.cc
 * Manage eigenray listeners and distribute eigenray updates.
 */

#include <usml/eigenrays/eigenray_notifier.h>

using namespace usml::eigenrays;

/**
 * Add an eigenray listener to this object.
 */
void eigenray_notifier::add_eigenray_listener(eigenray_listener* listener) {
    _listeners.insert(listener);
}

/**
 * Remove an eigenray listener to this object.
 */
void eigenray_notifier::remove_eigenray_listener(eigenray_listener* listener) {
    _listeners.erase(listener);
}

/**
 * Distribute an eigenray updates to all listeners.
 */
void eigenray_notifier::notify_eigenray_listeners(
    size_t t1, size_t t2, const eigenray_model::csptr& ray,
    size_t runID) const {
    for (eigenray_listener* listener : _listeners) {
        listener->add_eigenray(t1, t2, ray, runID);
    }
}

/**
 * For each eigenray_listener in the eigenray_listeners set,
 * call the check_eigenrays method to deliver all eigenrays after
 * a certain amount of time has passed.
 */
void eigenray_notifier::check_eigenray_listeners(double wave_time,
                                                 size_t runID) const {
    for (eigenray_listener* listener : _listeners) {
        listener->check_eigenrays(wave_time, runID);
    }
}
