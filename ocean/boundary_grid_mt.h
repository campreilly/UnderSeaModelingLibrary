/**
 * @file boundary_grid_mt.h
 * Creates a mutlti-threaded bottom model from a 1-D or 2-D data grid.
 */

#pragma once

#include <pthreadcc.h>
#include <usml/types/lockingvalue.h>
#include <usml/ocean/boundary_grid.h>
#include <usml/ocean/reflect_loss_rayleigh.h>

namespace usml {
namespace ocean {

/**
 * Bottom model constructed from a 1-D or 2-D data grid.
 * The coordinate system for each kind of data set is:
 *
 *      - 1-D: Assumes that the bottom depth is a function of latitude and
 *             that the geodetic axes have been transformed to
 *             their spherical earth equivalents (theta).
 *      - 2-D: Assumes that the order of axes in the grid is
 *             (latitude, longitude) and that the geodetic
 *             axes have been transformed to their spherical earth
 *             equivalents (theta,phi).
 *
 * Uses the GRID_INTERP_PCHIP interpolation in both directions
 * to reduce sudden changes in surface normal direction.  Values outside of the
 * latitude/longitude axes defined by the data grid at limited to the values
 * at the grid edge.
 */
template<class DATA_TYPE, int NUM_DIMS> class boundary_grid_mt: public ThreadBase,
		public boundary_grid<DATA_TYPE, NUM_DIMS>
{
public:
	/**
	 * Default Constructor - Initialize depth and reflection loss components for a boundary.
	 *
	 * @param height            Bottom depth (meters) as a function of position.
	 *                          Assumes control of this grid and deletes
	 *                          it when the class is destroyed.
	 * @param reflect_loss      Reflection loss model.  Defaults to a
	 *                          Rayleigh reflection for "sand" if NULL.
	 *                          The boundary_model takes over ownship of this
	 *                          reference and deletes it as part of its destructor.
	 */
	boundary_grid_mt(data_grid<DATA_TYPE, NUM_DIMS>* height, reflect_loss_model* reflect_loss = NULL)
		: ThreadBase(), boundary_grid<DATA_TYPE, NUM_DIMS>(height, reflect_loss)
	{
		m_pclMutex = new MutualExclusion();

		m_pclTerminateSignal = new LockingValue<bool>(false);

		LaunchThread(1 * 1024 * 1024);
		usleep(1000);
	}

	/**
	 * Destructor boundary grid.
	 */
	virtual ~boundary_grid_mt()
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
		boundary_grid<DATA_TYPE, NUM_DIMS>::height(location, rho, normal, quick_interp);
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
		boundary_grid<DATA_TYPE, NUM_DIMS>::height(location, rho, normal, quick_interp);
		m_pclMutex->Unlock();
	}

protected:
	/** Mutex to prevent simultaneous access/update by multiple threads. */
	MutualExclusion* m_pclMutex;

private:
	/** Signal to terminate the internal thread. */
	LockingValue<bool>* m_pclTerminateSignal;

};

}  // end of namespace ocean
}  // end of namespace usml
