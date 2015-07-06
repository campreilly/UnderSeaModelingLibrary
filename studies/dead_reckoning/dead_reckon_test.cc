/**
 * @example studies/dead_reckoning/dead_reckon_test.cc
 */

#include <usml/ocean/boundary_flat.h>
#include <usml/ocean/ocean_shared.h>
#include <usml/sensors/beams.h>
#include <usml/sensors/sensors.h>
#include <usml/eigenverb/wavefront_generator.h>
#include <list>
#include <boost/timer.hpp>
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
	 * transmit frequency:  2k - 12k Hz
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
		bool multistatic = true;
		vector<double> source_level(1, 200.0);

		// ParamsID = 10
        // Source frequencies 2.0K, 5.3K, 8.6K, 11.9 K
        seq_linear source_frequencies(2000.0, 3300.0, 4);

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

		paramsID = 13;
		receiver_params::reference receiver(
            new receiver_params(paramsID,
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
		wposition1 source_pos(54.955, 149.0, -15.240);         // 5km from receiver
		orientation orient(0.0, 0.0);	 // default orientation

		sensor_manager::instance()->add_sensor(sensorID, paramsID, "Fast Moving Target");

		// Set wave_queue attributes
        wavefront_generator::time_maximum =  7.0/2.0 + 0.5; // reverb_duration/2 + 1/2 sec
        wavefront_generator::intensity_threshold = 150.0; //dB

        /*************************** TIME STEP ********************************/
        wavefront_generator::time_step = 0.1;
        wavefront_generator::extra_rays = 4;
        //wavefront_generator::number_de = 181;
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
	 * Retrieve fathometers and envelopes from sensor_pair_manager.
	 */
	void wait_for_results() {

    	cout << "== wait for results ==" << endl ;

    	sensor_pair_manager* sp_manager = sensor_pair_manager::instance() ;
        fathometer_collection::fathometer_package fathometers ;
        // TODO: add when envelopes are ready
        // envelope_collection::envelope_package envelopes ;

        // Build query for fathometers
        sensor_data_map query;
        sensor_data sensor;
    	orientation orient(0.0, 0.0);  // default orientation

    	// Insert Receiver
    	sensor._sensorID = 1; // ADAR
    	sensor._mode = usml::sensors::RECEIVER;
    	sensor._orient = orient;
    	wposition1 pos(55.0, 149.0, -1.0);
        sensor._position = pos;

    	std::pair<sensor_model::id_type, sensor_data> pair;
    	pair.first = sensor._sensorID;
    	pair.second = sensor;
    	query.insert(pair);

    	// Set up source data
    	sensor._sensorID = 3; // Fast moving target
		sensor._mode = usml::sensors::SOURCE;
		sensor._orient = orient;

    	double latitude = 54.955;      // Starting longitude set earlier
    	double movement = 0.009;	   // 1 km latitude, velocity equivalent of 20 knots = ~ 97.297 sec/km

    	int request_id = 0;
        boost::timer timer;
        // Sleep enough for Waveq3d to run under debugger - 6.5 sec
        boost::this_thread::sleep(boost::posix_time::milliseconds(6800));

        while (true) {

        	request_id++;
        	// Only run three times
        	if ( request_id >= 4 ) break;

        	// Update source position
            wposition1 pos(latitude, 149.0, -15.240);
            sensor._position = pos;
            pair.first = sensor._sensorID;
            pair.second = sensor;
            query.insert(pair);

            cout << " Request ID = " << request_id << endl;
            fathometers = sp_manager->get_fathometers(const_cast<sensor_data_map&>(query));

            if ( fathometers.size() > 0 ) {
                write_fathometers(fathometers, request_id);
            }

            // TODO: add when envelopes are ready
//            envelopes = sp_manager->get_envelopes(const_cast<sensor_data_map&>(query));
//            if ( envelopes.size() > 0 ) {
//            	write_envelopes(envelopes, request_id);
//            }

            query.erase(sensor._sensorID);

			// Reposition for velocity equivalent of 20 knots = ~ 97.297 sec/km
			latitude = latitude + movement;
        }
        cout << "== wait completed for fathometers ==" << endl ;
        cout << "== run matlab dead_reckon_compare.m to verify results ==" << endl ;
	}

	void write_fathometers( fathometer_collection::fathometer_package& fathometers, int request_id ) {

	    std::string ncname_fathometers = USML_STUDIES_DIR "/dead_reckoning/fathometer_";
        fathometer_collection::fathometer_package::iterator iter_fathometers;
        for ( iter_fathometers = fathometers.begin();
            iter_fathometers != fathometers.end(); ++iter_fathometers )
        {
            fathometer_collection* collection = ( *iter_fathometers );
            sensor_model::id_type src_id = collection->source_id();
            sensor_model::id_type rcv_id = collection->receiver_id();
            std::stringstream ss ;
            ss << "src_" << src_id << "_rcv_" << rcv_id << "_request_" << request_id ;
            ncname_fathometers += ss.str();
            ncname_fathometers += ".nc";
            collection->write_netcdf(ncname_fathometers.c_str());

            cout << "Wrote fathometers to " << ncname_fathometers << endl ;
        }
    }

	void write_envelopes( envelope_collection::envelope_package& envelopes, int request_id ) {

		std::string ncname_envelopes = USML_STUDIES_DIR "/dead_reckoning/envelope_";
		envelope_collection::envelope_package::iterator iter_envelopes;
		for ( iter_envelopes = envelopes.begin();
			iter_envelopes != envelopes.end(); ++iter_envelopes )
		{
			envelope_collection* collection = ( *iter_envelopes );
			sensor_model::id_type src_id = collection->source_id();
			sensor_model::id_type rcv_id = collection->receiver_id();
			std::stringstream ss ;
			ss << "src_" << src_id << "_rcv_" << rcv_id << "_request_" << request_id ;
			ncname_envelopes += ss.str();
			ncname_envelopes += ".nc";
			collection->write_netcdf(ncname_envelopes.c_str());

			cout << "Wrote envelopes to " << ncname_envelopes << endl ;
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
