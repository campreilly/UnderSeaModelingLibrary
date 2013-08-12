/**
 * @file boundary_flat_mt.h
 * Multi-threaded flat boundary model for the ocean including the ocean surface.
 */

//#pragma once
#ifndef USML_OCEAN_BOUNDARY_FLAT_MT_H
#define USML_OCEAN_BOUNDARY_FLAT_MT_H

#include <pthreadcc.h>
#include <usml/types/lockingvalue.h>
#include <usml/ocean/boundary_flat.h>

namespace usml
{
namespace ocean
{

/// @ingroup boundaries
/// @{

/**
 * Models any flat boundary in the ocean including the ocean surface.
 */
class USML_DECLSPEC boundary_flat_mt: public ThreadBase, public boundary_flat
{

public:
	/**
	 * Constructor - Initialize depth and reflection loss components for a boundary for Mult-threading
	 *
	 * @param depth         Water depth relative to mean sea level.
	 * @param reflect_loss  Reflection loss model.  Assumes depth=0 is used to
	 *                      define the water surface and any other depths
	 *                      define the ocean bottom. Use perfect surface or
	 *                      bottom reflection if no model specified.
	 *                      The boundary_model takes over ownship of this
	 *                      reference and deletes it as part of its
	 *                      destructor.
	 */
	boundary_flat_mt(double depth = 0.0, reflect_loss_model* reflect_loss = NULL)
		:	boundary_flat(depth, reflect_loss)
	{
		m_pclMutex = new MutualExclusion();

		m_pclTerminateSignal = new LockingValue<bool>(false);

		LaunchThread(1 * 1024 * 1024);
		usleep(1000);
	}

	/**
	 * Destructor
	 */
	virtual ~boundary_flat_mt()
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
	 * Compute the height of the boundary and it's surface normal at
	 * a series of locations.
	 *
	 * @param location      Location at which to compute boundary.
	 * @param rho           Surface height in spherical earth coords (output).
	 * @param normal        Unit normal relative to location (output).
	 * @param quick_interp  Determines if you want a fast nearest or pchip interp
	 */
	virtual void height(const wposition& location, matrix<double>* rho,
			wvector* normal = NULL, bool quick_interp = false)
	{
		m_pclMutex->Lock();
		boundary_flat::height(location, rho, normal, quick_interp);
		m_pclMutex->Unlock();

	}

	/**
	 * Compute the height of the boundary and it's surface normal at
	 * a single location.  Often used during reflection processing.
	 *
	 * @param location      Location at which to compute boundary.
	 * @param rho           Surface height in spherical earth coords (output).
	 * @param normal        Unit normal relative to location (output).
	 * @param quick_interp  Determines if you want a fast nearest or pchip interp
	 */
	virtual void height(const wposition1& location, double* rho,
			wvector1* normal = NULL, bool quick_interp = false)
	{
		m_pclMutex->Lock();
		boundary_flat::height(location, rho, normal, quick_interp);
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
