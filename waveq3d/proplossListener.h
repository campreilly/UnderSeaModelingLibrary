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
 * and allows the multiple proploss listener to be added to wave_queue.
 */

class proplossListener
{
public:

	/**
	 * Destructor.
	 */
	virtual ~proplossListener() {}

	/**
	 * AddEigenray
	 * Pure virtual method to add eigenray to an object.
	 */
	virtual bool addEigenray(unsigned targetRow, unsigned targetCol, eigenray pclRay ) = 0;


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

