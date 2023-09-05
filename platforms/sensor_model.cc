/**
 * @file sensor_model.cc
 * Instance of an individual sensor in the simulation.
 */

#include <usml/eigenrays/eigenray_collection.h>
#include <usml/eigenverbs/eigenverb_collection.h>
#include <usml/managed/manager_template.h>
#include <usml/platforms/motion_thresholds.h>
#include <usml/platforms/platform_manager.h>
#include <usml/platforms/sensor_model.h>
#include <usml/threads/read_write_lock.h>
#include <usml/threads/thread_controller.h>
#include <usml/threads/thread_pool.h>
#include <usml/types/wposition.h>
#include <usml/wavegen/wavefront_generator.h>

#include <algorithm>
#include <boost/numeric/ublas/matrix.hpp>
#include <cfloat>
#include <cmath>
#include <iostream>
#include <memory>
#include <utility>

using namespace usml::platforms;
using namespace usml::wavegen;

/**
 * Add source beam pattern to this sensor.
 */
size_t sensor_model::src_beam(int keyID, bp_model::csptr pattern) {
    write_lock_guard guard(mutex());
    _src_beams[keyID] = std::move(pattern);
    return _src_beams.size();
}

/**
 * Find reference to specific source beam model.
 */
bp_model::csptr sensor_model::src_beam(int keyID) const {
    read_lock_guard guard(mutex());
    bp_model::csptr object = nullptr;
    auto iter = _src_beams.find(keyID);
    if (iter != _src_beams.end()) {
        object = iter->second;
    }
    return object;
}

/**
 * Return a list of all source beam keys.
 */
std::list<int> sensor_model::src_keys() const {
    read_lock_guard guard(mutex());
    std::list<int> list;
    for (const auto& beam : _src_beams) {
        list.push_back(beam.first);
    }
    return list;
}

/**
 * Add receiver beam pattern to this sensor.
 *
 * @param keyID		Identification number.
 * @param pattern   Reference to bp_model.
 */
size_t sensor_model::rcv_beam(int keyID, bp_model::csptr pattern) {
    write_lock_guard guard(mutex());
    _rcv_beams[keyID] = std::move(pattern);
    return _rcv_beams.size();
}

/**
 * Find reference to specific receiver beam model.
 */
bp_model::csptr sensor_model::rcv_beam(int keyID) const {
    read_lock_guard guard(mutex());
    bp_model::csptr object = nullptr;
    auto iter = _rcv_beams.find(keyID);
    if (iter != _rcv_beams.end()) {
        object = iter->second;
    }
    return object;
}

/**
 * Return a list of all source beam keys.
 */
std::list<int> sensor_model::rcv_keys() const {
    read_lock_guard guard(mutex());
    std::list<int> list;
    for (const auto& beam : _rcv_beams) {
        list.push_back(beam.first);
    }
    return list;
}

/**
 * Updates the internal state of this platform and its children.
 */
void sensor_model::update_internals(time_t time, const wposition1& pos,
                                    const orientation& orient, double speed,
                                    update_type_enum update_type) {
    orientation o1 = orient;
    orientation o2 = orientation();

    // clang-format off
    bool update_acoustics =
		update_type != NO_UPDATE && _time_maximum > 0.0 && (
        update_type == FORCE_UPDATE
		|| abs(pos.latitude() - position().latitude()) >= motion_thresholds::lat_threshold
		|| abs(pos.longitude() - position().longitude()) >= motion_thresholds::lon_threshold
		|| abs(pos.altitude() - position().altitude()) >= motion_thresholds::alt_threshold
		|| abs(o1.yaw() - o2.yaw()) >= motion_thresholds::yaw_threshold
		|| abs(o1.pitch() - o2.pitch()) >= motion_thresholds::pitch_threshold
		|| abs(o1.roll() - o2.roll()) >= motion_thresholds::roll_threshold);
    // clang-format on

    // update motion of sensor

    platform_model::update_internals(time, pos, orient, speed, update_type);

    // start wavefront_generator background task to update acoustics

    if (update_acoustics) {
        auto targets = find_targets();

        if (!targets.empty() || _compute_reverb) {
            // abort previous wavefront generator if it exists

            if (_wavefront_task != nullptr) {
                _wavefront_task->abort();
            }

            // launch a new wavefront generator

            wposition tpos(targets.size(), 1);
            matrix<int> targetIDs(targets.size(), 1);

            // count the number of targets
            size_t count = 0;
            for (const auto& target : targets) {
                tpos.latitude(count, 0, target->position().latitude());
                tpos.longitude(count, 0, target->position().longitude());
                tpos.altitude(count, 0, target->position().altitude());
                targetIDs(count, 0) = target->keyID();
                ++count;
            }
            auto frequencies = platform_manager::instance()->frequencies();

            _wavefront_task = std::make_shared<wavefront_generator>(
                this, tpos, targetIDs, frequencies, _de_fan, _az_fan,
                _time_step, _time_maximum, _intensity_threshold, _max_bottom,
                _max_surface);
            thread_controller::instance()->run(_wavefront_task);
            _wavefront_task.reset();  // destroy background task
        }
    }
}

/**
 * Get list of acoustic targets near this sensor.
 */
std::list<platform_model::sptr> sensor_model::find_targets() {
    double min_range2 = _min_range * _min_range;
    double max_range2 = _max_range * _max_range;
    wposition1 object_pos = position();

    std::list<platform_model::sptr> targets;
    for (const auto& platform : platform_manager::instance()->list()) {
        if (min_range2 < DBL_EPSILON && max_range2 < DBL_EPSILON) {
            targets.push_back(platform);
        } else {
            wposition1 platform_pos = platform->position();
            double distance2 = platform_pos.distance2(object_pos);
            if (distance2 >= min_range2) {
                if (max_range2 < DBL_EPSILON || distance2 <= max_range2) {
                    targets.push_back(platform);
                }
            }
        }
    }
    return targets;
}