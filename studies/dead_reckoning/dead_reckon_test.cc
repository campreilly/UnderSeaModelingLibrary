/**
 * @example studies/dead_reckoning/dead_reckon_test.cc
 */

#include <usml/beampatterns/beampatterns.h>
#include <usml/bistatic/bistatic.h>
#include <usml/ocean/boundary_flat.h>
#include <usml/ocean/ocean.h>
#include <usml/platforms/platforms.h>

#include <boost/timer.hpp>
#include <list>

using namespace usml::bistatic;
using namespace usml::ocean;
using namespace usml::platforms;
using namespace usml::beampatterns;

/**
 * Simple sonobuoy sensor for testing. Includes three receiver channels for
 * omni, cosine, and sine beams. Also includes single dipole transmit beam.
 */
class USML_DECLSPEC test_sonobuoy : public sensor_model {
   public:
    /**
     * Construct sensor with default options.
     *
     * @param keyID 	Identification used to find this sensor instance
     *                  in platform_model.
     * @param description   Human readable name for this platform instance.
     * @param time			Time at which platform is being created.
     * @param pos 			Initial location for this platform.
     * @param orient 		Initial orientation for this platform.
     * @param speed			Platform speed (m/s).
     */
    test_sonobuoy(platform_model::key_type keyID,
                    const std::string& description, time_t time = 0.0,
                    const wposition1& pos = wposition1(),
                    const orientation& orient = orientation(),
                    double speed = 0.0)
        : sensor_model(keyID, description, time, pos, orient, speed) {
        if (keyID > 1) {
            src_beam(0,
                     bp_model::csptr(new bp_line(2, 0.75, bp_line_type::VLA)));
        }
        rcv_beam(0, bp_model::csptr(new bp_omni()));
        rcv_beam(1, bp_model::csptr(new bp_trig(bp_trig_type::cosine)));
        rcv_beam(2, bp_model::csptr(new bp_trig(bp_trig_type::sine)));
        time_maximum(20.0);
        compute_reverb(true);
    };
};

/**
 * Command line interface.
 */
int main(int  /*argc*/, char*  /*argv*/[]) {
    platform_manager* platform_mgr = platform_manager::instance();
    bistatic_manager* bistatic_mgr = bistatic_manager::instance();

    // define frequencies for calculation (2.0K, 5.3K, 8.6K, 11.9K)

    seq_vector::csptr freq(new seq_linear(2000.0, 3300.0, 4));
    platform_mgr->frequencies(freq);

    // define ocean characteristics

    cout << "== define ocean characteristics ==" << endl;
    profile_model* water(new profile_linear(1500.0));
    water->attenuation(attenuation_model::csptr(new attenuation_constant(0.0)));
    profile_model::csptr profile(water);

    boundary_model::csptr surface(new boundary_flat());

    boundary_model* btm(new boundary_flat(500));
    btm->reflect_loss(reflect_loss_model::csptr(
        new reflect_loss_rayleigh(bottom_type_enum::sand)));
    btm->scattering(scattering_model::csptr(new scattering_lambert()));
    boundary_model::csptr bottom(btm);

    ocean_model::csptr ocean(new ocean_model(surface, bottom, profile));
    ocean_shared::update(ocean);

    // deploy undersea source traveling north at 10 m/s without a receiver

    cout << "== deploy source instance ==" << endl;
    time_t time = 0.0;
    platform_model::sptr source(
        new test_sonobuoy(3, "source", time, wposition1(54.955, 149.0, -15.0),
                            orientation(), 10.0));
    platform_mgr->add(source);
    bistatic_mgr->add_sensor(source);
    source->update(time, platform_model::FORCE_UPDATE);

    // deploy stationary near surface receiver that is also a source

//    cout << "== deploy receiver instance ==" << endl;
//    time = 60.0;
//    platform_model::sptr receiver(
//        new test_sonobuoy(1, "receiver", 0.0, wposition1(55.0, 149.0, -1.0)));
//    platform_mgr->add(receiver);
//    bistatic_mgr->add_sensor(receiver);
//    receiver->update(time, platform_model::FORCE_UPDATE);

    // wait for threads to finish

    while (thread_task::num_active() > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    cout << "== test complete ==" << endl;
    return 0;
}
