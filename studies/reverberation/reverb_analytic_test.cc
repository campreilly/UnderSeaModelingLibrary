/**
 * @example studies/reverberation/reverb_analytic_test.cc
 */
#include <usml/beampatterns/bp_model.h>
#include <usml/beampatterns/bp_omni.h>
#include <usml/eigenverbs/eigenverbs.h>
#include <usml/ocean/ocean.h>
#include <usml/platforms/platform_manager.h>
#include <usml/sensors/sensors.h>
#include <usml/wavegen/wavegen.h>

#include <list>

using namespace usml::ocean;
using namespace usml::sensors;
using namespace usml::eigenverbs;
using namespace usml::wavegen;

/**
 * This scenario compute reverberation for a scenario that has a simple
 * analytic solution. This scenario has a monostatic sensor, a flat bottom,
 * and a constant sound speed. Reverberation is only computed for the
 * ocean bottom, because that is the only interface given a scattering strength
 * in define_ocean().
 */
class reverb_analytic_test {
   public:
    /**
     * Initializes the scenario, starts the reverberation calculation,
     * and waits for the results.
     */
    reverb_analytic_test() {
        define_ocean();
        deploy_sensors();
        analyze_results();
        sensor_manager::reset();
        cout << "== test complete ==" << endl;
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
     * Generates the ocean model and then updates the ocean_shared
     * singleton. Uses profile_lock and boundary_lock classes so that the
     * new ocean can be shared by multiple threads.
     */
    void define_ocean() {
        cout << "== define ocean ==" << endl;

        attenuation_model::csptr attn(new attenuation_constant(0.0));
        profile_model::csptr profile(new profile_linear(1500.0, attn));

        boundary_model* btm = new boundary_flat(200.0);
        btm->reflect_loss(reflect_loss_model::csptr(
            new reflect_loss_rayleigh(bottom_type_enum::sand)));
        btm->scattering(scattering_model::csptr(new scattering_lambert()));
        boundary_model::csptr bottom(btm);

        boundary_model::csptr surface(new boundary_flat());
        ocean_model::csptr ocean(new ocean_model(surface, bottom, profile));
        ocean_shared::update(ocean);
    }

    /**
     * Creates a new instance of the simple omni-direction sensor,
     * and updates it with a position of (0.0,0.0,0.0). The update
     * causes reverberation calculations to start.
     */
    void deploy_sensors() {
        cout << "== deploy sensors ==" << endl;
        auto* sensor_mgr = sensor_manager::instance();
        seq_vector::csptr freq(new seq_linear(3000.0, 1.0, 1));
        sensor_mgr->frequencies(freq);

        const int platformID = 1;
        sensor_model::sptr sensor(new sensor_model(platformID, "sensor"));
        sensor->src_beam(0, bp_model::csptr(new bp_omni()));
        sensor->rcv_beam(0, bp_model::csptr(new bp_omni()));
        sensor->time_maximum(7.0);
        sensor->compute_reverb(true);

        sensor_mgr->add_sensor(sensor);
        sensor->update(0.0, platform_model::FORCE_UPDATE);

        thread_task::wait();	// wait for acoustic processing to finish
    }

    /**
     * Use
     */
    void compute_reverb() {
        cout << "== compute reverb ==" << endl;

//        rvbts_task = std::make_shared<rvbts_generator>(
//            this, tpos, targetIDs, frequencies, _de_fan, _az_fan,
//            _time_step, _time_maximum, _intensity_threshold, _max_bottom,
//            _max_surface);
//
//        rvbts_generator(const sensor_pair::sptr& pair,
//                         const seq_vector::csptr& times,
//                         const seq_vector::csptr& freqs, size_t num_azimuths);
//
//        thread_task::wait();	// wait for acoustic processing to finish
    }

    /**
     * Wait for the reverberation model to compute results.
     * Retrieve eigenrays and envelopes from sensor_pair_manager,
     * and write them to netCDF files for further analysis.
     */
    void analyze_results() {
        cout << "== analyze results ==" << endl;
        const char* ncname = USML_TEST_DIR "/studies/reverberation/";
        auto* sensor_mgr = sensor_manager::instance();

        // write direct path collections to disk

        cout << endl << "*** pairs ***" << endl;
        for (const auto& pair : sensor_mgr->list()) {
            cout << pair->description()
                 << " dirpaths=" << pair->dirpaths()->eigenrays().size()
                 << endl;
            if (pair->dirpaths() != nullptr) {
                std::ostringstream filename;
                filename << ncname << "dirpaths_" << pair->hash_key() << ".nc";
                cout << "writing " << filename.str().c_str() << endl;
                pair->dirpaths()->write_netcdf(filename.str().c_str());
            }
            if (pair->src_eigenverbs() != nullptr) {
                std::ostringstream filename;
                filename << ncname << "src_eigenverbs_" << pair->hash_key()
                         << ".nc";
                cout << "writing " << filename.str().c_str() << endl;
                pair->src_eigenverbs()->write_netcdf(filename.str().c_str(), 0);
            }
            if (pair->rcv_eigenverbs() != nullptr) {
                std::ostringstream filename;
                filename << ncname << "rcv_eigenverbs_" << pair->hash_key()
                         << ".nc";
                pair->rcv_eigenverbs()->write_netcdf(filename.str().c_str(), 0);
            }
            if (pair->biverbs() != nullptr) {
                std::ostringstream filename;
                filename << ncname << "biverbs_" << pair->hash_key() << ".nc";
                cout << "writing " << filename.str().c_str() << endl;
                pair->biverbs()->write_netcdf(filename.str().c_str(), 0);
            }
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
