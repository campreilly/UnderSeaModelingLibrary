/**
 * @file profile_grid_mt.h
 * Creates a multi-threaded sound speed model from a 1-D, 2-D, or 3-D data grid.
 */

//#pragma once
#ifndef USML_OCEAN_PROFILE_GRID_MT_H
#define USML_OCEAN_PROFILE_GRID_MT_H

#include <pthreadcc.h>
#include <usml/types/lockingvalue.h>
#include <usml/ocean/profile_grid.h>

namespace usml {
namespace ocean {

/// @ingroup profiles
/// @{

/**
 * Sound speed model constructed from a 1-D, 2-D, or 3-D data grid.
 * The coordinate system for each kind of data set is:
 *
 *      - 1-D: Assumes that the sound speed is only a function
 *             of altitude
 *      - 2-D: Assumes that the order of axes in the grid is
 *             (altitude, latitude)
 *      - 3-D: Assumes that the order of axes in the grid is
 *             (altitude, latitude, longitude)
 *
 *    ---NOTE: altitude is the distance from the surface of the ocean
 *             to the location under the ocean with down as negative.
 *
 *    ^^^NOTE: All calculations are under the assumption that the
 *             grid axes passed in have already been transformed
 *             to their spherical earth equivalents (altitude -> rho,
 *             theta,phi).
 */

template<class DATA_TYPE, int NUM_DIMS> class profile_grid_mt: public ThreadBase,
		public profile_grid<DATA_TYPE, NUM_DIMS>
{

public:

	/**
	 * Constructor - Default behavior for new profile models.
	 *
	 * @param speed         Sound speed for the whole ocean (m/s).
	 *                      Assumes control of this grid and deletes
	 *                      it when the class is destroyed.
	 * @param attmodel      In-water attenuation model.  Defaults to Thorp.
	 *                      The profile_model takes over ownership of this
	 *                      reference and deletes it as part of its destructor.
	 */
	profile_grid_mt(data_grid<DATA_TYPE, NUM_DIMS>* speed, attenuation_model* attmodel = NULL)
		: ThreadBase(), profile_grid<DATA_TYPE, NUM_DIMS>(speed, attmodel)
	{
		m_pclMutex = new MutualExclusion();

		m_pclTerminateSignal = new LockingValue<bool>(false);

		LaunchThread(1 * 1024 * 1024);
		usleep(1000);
	}

	/**
	 * Destructor
	 */
	virtual ~profile_grid_mt()
	{
		m_pclTerminateSignal->SetValue(true);
		WaitThread();

		if (m_pclTerminateSignal)
		{
			delete m_pclTerminateSignal;
			m_pclTerminateSignal = NULL;
		}

		if (m_pclMutex)
		{
			delete m_pclMutex;
			m_pclMutex = NULL;
		}
	}

	/**
	 * Overrides base class.
	 */
	void* Initialise()
	{
		return NULL;
	}

	/**
	 * Overrides base class.
	 */
	void* Execute()
	{
		try
		{
			usleep(10000); // This needs to be here or we will crash for unknown reasons.

			// Loop until we receive a stop message.
			while (m_pclTerminateSignal && !m_pclTerminateSignal->GetValue())
			{
				//Update Gaming Area

				usleep(10000);
			}
		} catch (...)
		{
		}

		return NULL;
	}

	/**
	 * Overrides base class.
	 */
	void CleanUp()
	{
		m_pclTerminateSignal->SetValue(true);
	}

	/**
	 * Compute the speed of sound and it's first derivatives at
	 * a series of locations.
	 *
	 * @param location      Location at which to compute attenuation.
	 * @param speed         Speed of sound (m/s) at each location (output).
	 * @param gradient      Sound speed gradient at each location (output).
	 */
	void sound_speed(const wposition& location, matrix<double>* speed,
			wvector* gradient = NULL)
	{
		m_pclMutex->Lock();
		profile_grid<DATA_TYPE, NUM_DIMS>::sound_speed(location, speed, gradient);
		m_pclMutex->Unlock();
	}

protected:
	/** Mutex to prevent simultaneous access/update by multiple threads. */
	MutualExclusion* m_pclMutex;

private:
	/** Signal to terminate the internal thread. */
	LockingValue<bool>* m_pclTerminateSignal;
};

/// @}
}// end of namespace ocean
}  // end of namespace usml

#endif
