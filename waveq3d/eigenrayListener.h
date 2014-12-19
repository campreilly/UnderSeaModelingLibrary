/*
 * @file eigenrayListener.h
 *
 *  Created on: Sep 9, 2013
 *      Author: Ted Burns (EGB) AEgis Technologies Group, Inc.
 */

#pragma once

#include <usml/waveq3d/eigenray.h>

namespace usml {
namespace waveq3d {

/**
 * @class eigenrayListener
 * @brief This class is part of a Observer/Subject pattern for the wave_queue class
 * and allows for multiple eigenray listeners to be added to wave_queue.
 * The addEigenray call must be defined in each class which inherits it.
 */

class USML_DECLSPEC eigenrayListener
{
public:

	/**
	 * Destructor.
	 */
	virtual ~eigenrayListener() {}

	/**
	 * addEigenray
	 * Pure virtual method to add eigenray to an object.
	 *  @param   targetRow Index of the target row to add to list of eigenrays
     *  @param   targetCol Index of the target row to add to list of eigenrays
     *  @param   pRay      Pointer to eigenray data to add to list of eigenrays
     *  @param   run_id    Run Identification number.
	 */
	virtual bool addEigenray(size_t targetRow, size_t targetCol, eigenray pRay, size_t run_id) = 0;


protected:

	/**
	 * Constructor - protected
	 */
	eigenrayListener() {}



private:

	// -------------------------
	// Disabling default copy constructor and default
	// assignment operator.
	// -------------------------
	eigenrayListener(const eigenrayListener& yRef);
	eigenrayListener& operator=(const eigenrayListener& yRef);

};

/// @}
} // end of namespace waveq3d
} // end of namespace usml

