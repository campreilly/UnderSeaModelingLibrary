/**
 * @example studies/reverberation/reverb_analytic_test.cc
 */
#include <usml/ocean/ocean_shared.h>
#include <usml/sensors/beams.h>
#include <usml/sensors/sensors.h>
#include <list>
#include <boost/progress.hpp>

using namespace usml::sensors ;

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
        define_ocean_characteristics() ;
//        define_sensor_characteristics() ;
//        add_sensor_instance() ;     // starts reverb calc `
//        wait_for_results() ;
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
        cout << "== define_ocean_characteristics ==" ;
        profile_model* water = new profile_linear(1500.0);
        water->attenuation(new attenuation_constant(0.0));
        profile_lock* profile = new profile_lock( water ) ;

        boundary_model* surf = new boundary_flat();
        boundary_lock* surface = new boundary_lock( surf ) ;

        boundary_model* btm = new boundary_flat(200.0);
        btm->reflect_loss(new reflect_loss_rayleigh(reflect_loss_rayleigh::SAND));
        btm->scattering(new scattering_lambert());
        boundary_lock* bottom = new boundary_lock( btm ) ;

        ocean_shared::reference ocean( new ocean_model(surface, bottom, profile) );
        ocean_shared::update(ocean);
    }

//    /**
//     * Defines sensor parameters for source and receiver behaviors.
//     *
//     * <pre>
//     * profile:             constant 1500 m/s
//     * source strength:     200 dB
//     * transmit frequency:  3000 dB
//     * beam pattern:        omni directional
//     * </pre>
//     *
//     * Generates the sensor parameter then updates the sensor parameter
//     * singletons.
//     */
//    void define_sensor_characteristics() {
//
//        // define beam patterns
//
//        beam_pattern_map::instance()->insert( 1, new beam_pattern_omni() ) ;
//
//        // define source parameters
//
//        std::list<beamIDType> source_beams ;
//        source_beams.push_back(1);
//
//        source_params* source = new source_params(
//                1,          // sourceID
//                200.0,      // sourceStrength
//                3000.0,     // transmitFrequency
//                0.0,        // initialPingTime
//                0.0,        // repeationInterval
//                source_beams ) ;
//        source_params_map::instance()->insert( 1, source ) ;
//
//        // define receiver parameters
//
//        std::list<beamIDType> receiver_beams ;
//        receiver_beams.push_back(1);
//
//        receiver_params* receiver = new receiver_params( receiver_beams ) ;
//        receiver_params_map::instance()->insert( 1, receiver ) ;
//    }
//
//    /**
//     * Creates a new instance of the sensor, and adds it to the reverberation model.
//     * Cause reverberation calculations to start.
//     */
//    void add_sensor_instance() {
//        wposition1 position() ;
//        sensor* sensor_data = new sensor(
//                1,     // sensorID
//                1,      // paramsID
//                usml::sensors::BOTH,
//                position,
//                0.0,    // pitch
//                0.0,    // yaw
//                "omni" ) ;
//        sensor_map::instance()->insert(sensor_data->sensorID(), sensor_data);
//    }
//
//    /**
//     * Wait for the reverberation model to compute results.
//     */
//    void wait_for_results() {
//        sensor_pair_manager* manager = sensor_pair_manager::instance() ;
//        proploss* fathometers = NULL ;
//        envelope_collection* envelopes = NULL ;
//
//        // wait for results
//
//        boost::timer timer ;
//        while ( true ) {
//            fathometers = manager->get_fathometers(1);
//            envelopes = manager->get_envelopes(1);
//            if ( fathometers != NULL && envelopes ) break ;
//            boost::this_thread::sleep(boost::posix_time::milliseconds(100));
//        }
//        cout << "waited for " << timer.elapsed() << " secs" << endl ;
//
//        // record results of this test
//
//        // TBD
//    }
};

/**
 * Command line interface.
 */
int main( int argc, char* argv[] ) {
    reverb_analytic_test test ;
    return 0 ;
}
