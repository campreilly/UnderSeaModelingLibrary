/**
 * @example studies/reverberation/reverb_analytic_test.cc
 */
#include <usml/ocean/ocean_shared.h>
#include <usml/sensors/beams.h>
#include <usml/sensors/sensors.h>
#include <list>
#include <boost/progress.hpp>
#include <boost/foreach.hpp>

using namespace usml::sensors;

/**
 * This scenario compute reverberation for a scenario that has a simple
 * analytic solution.  This scenario has a monostatic sensor, a flat bottom,
 * and a constant sound speed.  Reverberation is only computed for the
 * ocean bottom.
 */
class reverb_analytic_test {

public:

	/**
	 * Initializes the scenario, starts the reverberation calculation,
	 * and waits for the results.
	 */
	reverb_analytic_test() {
		define_ocean_characteristics();
		define_sensor_characteristics();
		deploy_sensor_instance();     // starts reverb calc `
		wait_for_results();
		cout << "== test complete ==" << endl;
	}

	/**
	 * Cleans up singletons so that valgrind can be used to find
	 * other memory leaks.
	 */
	~reverb_analytic_test() {
	}

private:

	/**
	 * Creates a simple ocean for comparison to the analytic solution
	 * for monostatic reverberation defined in the reverb theory paper.
	 *
	 * <pre>
	 * profile:             constant 1500 m/s
	 * attenuation:         none
	 * surface loss:        none
	 * surface scattering:  none
	 * bottom depth:        constant 200 meters
	 * bottom loss:         Rayleigh sand
	 * bottom scattering:   Lambert -27 dB
	 * </pre>
	 *
	 * Generates the ocean model and then updates the ocean_shared singleton.
	 * Uses profile_lock and boundary_lock classes so that the new ocean
	 * can be shared by multiple threads.
	 */
	void define_ocean_characteristics() {
		cout << "== define ocean characteristics ==" << endl;
		profile_model* water = new profile_linear(1500.0);
		water->attenuation(new attenuation_constant(0.0));
		profile_lock* profile = new profile_lock(water);

		boundary_model* surf = new boundary_flat();
		boundary_lock* surface = new boundary_lock(surf);

		boundary_model* btm = new boundary_flat(200.0);
		btm->reflect_loss(
				new reflect_loss_rayleigh(reflect_loss_rayleigh::SAND));
		btm->scattering(new scattering_lambert());
		boundary_lock* bottom = new boundary_lock(btm);

		ocean_shared::reference ocean(
				new ocean_model(surface, bottom, profile));
		ocean_shared::update(ocean);
	}

	/**
	 * Defines the source and receiver parameters for a simple
	 * omni-direction sensor.
	 *
	 * <pre>
	 * multistatic:			false
	 * source strength:     200 dB
	 * transmit frequency:  3 KHz
	 * src beam pattern:    omni directional
	 * rcv beam pattern: 	omni directional
	 * </pre>
	 *
	 * Generates the sensor parameter then updates the sensor parameter
	 * singletons.
	 */
	void define_sensor_characteristics() {
		cout << "== define sensor characteristics ==" << endl;
		sensor_params::id_type paramsID = 1;
		bool multistatic = false;
		vector<double> source_level(1, 200.0);
        // Source frequencies 6.5K, 7.5K, 8.5K, 9.5K
		seq_linear source_frequencies(6500.0, 1000.0, 4);
        // Receiver frequencies 3.0K, 10.0K
        seq_linear receiver_frequencies(3000.0, 7000.0, 2);
		beam_pattern_model::id_type omni = 0;

		// define source parameters

		source_params::reference source(
				new source_params(paramsID, source_level,
                source_frequencies, omni, multistatic));
		source_params_map::instance()->insert(source->paramsID(), source);

		// define receiver parameters

		std::list<beam_pattern_model::id_type> receiver_beams;
		receiver_beams.push_back(omni);

		receiver_params::reference receiver(
            new receiver_params(paramsID, receiver_frequencies, receiver_beams, multistatic));
		receiver_params_map::instance()->insert(receiver->paramsID(), receiver);
	}

	/**
	 * Creates a new instance of the simple omni-direction sensor,
	 * and updates it with a position of (0.0,0.0,0.0).  The update
	 * causes reverberation calculations to start.
	 */
	void deploy_sensor_instance() {
		cout << "== deploy sensor instance ==" << endl;
		sensor_model::id_type sensorID = 1;
		sensor_params::id_type paramsID = 1;
		wposition1 pos(0.0, 0.0);				// locate on ocean surface
		orientation orient(0.0, 0.0);	// default orientation

		sensor_manager::instance()->add_sensor(sensorID, paramsID, "sensor1");
		sensor_manager::instance()->update_sensor(sensorID, pos, orient, true);
	}

	/**
	 * Wait for the reverberation model to compute results.
	 *
	 * TODO Retrieve envelopes from sensor_pair_manager.
	 * TODO Record results of this test.
	 */
	void wait_for_results() {

    	cout << "== wait for results ==" << endl ;

    	sensor_pair_manager* sp_manager = sensor_pair_manager::instance() ;
        fathometer_model::fathometer_package fathometers;
        envelope_collection::reference envelopes ;

    	// Build query for fathometers
    	sensor_model::id_type sensor_ids[] = {1};

    	// For modes use one of the following
    	// usml::sensors::SOURCE
        // usml::sensors::RECEIVER
        // usml::sensors::BOTH
    	xmitRcvModeType sensor_modes[] = {usml::sensors::BOTH};

        // Build a query
        sensor_pair_manager::sensor_query_map query;
        for ( int i = 0; i < sizeof(sensor_ids) / sizeof(sensor_model::id_type); ++i ) {
            query.insert(std::pair<sensor_model::id_type, xmitRcvModeType>(sensor_ids[i], sensor_modes[i]));
        }

        // wait for results
        boost::timer timer ;

        while ( true ) {
            fathometers = sp_manager->get_fathometers(query);
            //envelopes = manager->get_envelopes(sensorID);
            if ( fathometers.size() > 0 ) break ;
            //if ( fathometers.size() > 0 && envelopes.get() != NULL ) break ;
            boost::this_thread::sleep(boost::posix_time::milliseconds(250));
        }
        cout << "waited for " << timer.elapsed() << " secs" << endl ;

        int index = 0;
        std::string ncname = USML_STUDIES_DIR "/reverberation/fathometer_";
        fathometer_model::fathometer_package::iterator iter;
        for ( iter = fathometers.begin(); iter != fathometers.end(); ++iter )
        {
            fathometer_model* model = ( *iter );
            sensor_model::id_type src_id = model->source_id();
            sensor_model::id_type rcv_id = model->receiver_id();
            std::stringstream ss ;
            ss << "src_" << src_id << "_rcv_" << rcv_id;
            ncname += ss.str();
            ncname += ".nc";
            model->write_netcdf(ncname.c_str());
        }

        sensor_pair_manager* sp_man = sensor_pair_manager::instance();
    
        std::string ncname_all = USML_STUDIES_DIR "/reverberation/fathometers.nc";
        sp_man->write_fathometers(fathometers, ncname_all.c_str());

		// Delete all for valgrind memcheck testing
        BOOST_FOREACH(fathometer_model* fathometer, fathometers) {
            delete fathometer;
        }
	}
};

/**
 * Command line interface.
 */
int main(int argc, char* argv[]) {
	reverb_analytic_test test;
	return 0;
}
