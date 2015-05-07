/*
 * @file eigenverb_notifier.h
 * Manage eigenverb listeners and distribute eigenverb updates.
 */
#pragma once

#include <usml/eigenverb/eigenverb_listener.h>
#include <set>

namespace usml {
namespace eigenverb {

/// @ingroup eigenverb
/// @{

/**
 * Manage eigenverb listeners and distribute eigenverb updates.
 */
class USML_DECLSPEC eigenverb_notifier {
public:

    /**
     * Add an eigenverb listener to this object.
     */
    void add_eigenverb_listener(eigenverb_listener* listener) ;

    /**
     * Remove an eigenverb listener to this object.
     */
    void remove_eigenverb_listener(eigenverb_listener* listener) ;

    /**
     * Distribute an eigenverb updates to all listeners.
     */
    void notify_eigenverb_listeners( const eigenverb& verb, size_t interface) ;

    /**
     * Determines if any listeners exist
     * @return true when listeners exist, false otherwise.
     */
    inline bool has_eigenverb_listeners() {
        if (_listeners.size() > 0) {
            return true;
        }
        return false;
    }

private:

    /**
     * List of active eigenverb listeners.
     */
    std::set<eigenverb_listener*> _listeners ;
};

/// @}
} // end of namespace eigenverb
} // end of namespace usml

