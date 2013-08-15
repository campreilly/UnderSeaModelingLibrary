/**
 * @file lvcms_waveq3d.h
 */

#pragma once

#include <fstream>
#include <iomanip>
#include <string>
#include <vector>

#include <usml/types/lockingvalue.h>
#include <usml/waveq3d/waveq3d.h>
#include <usml/waveq3d/eigenray.h>
#include <usml/waveq3d/proploss.h>
#include <usml/netcdf/netcdf_files.h>
#include <usml/ocean/profile_mt.h>
#include <usml/ocean/boundary_mt.h>

/**
 * @class LvcmsWaveQ3D
 * @brief This is the wrapper class to WaveQ3D.
 */
class LvcmsWaveQ3D :  public ThreadBase
{
public:

	/**
	 * Constructor.
	 */
	LvcmsWaveQ3D();

	/**
	 * Destructor.
	 */
	~LvcmsWaveQ3D();

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
	void* Execute();

	/**
	 * Overrides base class.
	 */
	void CleanUp()
	{
		m_pclDoneSignal->SetValue(true);
	}

	void Process();

	const usml::waveq3d::proploss* getPropLoss() const
	{
		return m_pclPropLoss;
	}

	void setProploss(usml::waveq3d::proploss* pclPropLoss)
	{
		m_pclPropLoss = pclPropLoss;
	}

	const usml::types::wposition1* getSrcPos() const
	{
		return m_pclSrcPos;
	}

	void setSrcPos(const usml::types::wposition1* pclSrcPos)
	{
		m_pclSrcPos = pclSrcPos;
	}

	const usml::types::wposition* getTargets() const
	{
		return m_pclTargets;
	}

	void setTargets(const usml::types::wposition* pclTargets)
	{
		m_pclTargets = pclTargets;
	}

	void setDE(const usml::types::seq_vector* pclDE)
	{
		m_pclDE = pclDE;
	}

	void setAZ(const usml::types::seq_vector* pclAZ)
	{
		m_pclAZ = pclAZ;
	}

	void setFreq(const usml::types::seq_log* pclFreq)
	{
		m_pclFreq = pclFreq;
	}

	void setThreadNum(int num)
	{
		threadNum = num;
	}

	void setOcean(usml::ocean::ocean_model* pclOcean)
	{
		m_pclOcean = pclOcean;
	}

	// Print SPP to a csv file.
	void PrintSPPToCSV(usml::netcdf::netcdf_woa temp, usml::netcdf::netcdf_woa salt,
			usml::ocean::profile_mt* profile, int thread);

private:

	int threadNum;

	// Print eigenrays to a csv file.
	void PrintEigenraysToCSV(usml::waveq3d::proploss* pclPropLoss, int thread);

	/** Pointer to WaveQ3D Source object */
	const usml::types::wposition1* m_pclSrcPos;

	/** Pointer to WaveQ3D Targets object */
	const usml::types::wposition* m_pclTargets;

	/** Pointer to WaveQ3D PropLoss object */
	usml::waveq3d::proploss* m_pclPropLoss;

	/** Pointer to WaveQ3D Ocean object */
	usml::ocean::ocean_model* m_pclOcean;

	/** Pointer to WaveQ3D frequency object */
	const usml::types::seq_log* m_pclFreq;

	/** Pointer to WaveQ3D de object */
	const usml::types::seq_vector* m_pclDE;

	/** Pointer to WaveQ3D az object */
	const usml::types::seq_vector* m_pclAZ;

	/** Signal to terminate the internal thread. */
	LockingValue<bool>* m_pclTerminateSignal;

	/** Signal to terminate the internal thread. */
	LockingValue<bool>* m_pclDoneSignal;

};

