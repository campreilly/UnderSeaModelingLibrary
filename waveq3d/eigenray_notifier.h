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
	void add_eigenray_listener(eigenray_listener* listener);

	/**
	 * Remove an eigenray listener to this object.
	 */
	void remove_eigenray_listener(eigenray_listener* listener);

	/**
	 * Notifies all of the listeners that a wave front collision has been detected for
	 * one of the targets. Targets are specified by a row and column number.
	 *
	 * @param   target_row 	Row identifier for the target involved in this collision.
	 * @param   target_col 	Column identifier for the target involved in this collision.
	 * @param   ray        	Propagation loss information for this collision.
	 * @param 	runID		Identification number of the wavefront that
	 * 						produced this result.
	 * @see		wave_queue.runID()
	 */
	void notify_eigenray_listeners(
			size_t target_row, size_t target_col, eigenray ray, size_t runID );

	/**
	 * Notifies all of the listeners that eigenray processing is complete for
	 * a specific wavefront time step. This can be used to limit the time
	 * window for eigenrays to each specific target.
	 *
	 * @param  wave_time   	Elapsed time for this wavefront step.
	 * @param 	runID		Identification number of the wavefront that
	 * 						produced this result.
	 * @see		wave_queue.runID()
	 */
	void check_eigenray_listeners( long wave_time, size_t runID );

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
	std::set<eigenray_listener*> _listeners;
};

/// @}
}
 // end of namespace waveq3d
}// end of namespace usml

