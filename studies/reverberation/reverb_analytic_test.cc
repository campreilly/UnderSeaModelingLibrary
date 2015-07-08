/**
 * @example studies/reverberation/reverb_analytic_test.cc
 */
#include <usml/ocean/ocean_shared.h>
#include <usml/sensors/beams.h>
#include <usml/sensors/sensors.h>
#include <usml/eigenverb/wavefront_generator.h>
#include <list>
#include <boost/progress.hpp>
#include <boost/foreach.hpp>

using namespace usml::sensors;
using namespace usml::eigenverb;

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

        // define global control parameters

        wavefront_generator::intensity_threshold = 170.0; //dB
        wavefront_generator::time_maximum = 4.0;
        wavefront_generator::time_step = 0.001;
        wavefront_generator::extra_rays = 4;
        wavefront_generator::max_bottom = 2;
        wavefront_generator::max_surface = 2;

        // execute scenario

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
        sensor_params::id_type paramsID = 1;
        bool multistatic = false;
        vector<double> source_level(1, 200.0);
        seq_linear freq(3000.0, 1.0, 1);

        beam_pattern_model::id_type omni = 0;
        std::list<beam_pattern_model::id_type> beam_list;
        beam_list.push_back(omni);

        // define source parameters

        source_params::reference source(
                new source_params(paramsID, source_level, 0.250,// pulse_length
                        7.0,                // reverb_duration
                        3000, 3000.0,       // min, max active freq
                        freq, beam_list, multistatic));
        source_params_map::instance()->insert(source->paramsID(), source);

        // define receiver parameters

        std::list<beam_pattern_model::id_type> receiver_beams;
        receiver_params::reference receiver(
                new receiver_params(paramsID,
                        3000.0, 3000.0,     // min, max active freq
                        freq, beam_list, multistatic));
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
        wposition1 pos(0.0, 0.0);        // locate on ocean surface
        orientation orient(0.0, 0.0);	 // default orientation

        sensor_manager::instance()->add_sensor(sensorID, paramsID, "sensor1");
        sensor_manager::instance()->update_sensor(sensorID, pos, orient, true);
    }

    /**
     * Wait for the reverberation model to compute results.
     * Retrieve eigenrays and envelopes from sensor_pair_manager,
     * and write them to netCDF files for further analysis.
     */
    void wait_for_results() {
        cout << "== wait for results ==" << endl;

        // wait for results

        boost::timer timer;
        while (true) {
            if (thread_task::num_active() == 0) break;
            boost::this_thread::sleep(boost::posix_time::milliseconds(250));
        }
        cout << "waited for " << timer.elapsed() << " secs" << endl;

        // build list of sensors for extracting data from sensor_pair_manager

        sensor_pair_manager* sp_manager = sensor_pair_manager::instance();
        sensor_model::id_type sensor_ids[] = { 1 };
        xmitRcvModeType sensor_modes[] = { usml::sensors::BOTH };

        sensor_data_map query;
        for (int i = 0; i < sizeof(sensor_ids) / sizeof(sensor_model::id_type); ++i) {
            sensor_data sensor;
            sensor._sensorID = sensor_ids[i];
            sensor._mode = sensor_modes[i];
            sensor._position = wposition1(0.0, 0.0);
            sensor._orient = orientation(0.0, 0.0);
            query[sensor._sensorID] = sensor ;
        }

        // write fathometer data to netCDF file

        fathometer_collection::fathometer_package fathometers =
                sp_manager->get_fathometers(query);
        const std::string ncname_fathometers = USML_STUDIES_DIR "/reverberation/fathometer_";
        BOOST_FOREACH( fathometer_collection* model, fathometers ) {
            sensor_model::id_type src_id = model->source_id();
            sensor_model::id_type rcv_id = model->receiver_id();
            std::stringstream ss;
            ss << ncname_fathometers << "src_" << src_id << "_rcv_" << rcv_id << ".nc" ;
            cout << "write to " << ss.str().c_str() << endl ;
            model->write_netcdf( ss.str().c_str() );
        }

        // write envelope data to netCDF file

        envelope_collection::envelope_package envelopes =
                sp_manager->get_envelopes(query);
        const std::string ncname_envelopes = USML_STUDIES_DIR "/reverberation/envelopes_";
        BOOST_FOREACH( envelope_collection* collection, envelopes ) {
            sensor_model::id_type src_id = collection->source_id();
            sensor_model::id_type rcv_id = collection->receiver_id();
            std::stringstream ss;
            ss << ncname_envelopes << "src_" << src_id << "_rcv_" << rcv_id << ".nc" ;
            cout << "write to " << ss.str().c_str() << endl ;
            collection->write_netcdf( ss.str().c_str() ) ;
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
