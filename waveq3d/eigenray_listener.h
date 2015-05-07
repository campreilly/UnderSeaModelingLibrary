/**
 * @file eigenray_listener.h
 * Abstract interface for passing newly created eigenrays to an observer.
 */
#pragma once

#include <usml/waveq3d/eigenray.h>

namespace usml {
namespace waveq3d {

/// @ingroup waveq3d
/// @{

/**
 * Abstract interface for passing newly created eigenrays to an observer.
 * Uses an Observer/Subject pattern which allows the receiver to process
 * propagation information as soon as it becomes available. The observer does
 * not need to wait until the propagation model is complete.
 */
class USML_DECLSPEC eigenray_listener {
public:

	/**
	 * Destructor.
	 */
	virtual ~eigenray_listener() {

	}

	/**
	 * Notifies the observer that a wave front collision has been detected for
	 * one of the targets. Targets are specified by a row and column number.
	 * Must be overloaded by sub-classes.
	 *
	 * @param   target_row 	Row identifier for the target involved in this collision.
	 * @param   target_col 	Column identifier for the target involved in this collision.
	 * @param   ray        	Propagation loss information for this collision.
	 * @param 	runID		Identification number of the wavefront that
	 * 						produced this result.
	 * @see		wave_queue.runID()
	 */
	virtual void add_eigenray(
		size_t target_row, size_t target_col, eigenray ray, size_t runID) = 0;

	/**
	 * Notifies the observer that eigenray processing is complete for
	 * a specific wavefront time step. This can be used to limit the time
	 * window for eigenrays to each specific target.
	 *
	 * @param  wave_time   	Elapsed time for this wavefront step.
	 * @param 	runID		Identification number of the wavefront that
	 * 						produced this result.
	 * @see		wave_queue.runID()
	 */
	virtual void check_eigenrays(long wave_time, size_t runID) {}

protected:

	/**
	 * Constructor - protected
	 */
	eigenray_listener() {}

private:

	/*
	 * Disabling default copy constructor and default
	 * assignment operator.
	  */
	eigenray_listener(const eigenray_listener& yRef);
	eigenray_listener& operator=(const eigenray_listener& yRef);

};

/// @}
}
	// end of namespace waveq3d
}	// end of namespace usml

