/*
 * @file envelope_notifier.h
 * Manage envelope listeners and distribute envelope updates.
 */
#pragma once

#include <usml/eigenverb/envelope_listener.h>
#include <set>

namespace usml {
namespace eigenverb {

/// @ingroup eigenverb
/// @{

/**
 * Manage envelope listeners and distribute envelope updates.
 */
class USML_DECLSPEC envelope_notifier {
public:

    /**
     * Add an envelope listener to this object.
     */
    void add_envelope_listener(envelope_listener* listener) ;

    /**
     * Remove an envelope listener to this object.
     */
    void remove_envelope_listener(envelope_listener* listener) ;

    /**
     * Distribute an envelope updates to all listeners.
     */
    void notify_envelope_listeners( envelope_collection::reference& envelopes ) ;

private:

    /**
     * List of active envelope listeners.
     */
    std::set<envelope_listener*> _listeners ;
};

/// @}
} // end of namespace eigenverb
} // end of namespace usml

