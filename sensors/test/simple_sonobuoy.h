/**
 * @file simple_sonobuoy.h
 * Simple sonobuoy sensor for testing.
 */
#pragma once

#include <bits/types/time_t.h>
#include <usml/beampatterns/bp_line.h>
#include <usml/beampatterns/bp_model.h>
#include <usml/beampatterns/bp_omni.h>
#include <usml/beampatterns/bp_trig.h>
#include <usml/sensors/sensor_model.h>
#include <usml/types/orientation.h>
#include <usml/types/wposition1.h>
#include <usml/usml_config.h>

#include <string>

using namespace usml::sensors;

namespace usml {
namespace platforms {
namespace test {

/**
 * @ingroup sensors_test
 * @{
 */

/**
 * Simple sonobuoy sensor for testing. Includes three receiver channels for
 * omni, cosine, and sine beams. Also includes single dipole transmit beam.
 */
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

/// @}
}  // namespace test
}  // namespace platforms
}  // namespace usml
