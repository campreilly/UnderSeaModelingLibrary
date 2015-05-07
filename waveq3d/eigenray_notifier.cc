/**
 * @file eigenray_notifier.cc
 * Manage eigenray listeners and distribute eigenray updates.
 */
#include <usml/waveq3d/eigenray_notifier.h>
#include <boost/foreach.hpp>

using namespace usml::waveq3d;

/**
 * Add an eigenray listener to this object.
 */
void eigenray_notifier::add_eigenray_listener(eigenray_listener* listener) {
	_listeners.insert(listener) ;
}

/**
 * Remove an eigenray listener to this object.
 */
void eigenray_notifier::remove_eigenray_listener(eigenray_listener* listener) {
	_listeners.erase(listener) ;
}

/**
 * Distribute an eigenray updates to all listeners.
 */
void eigenray_notifier::notify_eigenray_listeners( size_t target_row, size_t target_col, eigenray ray ) {
	BOOST_FOREACH( eigenray_listener* listener, _listeners ) {
		 listener->add_eigenray(target_row, target_col, ray) ;
	}
}

/**
 * For each eigenray_listener in the eigenray_listeners set
 * call the check_eigenrays method to deliver all eigenrays after
 * a certain amount of time has passed.
 */
void eigenray_notifier::check_eigenray_listeners(size_t run_id, long wave_time) {
    BOOST_FOREACH( eigenray_listener* listener, _listeners ) {
         listener->check_eigenrays(run_id, wave_time);
    }
}

