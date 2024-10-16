/**
 * @file eigenray_notifier.h
 * Manages eigenray listeners and distributes eigenray updates.
 */
#pragma once

#include <usml/eigenrays/eigenray_listener.h>
#include <usml/eigenrays/eigenray_model.h>
#include <usml/usml_config.h>

#include <cstddef>
#include <set>

namespace usml {
namespace eigenrays {

/// @ingroup eigenrays
/// @{

/**
 * Manages eigenray listeners and distributes eigenray updates.
 */
class USML_DECLSPEC eigenray_notifier {
   public:
    /**
     * Add an eigenray listener to this object.
     */
    void add_eigenray_listener(eigenray_listener* listener);

    /**
     * Remove an eigenray listener to this object.
     */
    void remove_eigenray_listener(eigenray_listener* listener);

    /**
     * Notifies all of the listeners that a wave front collision has been
     * detected for one of the targets.
     *
     * @param t1     	Row number of target.
     * @param t2     	Column number of target.
     * @param ray    	Propagation loss information for this collision.
     * @param runID 	Wavefront identification number.
     * @see wave_queue.runID()
     */
    void notify_eigenray_listeners(size_t t1, size_t t2,
                                   const eigenray_model::csptr& ray,
                                   size_t runID) const;

    /**
     * Notifies all of the listeners that eigenray processing is complete for
     * a specific wavefront time step. This can be used to limit the time
     * window for eigenrays to each specific target.
     *
     * @param  wave_time    Elapsed time for this wavefront step.
     * @param runID 		Wavefront identification number.
     * @see wave_queue.runID()
     */
    void check_eigenray_listeners(double wave_time, size_t runID) const;

    /**
     * Determines if any listeners exist
     * @return true when listeners exist, false otherwise.
     */
    inline bool has_eigenray_listeners() const { return _listeners.size() > 0; }

   private:
    /**
     * List of active eigenray listeners.
     */
    std::set<eigenray_listener*> _listeners;
};

/// @}
}  // namespace eigenrays
}  // namespace usml
