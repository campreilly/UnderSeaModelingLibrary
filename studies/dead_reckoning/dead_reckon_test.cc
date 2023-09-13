/**
 * @example studies/dead_reckoning/dead_reckon_test.cc
 */

#include <bits/types/time_t.h>
#include <usml/beampatterns/bp_line.h>
#include <usml/beampatterns/bp_model.h>
#include <usml/beampatterns/bp_omni.h>
#include <usml/beampatterns/bp_trig.h>
#include <usml/ocean/ocean_utils.h>
#include <usml/sensors/sensor_manager.h>
#include <usml/sensors/sensor_model.h>
#include <usml/types/orientation.h>
#include <usml/types/seq_linear.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition1.h>
#include <usml/usml_config.h>

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

using namespace usml::beampatterns;
using namespace usml::sensors;
using namespace usml::ocean;

class USML_DECLSPEC simple_sonobuoy : public sensor_model {
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
    simple_sonobuoy(platform_model::key_type keyID,
                    const std::string& description, time_t time = 0.0,
                    const wposition1& pos = wposition1(),
                    const orientation& orient = orientation(),
                    double speed = 0.0)
        : sensor_model(keyID, description, time, pos, orient, speed) {
        src_beam(0, bp_model::csptr(new bp_line(2, 0.75, bp_line_type::VLA)));
        rcv_beam(0, bp_model::csptr(new bp_omni()));
        rcv_beam(1, bp_model::csptr(new bp_trig(bp_trig_type::cosine)));
        rcv_beam(2, bp_model::csptr(new bp_trig(bp_trig_type::sine)));
    }
};

/**
 * Command line interface.
 */
int main(int /*argc*/, char* /*argv*/[]) {
    sensor_manager* smgr = sensor_manager::instance();

    // define frequencies for calculation (2.0K, 5.3K, 8.6K, 11.9K)

    seq_vector::csptr freq(new seq_linear(2000.0, 3300.0, 4));
    smgr->frequencies(freq);

    // define ocean characteristics

    cout << "== define ocean characteristics ==" << endl;
    ocean_utils::make_iso(500.0);

    // deploy undersea source traveling north at 10 m/s without a receiver

    cout << "== deploy source instance ==" << endl;
    time_t time = 0.0;
    sensor_model::sptr source(
        new simple_sonobuoy(3, "source", time, wposition1(54.955, 149.0, -15.0),
                            orientation(), 10.0));
    smgr->add_sensor(source);
    source->update(time, platform_model::FORCE_UPDATE);

    // deploy stationary near surface receiver that is also a source

    cout << "== deploy receiver instance ==" << endl;
    time = 60.0;
    sensor_model::sptr receiver(
        new simple_sonobuoy(1, "receiver", 0.0, wposition1(55.0, 149.0, -1.0)));
    smgr->add_sensor(receiver);
    receiver->update(time, platform_model::FORCE_UPDATE);
    thread_task::wait();

    cout << "== test complete ==" << endl;
    return 0;
}
