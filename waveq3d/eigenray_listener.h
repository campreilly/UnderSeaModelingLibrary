/*
 * @file eigenray_listener.h
 *
 *  Created on: Sep 9, 2013
 *      Author: Ted Burns (EGB) AEgis Technologies Group, Inc.
 */

#pragma once

#include <usml/waveq3d/eigenray.h>

namespace usml {
namespace waveq3d {

/// @ingroup waveq3d
/// @{

/**
 * @class eigenray_listener
 * @brief This class is part of a Observer/Subject pattern for the wave_queue class
 * and allows for multiple eigenray listeners to be added to wave_queue.
 * The add_eigenray call must be defined in each class which inherits it.
 */

class USML_DECLSPEC eigenray_listener
{
public:

	/**
	 * Destructor.
	 */
	virtual ~eigenray_listener() {}

	/**
	 * Pure virtual method to add eigenray to an object.
	 *  @param   targetRow Index of the target row to add to list of eigenrays
     *  @param   targetCol Index of the target row to add to list of eigenrays
     *  @param   pRay      Pointer to eigenray data to add to list of eigenrays
     *  @param   run_id    Run Identification number.
	 */
	virtual bool add_eigenray(size_t targetRow, size_t targetCol, eigenray pRay, size_t run_id) = 0;
	
	/**
	 * Virtual method to check if all eigenrays are available within a time frame.
	 *  @param 	runID Run number of waveQ3D
	 *  @param  waveTime Current Time of the wavefront used to check elapsed time.
	 *  @return  		   True on Success, false otherwise.
	 */
	virtual bool check_eigenrays(size_t runID, long waveTime)
	{
		return false;
	}


protected:

	/**
	 * Constructor - protected
	 */
	eigenray_listener() {}



private:

	// -------------------------
	// Disabling default copy constructor and default
	// assignment operator.
	// -------------------------
	eigenray_listener(const eigenray_listener& yRef);
	eigenray_listener& operator=(const eigenray_listener& yRef);

};

/// @}
} // end of namespace waveq3d
} // end of namespace usml

