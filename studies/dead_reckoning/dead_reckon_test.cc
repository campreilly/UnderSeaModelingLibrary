/**
 * @example studies/dead_reckoning/dead_reckon_test.cc
 */
#include <usml/ocean/boundary_flat.h>
#include <usml/ocean/ocean_shared.h>
#include <usml/sensors/beams.h>
#include <usml/sensors/sensors.h>
#include <usml/eigenverb/wavefront_generator.h>
#include <list>
#include <boost/progress.hpp>
#include <boost/foreach.hpp>

using namespace usml::sensors ;
using namespace usml::eigenverb ;

/**
 * This scenario compute reverberation for a scenario that has a simple
 * analytic solution.  This is a bi-static scenario that uses two sensors,
 * one source, one receiver, a flat bottom, and a constant sound speed.
 * Reverberation is only computed for the ocean bottom.
 */
class dead_reckon_test {

public:

	/**
	 * Initializes the scenario, starts the reverberation calculation,
	 * and waits for the results.
	 */
	dead_reckon_test() {
		define_ocean_characteristics();
		define_sensor_characteristics();
		deploy_sensor_instance();
		wait_for_results();
		cout << "== test complete ==" << endl;
	}

	/**
	 * Cleans up singletons so that valgrind can be used to find
	 * other memory leaks.
	 */
	~dead_reckon_test() {
	}

private:

	/**
	 * Creates a simple ocean for comparison to the analytic solution
	 * for bi-static reverberation.
	 *
	 * <pre>
	 * profile:             constant 1500 m/s
	 * attenuation:         none
	 * surface loss:        none
	 * surface scattering:  none
	 * bottom depth:        constant 505 meters
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

		/****************** BOTTOM DEPTH *****************************/
		boundary_model* btm = new boundary_flat(505); // default 1657 ft.
		btm->reflect_loss(
				new reflect_loss_rayleigh(reflect_loss_rayleigh::SAND));
		btm->scattering(new scattering_lambert());
		boundary_lock* bottom = new boundary_lock(btm);

		ocean_shared::reference ocean(
				new ocean_model(surface, bottom, profile));
		ocean_shared::update(ocean);
	}

	/**
	 * Defines the a source and a receiver parameters for a simple
	 * sensor.
	 *
	 * <pre>
	 * multistatic:			true
	 * source strength:     200 dB
	 * pulse length:		250 msec
	 * reverb duration:		7.0 sec
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
		sensor_params::id_type paramsID = 10;
		bool multistatic = false;
		vector<double> source_level(1, 200.0);

		// ParamsID = 10
        // Source frequencies 2.0K, 5.3K, 8.6K, 11.9 K
        seq_linear source_frequencies(2000.0, 33000, 4);

        beam_pattern_model::id_type omni = 0;
        std::list<beam_pattern_model::id_type> source_beams;
        source_beams.push_back(omni);

        // Receiver frequencies 500, 750
        seq_linear receiver_frequencies(500.0, 250.0, 2);
		
		// define source parameters

		source_params::reference source(
				new source_params(paramsID, source_level,
				0.250,						// pulse_length
				7.0,						// reverb_duration
                10.0, 50000.0,       		// min, max active freq
                source_frequencies, source_beams, multistatic));
		source_params_map::instance()->insert(source->paramsID(), source);

		// define receiver parameters

		std::list<beam_pattern_model::id_type> receiver_beams;
		receiver_beams.push_back(omni);

		receiver_params::reference receiver(
            new receiver_params(13,
            10.0, 50000.0,  // min, max active freq
            receiver_frequencies, receiver_beams, multistatic));
		receiver_params_map::instance()->insert(receiver->paramsID(), receiver);
	}

	/**
	 * Creates a new instance of a fast-moving target and an
	 * omni-direction receiver, and updates it with a position of
	 * (55.0, 149.0, 1.0).
	 */
	void deploy_sensor_instance() {

		cout << "== deploy source instance ==" << endl;
		sensor_model::id_type sensorID = 3;
		sensor_params::id_type paramsID = 10;
		wposition1 source_pos(55.0, 149.4, -15.24003);
		orientation orient(0.0, 0.0);	 // default orientation

		sensor_manager::instance()->add_sensor(sensorID, paramsID, "Fast Moving Target");

		// Set wave_queue attributes
        wavefront_generator::time_maximum =  7.0/2.0 + 0.5; // reverb_duration/2 + 1/2 sec
        wavefront_generator::intensity_threshold = 150.0; //dB

        /*************************** TIME STEP ********************************/
        wavefront_generator::time_step = 0.1;
        wavefront_generator::extra_rays = 4;
        //wavefront_generator::number_de = 181;             // For comparsion to eigenverb_demo.m
        //wavefront_generator::max_bottom = 0;              // Max number of bottom bounces.
        //wavefront_generator::max_surface = 0;             // Max number of surface bounces.

        // Update sensor data and run wave_queue.
        sensor_manager::instance()->update_sensor(sensorID, source_pos, orient);


        cout << "== deploy receiver instance ==" << endl;
		// Receiver
		sensorID = 1;
        paramsID = 13;
        wposition1 receiver_pos(55.0, 149.0, -1.0);  // locate close to ocean surface
        // default orientation

        sensor_manager::instance()->add_sensor(sensorID, paramsID, "ADAR");

		// Update sensor data and run wave_queue.
		sensor_manager::instance()->update_sensor(sensorID, receiver_pos, orient);
	}



	/**
	 * Wait for the reverberation model to compute results.
	 *
	 * Retrieve fathometers from sensor_pair_manager.
	 */
	void wait_for_results() {

    	cout << "== wait for results ==" << endl ;

    	sensor_pair_manager* sp_manager = sensor_pair_manager::instance() ;
        fathometer_collection::fathometer_package fathometers ;

        // Build query for fathometers
        sensor_data sensor;
    	orientation orient(0.0, 0.0);  // default orientation
    	double longitude = 149.4;      // Starting longitude
    	double movement = 0.001;
        sensor_data_list query;

        // wait for results
        sleep(4);

        boost::timer timer ;

        while (true){

            wposition1 pos(55.0, longitude, 15.24003);

            sensor._sensorID = 3; // Fast moving target
            sensor._mode = usml::sensors::SOURCE;
            sensor._position = pos;
            sensor._orient = orient;
            query.push_back(sensor);

            fathometers = sp_manager->get_fathometers(query);

            longitude = longitude - movement;

            if ( fathometers.size() > 0 ) {
                write_fathometers(fathometers);
            }

            cout << "waited for " << timer.elapsed() << " secs" << endl ;
        }
	}

	void write_fathometers( fathometer_collection::fathometer_package& fathometers ) {

	    std::string ncname_fathometers = USML_STUDIES_DIR "/reverberation/fathometer_";
        fathometer_collection::fathometer_package::iterator iter_fathometers;
        for ( iter_fathometers = fathometers.begin();
            iter_fathometers != fathometers.end(); ++iter_fathometers )
        {
            fathometer_collection* model = ( *iter_fathometers );
            sensor_model::id_type src_id = model->source_id();
            sensor_model::id_type rcv_id = model->receiver_id();
            std::stringstream ss ;
            ss << "src_" << src_id << "_rcv_" << rcv_id;
            ncname_fathometers += ss.str();
            ncname_fathometers += ".nc";
            model->write_netcdf(ncname_fathometers.c_str());
        }
    }
};

/**
 * Command line interface.
 */
int main(int argc, char* argv[]) {
	dead_reckon_test test;
	return 0;
}
