/**
 * @file envelope_notifier.cc
 * Manage envelope listeners and distribute envelope updates.
 */
#include <usml/eigenverb/envelope_notifier.h>
#include <boost/foreach.hpp>

using namespace usml::eigenverb;

/**
 * Add an envelope listener to this object.
 */
void envelope_notifier::add_envelope_listener(envelope_listener* listener) {
	_listeners.insert(listener) ;
}

/**
 * Remove an envelope listener to this object.
 */
void envelope_notifier::remove_envelope_listener(envelope_listener* listener) {
	_listeners.erase(listener) ;
}

/**
 * Distribute an envelope updates to all listeners.
 */
void envelope_notifier::notify_envelope_listeners( envelope_collection::reference& envelopes ) {
	BOOST_FOREACH( envelope_listener* listener, _listeners ) {
		listener->update_envelopes(envelopes) ;
	}
}

