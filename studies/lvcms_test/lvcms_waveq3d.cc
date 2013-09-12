/*
 * lvcms_waveq3d.cc
 *
 *  Created on: Aug 13, 2013
 *      Author: safuser
 */

#include "lvcms_waveq3d.h"

using namespace usml::waveq3d ;
using namespace usml::netcdf ;
using namespace usml::ocean ;

LvcmsWaveQ3D::LvcmsWaveQ3D() : ThreadBase(),
	m_pclSrcPos(NULL), m_pclPropLoss(NULL), m_pclOcean(NULL), m_pclFreq(NULL), m_pclDE(NULL), m_pclAZ(NULL)

{
	m_pclTerminateSignal = new LockingValue<bool>(false);
	m_pclDoneSignal = new LockingValue<bool>(false);
}

LvcmsWaveQ3D::~LvcmsWaveQ3D()
{
	// Signal the internal thread to terminate, and then wait here until it does.
	m_pclTerminateSignal->SetValue(true);
	WaitThread();

	if (m_pclDoneSignal)
	{
		delete m_pclDoneSignal;
		m_pclDoneSignal = NULL;
	}

	if (m_pclTerminateSignal)
	{
		delete m_pclTerminateSignal;
		m_pclTerminateSignal = NULL;
	}
}

void LvcmsWaveQ3D::Process()
{
	LaunchThread(1 * 1024 * 1024);
	usleep(1000);
}

void* LvcmsWaveQ3D::Execute()
{
	try {

#define DEBUG

		time_t start = time(NULL) ;

		wave_queue wave( *m_pclOcean, *m_pclFreq, *m_pclSrcPos, *m_pclDE, *m_pclAZ, m_timeStep, m_pclTargets ) ;

		if (!wave.addProplossListener(m_pclPropLoss)) {
			cout << "Error adding proploss listener! " << endl ;
			exit(1);
		}

		cout << "*** LvcmsWaveQ3D::threadNum is " << threadNum << endl;

	#ifdef DEBUG
		char ncname_wave[40];
		sprintf(ncname_wave,"lvcms_test_wave_front.%d.nc", threadNum);
		cout << "writing wavefronts to " << ncname_wave << endl;
		wave.init_netcdf( ncname_wave );
		wave.save_netcdf();
	#endif

		cout << "LvcmsWaveQ3D:: propagate wavefronts for " << m_maxTime << " secs" << endl ;

		while ( wave.time() < m_maxTime ) {
			wave.step() ;
	#ifdef DEBUG
			//cout << "*** LvcmsWaveQ3D::step: time=" << wave.time() << endl ;
			wave.save_netcdf();
	#endif
		}

	#ifdef DEBUG
		wave.close_netcdf();
	#endif

		// compute coherent propagation loss
		m_pclPropLoss->sum_eigenrays();

	#ifdef DEBUG

		// write eigenrays to disk
		char ncname[40];
		sprintf(ncname,"lvcms_test_eigenrays.%d.nc", threadNum);
		cout << "writing eigenrays to " << ncname << endl;
		m_pclPropLoss->write_netcdf(ncname,"lvcms test eigenrays");

		PrintEigenraysToCSV(m_pclPropLoss, threadNum);

	#endif
		time_t complete = time(NULL) ;
		cout << "Propagating for " << m_maxTime << " sec with "
			 << ( m_pclTargets->size1() * m_pclTargets->size2() ) << " targets took "
			 << (difftime(complete,start)) << " sec." << endl ;

	} catch (std::exception const& e)
	{
		cout << "Caught Exception " << e.what() << endl ;
	}

	return NULL;
}

void LvcmsWaveQ3D::PrintSPPToCSV(netcdf_woa temp, netcdf_woa salt,  profile_mt* profile, int thread)
{
	char ncname_ssp[40];
	sprintf(ncname_ssp,"lvcms_test_ssp.%d.csv", thread);

    // print ssp results for source lat/long entry
	std::ofstream ssp_output(ncname_ssp);
	cout << "writing tables to " << ncname_ssp << endl;

	unsigned index[3];
	index[1] = 0;
	index[2] = 0;

	matrix<double> speed(1, 1);
	wposition location(1, 1);
	location.latitude(0, 0, 29.0);
	location.longitude(0, 0, -80.0);
	wvector gradient(1, 1);

	ssp_output << "Depth,Temp,Sal,Speed,Gradient" << endl;

	for (unsigned d = 0; d < temp.axis(0)->size(); ++d) {
		index[0] = d;
		location.rho(0, 0, (*temp.axis(0))(d));
		profile->sound_speed(location, &speed, &gradient);
		ssp_output << -location.altitude(0, 0) << "," << temp.data(index)
		<< "," << salt.data(index) << "," << speed(0, 0) << ","
		<< -gradient.rho(0, 0) << std::endl;
	}

	ssp_output.close();

	return;
}


void LvcmsWaveQ3D::PrintEigenraysToCSV(usml::waveq3d::proploss* pclPropLoss, int thread)
{
	char csvname[40];
	sprintf(csvname,"lvcms_test_eigenray.%d.csv", thread);

	// save results to spreadsheet and compare to analytic results
	cout << "writing tables to " << csvname << endl;
	std::ofstream os(csvname);
	os << "target, ray, time, intensity, phase, s_de, s_az, t_de, t_az, srf, btm, cst" << endl;
	os << std::setprecision(18);
	cout << std::setprecision(18);

	for (unsigned int m = 0; m < 3 /* num targets */; ++m) {
		const eigenray_list *raylist = pclPropLoss->eigenrays(m,0);
		int n=0;
		for ( eigenray_list::const_iterator iter = raylist->begin();
				iter != raylist->end(); ++n, ++iter )
		{
			const eigenray &ray = *iter ;
			os  <<  m
				<< "," << n
				<< "," << ray.time
				<< "," << ray.intensity(0)
				<< "," << ray.phase(0)
				<< "," << ray.source_de
				<< "," << ray.source_az
				<< "," << ray.target_de
				<< "," << ray.target_az
				<< "," << ray.surface
				<< "," << ray.bottom
				<< "," << ray.caustic
				<< endl;
		}
	}

	os.close();
	return;
}
