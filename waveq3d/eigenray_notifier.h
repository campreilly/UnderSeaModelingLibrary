/*
 * @file eigenray_notifier.h
 * Manage eigenray listeners and distribute eigenray updates.
 */
#pragma once

#include <usml/waveq3d/eigenray_listener.h>
#include <set>

namespace usml {
namespace waveq3d {

/// @ingroup waveq3d
/// @{

/**
 * Manage eigenray listeners and distribute eigenray updates.
 */
class USML_DECLSPEC eigenray_notifier {

public:
    /**
     * Add an eigenray listener to this object.
     */
    void add_eigenray_listener(eigenray_listener* listener) ;

    /**
     * Remove an eigenray listener to this object.
     */
    void remove_eigenray_listener(eigenray_listener* listener) ;

    /**
     * Distribute eigenray updates to all listeners.
     */
    void notify_eigenray_listeners( size_t target_row, size_t target_col, eigenray ray) ;
    /**
     * For each eigenray_listener in the eigenray_listeners set
     * call the check_eigenrays method to deliver all eigenrays after
     * a certain amount of time has passed.
     */
    void check_eigenray_listeners(size_t run_id, long wave_time ) ;

    /**
     * Determines if any listeners exist
     * @return true when listeners exist, false otherwise.
     */
    inline bool has_eigenray_listeners() {
        if (_listeners.size() > 0) {
            return true;
        }
        return false;
    }

private:

    /**
     * List of active eigenray listeners.
     */
    std::set<eigenray_listener*> _listeners ;
};

/// @}
} // end of namespace waveq3d
} // end of namespace usml

