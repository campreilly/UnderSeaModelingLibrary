/**
 * @file simple_sonobuoy.cc
 * Simple sonobuoy sensor for testing.
 */

#include <usml/beampatterns/bp_line.h>
#include <usml/beampatterns/bp_model.h>
#include <usml/beampatterns/bp_omni.h>
#include <usml/beampatterns/bp_trig.h>
#include <usml/platforms/test/simple_sonobuoy.h>

#include <memory>

using namespace usml::platforms;
using namespace usml::beampatterns;

/**
 * Construct sensor with default options.
 */
simple_sonobuoy::simple_sonobuoy(platform_model::key_type keyID,
                                 const std::string& description, time_t time,
                                 const wposition1& pos,
                                 const orientation& orient, double speed)
    : sensor_model(keyID, description, time, pos, orient, speed) {
    src_beam(0, bp_model::csptr(new bp_line(2, 0.75, bp_line_type::VLA)));
    rcv_beam(0, bp_model::csptr(new bp_omni()));
    rcv_beam(1, bp_model::csptr(new bp_trig(bp_trig_type::cosine)));
    rcv_beam(2, bp_model::csptr(new bp_trig(bp_trig_type::sine)));
}
