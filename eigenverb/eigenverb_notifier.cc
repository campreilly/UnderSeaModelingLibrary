/**
 * @file eigenverb_notifier.cc
 * Manage eigenverb listeners and distribute eigenverb updates.
 */
#include <usml/eigenverb/eigenverb_notifier.h>
#include <boost/foreach.hpp>

using namespace usml::eigenverb;

/**
 * Add an eigenverb listener to this object.
 */
void eigenverb_notifier::add_eigenverb_listener(eigenverb_listener* listener) {
	_listeners.insert(listener) ;
}

/**
 * Remove an eigenverb listener to this object.
 */
void eigenverb_notifier::remove_eigenverb_listener(eigenverb_listener* listener) {
	_listeners.erase(listener) ;
}

/**
 * Distribute an eigenverb updates to all listeners.
 */
void eigenverb_notifier::notify_eigenverb_listeners( const eigenverb& verb, size_t interface_num ) {
	BOOST_FOREACH( eigenverb_listener* listener, _listeners ) {
		listener->add_eigenverb(verb, interface_num) ;
	}
}

