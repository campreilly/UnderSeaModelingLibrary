/**
 * @file sensor_model.h
 * Sensors are platforms that can automatically compute their own acoustics.
 */
#pragma once

#include <bits/types/time_t.h>
#include <usml/beampatterns/bp_model.h>
#include <usml/managed/managed_obj.h>
#include <usml/platforms/platform_model.h>
#include <usml/threads/read_write_lock.h>
#include <usml/transmit/transmit_model.h>
#include <usml/types/bvector.h>
#include <usml/types/orientation.h>
#include <usml/types/seq_linear.h>
#include <usml/types/seq_rayfan.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition1.h>
#include <usml/usml_config.h>
#include <usml/wavegen/wavefront_notifier.h>

#include <cstddef>
#include <list>
#include <map>
#include <memory>
#include <string>

namespace usml {
namespace wavegen {
class wavefront_generator;
} /* namespace wavegen */
} /* namespace usml */

namespace usml {
namespace sensors {

using namespace usml::beampatterns;
using namespace usml::managed;
using namespace usml::platforms;
using namespace usml::threads;
using namespace usml::transmit;
using namespace usml::types;
using namespace usml::wavegen;

/// @ingroup sensors
/// @{

/**
 * Sensors are platforms that can automatically compute their own acoustics.
 * Simple sensors can be implemented as objects that control their own motion,
 * or they can be attached to host platforms using the add_child() method. Uses
 * mutex to lock queries and updates in multi-threaded environment.
 *
 * This class also stores the beampattern models used by this sensor. Each
 * beampattern has a keyID and a const shared pointer to the beampattern model
 * to use. Beampatterns models are immutable and may be shared between sensors.
 * The transmission schedule used to generate acoustic time series and it
 * includes pulse characteristics, a transmit keyID, and a transmit steering
 * direction for each pulse. The keyID for of the source beam patterns
 * identifies the beam pattern model to use for each of these pulses. The keyID
 * for each receiver beam patterns identifies the receiver channel associated
 * with each pattern. This implementation currently supports beam level
 * simulations where each channel has its own beam pattern and steering. Time
 * series results are generated as a function of receiver channel number and
 * time.
 *
 * Automatically launches a background task to recompute eigenrays and
 * eigenverbs when sensor motion exceeds position or orientation thresholds.
 * The find_targets() calculation searches the platform_manager for all
 * platforms and sensors sensors between the maximum and minimum slant range. If
 * an existing wavefront_generator is running for this sensor, that task is
 * aborted before the new background task is created. Uses update_notifier to
 * notify listeners when eigenray and eigenverb data has changed. Does not
 * notify listeners when other fields like position and orientation change.
 */
class USML_DECLSPEC sensor_model : public platform_model,
                                   public wavefront_notifier {
   public:
    /**
     * Form of the shared pointer that supports access to sensor_model
     * attributes and methods.
     */
    using sptr = std::shared_ptr<sensor_model>;

    /**
     * Initialize location and orientation of the sensor in world coordinates.
     *
     * @param keyID        	Identification used to find this platform
     * 						instance in platform_manager.
     * @param description   Human readable name for this platform instance.
     * @param time			Time at which platform is being created.
     * @param pos 			Initial location for this platform.
     * @param orient 		Initial orientation for this platform.
     * @param speed			Platform speed (m/s).
     */
    sensor_model(platform_model::key_type keyID, const std::string& description,
                 time_t time = 0.0, const wposition1& pos = wposition1(),
                 const orientation& orient = orientation(), double speed = 0.0)
        : platform_model(keyID, description, time, pos, orient, speed) {}

    /// Minimum range to find valid targets (m).
    double min_range() const { return _min_range; }

    /// Minimum range to find valid targets (m).
    void min_range(double value) { _min_range = value; }

    /// Maximum range to find valid targets (m).  Set to zero for infinite
    /// range.
    double max_range() const { return _max_range; }

    /// Maximum range to find valid targets (m).  Set to zero for infinite
    /// range.
    void max_range(double value) { _max_range = value; }

    /// List of depression/elevation angles to use in wavefront calculation.
    seq_vector::csptr de_fan() const { return _de_fan; }

    /// List of depression/elevation angles to use in wavefront calculation.
    void de_fan(seq_vector::csptr value) { _de_fan = value; }

    /// List of azimuthal angles  to use in wavefront calculation.
    seq_vector::csptr az_fan() const { return _az_fan; }

    /// List of azimuthal angles  to use in wavefront calculation.
    void az_fan(seq_vector::csptr value) { _az_fan = value; }

    /// Time step between wavefronts (sec).
    double time_step() const { return _time_step; }

    /// Time step between wavefronts (sec).
    void time_step(double value) { _time_step = value; }

    /// Maximum time to propagate wavefront (sec).
    double time_minimum() const { return _time_minimum; }

    /// Maximum time to propagate wavefront (sec).
    void time_minimum(double value) { _time_minimum = value; }

    /// Maximum time to propagate wavefront (sec).
    double time_maximum() const { return _time_maximum; }

    /// Maximum time to propagate wavefront (sec).
    void time_maximum(double value) { _time_maximum = value; }

    /**
     * The value of the intensity threshold in dB. Any eigenray or eigenverb
     * with an intensity value that are weaker than this threshold is not sent
     * the listeners.
     */
    double intensity_threshold() const { return _intensity_threshold; }

    /// The value of the intensity threshold in dB.
    void intensity_threshold(double value) { _intensity_threshold = value; }

    /**
     * The maximum number of bottom bounces.
     * Any eigenray or eigenverb with more than this number
     * of bottom bounces is not sent the listeners.
     */
    int max_bottom() const { return _max_bottom; }

    /// The maximum number of bottom bounces.
    void max_bottom(int value) { _max_bottom = value; }

    /**
     * The maximum number of surface bounces.
     * Any eigenray or eigenverb with more than this number
     * of surface bounces is not sent the listeners.
     */
    int max_surface() const { return _max_surface; }

    /// The maximum number of surface bounces.
    void max_surface(int value) { _max_surface = value; }

    /// True if eigenverbs computed for this sensor.
    bool compute_reverb() const { return _compute_reverb; }

    /// True if eigenverbs computed for this sensor.
    void compute_reverb(bool value) { _compute_reverb = value; }

    /**
     * Multi-static group for this sensor (0=none). The sensor_manager
     * automatically creates bistatic pairs for sources and receivers in the
     * same multistatic group.
     */
    int multistatic() const { return _multistatic; }

    /// Multi-static group for this sensor (0=none).
    void multistatic(int value) { _multistatic = value; }

    /// Add transmit mode beam pattern to this sensor.
    size_t src_beam(int keyID, const bp_model::csptr& pattern);

    /// Find reference to specific source beam model.
    bp_model::csptr src_beam(int keyID) const;

    /// Return a list of all source beam keys.
    std::list<int> src_keys() const;

    /// True if the sensor has source beams.
    bool is_source() const {
        read_lock_guard guard(mutex());
        return _src_beams.size() > 0;
    }

    /// Add receiver beam pattern to this sensor.
    size_t rcv_beam(int keyID, const bp_model::csptr& pattern,
                    const bvector& steering = bvector(1.0, 0.0, 0.0));

    ///  Find reference to specific receiver beam model.
    bp_model::csptr rcv_beam(int keyID) const;

    /// Retrieve source steering for specific channel number.
    bvector rcv_steering(int keyID) const;

    /// Update source steering for specific channel number.
    void rcv_steering(int keyID, const bvector& steering);

    /// Return a list of all receiver beam keys.
    std::list<int> rcv_keys() const;

    /// True if the sensor has receiver beams.
    bool is_receiver() const {
        read_lock_guard guard(mutex());
        return _rcv_beams.size() > 0;
    }

    /// Retrieve receiver sampling rate (Hz).
    double fsample() const { return _fsample; }

    /// Update receiver sampling rate (Hz).
    void fsample(double value) { _fsample = value; }

    /// Retrieve receiver center frequency (Hz).
    double fcenter() const { return _fcenter; }

    /// Update receiver center frequency (Hz).
    void fcenter(double value) { _fcenter = value; }

    /// List of pulses to transmit.
    transmit_list transmit_schedule() const {
        read_lock_guard guard(mutex());
        return _transmit_schedule;
    }

    /// List of pulses to transmit.
    /// TODO Recompute reverberation time series when transmit schedule changes.
    void transmit_schedule(const transmit_list& schedule) {
        write_lock_guard guard(mutex());
        _transmit_schedule = schedule;
    }

   protected:
    /**
     * Updates the internal state of this platform and its children. Starts
     * wavefront_generator background task to update acoustics if sensor has
     * moved by moved by more than the thresholds defined in motion_thresholds
     * class. Acoustics not computed if sensor has time_maximum set to zero.
     * Acoustics not computed if there are no eigenrays or eigenverbs to be
     * computed.
     *
     * @param time          Time at which platform was updated.
     * @param pos           New location for this platform.
     * @param orient        New orientation for this platform.
     * @param speed         Platform speed (m/s).
     * @param update_type	Controls testing of thresholds.
     */
    virtual void update_internals(
        time_t time, const wposition1& pos,
        const orientation& orient = orientation(), double speed = 0.0,
        update_type_enum update_type = TEST_THRESHOLD);

    /**
     * Get list of acoustic targets near this sensor.
     */
    std::list<platform_model::sptr> find_targets();

   private:
    /// Type used to store list of objects.
    typedef std::map<int, bp_model::csptr> beam_map_type;

    /// Type used to store list of beam steerings.
    typedef std::map<int, bvector> steering_map_type;

    /// Minimum range to find valid targets (m).
    double _min_range{0.0};

    /// Maximum range to find valid targets (m).  Use zero for infinite range.
    double _max_range{0.0};

    /// List of depression/elevation angles to use in wavefront calculation.
    seq_vector::csptr _de_fan{seq_vector::csptr(new seq_rayfan())};

    /// List of azimuthal angles  to use in wavefront calculation.
    seq_vector::csptr _az_fan{
        seq_vector::csptr(new seq_linear(0.0, 10.0, 360.0))};

    /// Time step between wavefronts (sec).
    double _time_step{0.1};

    /// Minimum time to compute wavefront results (sec).
    double _time_minimum{0.0};

    /// Maximum time to propagate wavefront (sec).
    double _time_maximum{0.0};

    /**
     * The value of the intensity threshold in dB.
     * Any eigenray or eigenverb with an intensity value that are weaker
     * than this threshold is not sent the listeners.
     */
    double _intensity_threshold{-300.0};

    /**
     * The maximum number of bottom bounces.
     * Any eigenray or eigenverb with more than this number
     * of bottom bounces is not sent the listeners.
     */
    int _max_bottom{999};

    /**
     * The maximum number of surface bounces.
     * Any eigenray or eigenverb with more than this number
     * of surface bounces is not sent the listeners.
     */
    int _max_surface{999};

    /// True if computing reverberation from this sensor.
    bool _compute_reverb{false};

    /// Multi-static group for this sensor (0=none).
    int _multistatic{0};

    /// Source beam patterns.
    beam_map_type _src_beams;

    /// Receiver beam patterns.
    beam_map_type _rcv_beams;

    /// Receiver beam steerings.
    steering_map_type _rcv_steering;

    /// Receiver sampling rate (Hz).
    double _fsample{0.0};

    /// Receiver center frequency (Hz).
    double _fcenter{0.0};

    /// List of pulses to transmit.
    transmit_list _transmit_schedule;

    /// Reference to currently executing wavefront generator.
    std::shared_ptr<wavefront_generator> _wavefront_task;
};

/// @}
}  // namespace sensors
}  // namespace usml
