/*
 * @file proplossListener.h
 *
 *  Created on: Sep 9, 2013
 *      Author: safuser
 */

#pragma once

#include <usml/waveq3d/eigenray.h>

namespace usml {
namespace waveq3d {

/**
 * @class proplossListener
 * @brief This class is part of a Observer/Subject pattern for the wave_queue class
 * and allows for multiple proploss listeners to be added to wave_queue.
 * The addEigenray call must be defined in each class which inherits it.
 */

class proplossListener
{
public:

	/**
	 * Destructor.
	 */
	virtual ~proplossListener() {}

	/**
	 * addEigenray
	 * Pure virtual method to add eigenray to an object.
	 *  @param   targetRow Index of the target row to add to list of eigenrays
     *  @param   targetCol Index of the target row to add to list of eigenrays
     *  @param   pRay      Pointer to eigenray data to add to list of eigenrays
	 */
	virtual bool addEigenray(unsigned targetRow, unsigned targetCol, eigenray pRay ) = 0;


protected:

	/**
	 * Constructor - protected
	 */
	proplossListener() {}



private:

	// -------------------------
	// Disabling default copy constructor and default
	// assignment operator.
	// -------------------------
	proplossListener(const proplossListener& yRef);
	proplossListener& operator=(const proplossListener& yRef);

};

/// @}
} // end of namespace waveq3d
} // end of namespace usml

