/**
 * @file sensor_model.h
 * Instance of an individual sensor in the simulation.
 */
#pragma once

#include <bits/types/time_t.h>
#include <usml/beampatterns/bp_model.h>
#include <usml/eigenrays/eigenray_collection.h>
#include <usml/eigenverbs/eigenverb_collection.h>
#include <usml/managed/managed_obj.h>
#include <usml/platforms/platform_model.h>
#include <usml/types/orientation.h>
#include <usml/types/seq_linear.h>
#include <usml/types/seq_rayfan.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition1.h>
#include <usml/usml_config.h>
#include <usml/wavegen/wavefront_listener.h>

#include <cstddef>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>

namespace usml {
namespace platforms {

using namespace usml::beampatterns;
using namespace usml::eigenrays;
using namespace usml::eigenverbs;
using namespace usml::managed;
using namespace usml::types;
using namespace usml::wavegen;

/// @ingroup platforms
/// @{

/**
 * Instance of an individual sensor in the simulation. Simple sensors can be
 * implemented as objects that control their own motion, or they can be attached
 * to host platforms.
 *
 * Stores the parameters needed to invoke the wavefront_generator and stores the
 * results of that invocation. Uses update_notifier to notify listeners when
 * eigenray and eigenverb data has changed. Does not notify listeners when other
 * fields like position and orientation change. Uses mutex to lock queries and
 * updates in multi-threaded environment.
 *
 * The wavefront_generator passes eigenrays and eigenverbs to the
 * sensor_model::notify_wavefront_listeners() method, who then passes it on to
 * all of the registered wavefront_listener objects. Sensor model uses
 * customized methods instead of implementing either the wavefront_listener or
 * wavefront_notifier interfaces to avoid circular dependencies.
 *
 * This class also stores the beampattern models used by this sensor. Each
 * beampattern has a keyID and a const shared pointer to the beampattern model
 * to use. Beampatterns models are immutable and may be shared between sensors.
 * The keyID for source beam patterns identifies the pattern to use for each
 * transmitter operating mode (ex: narrow vs. wide) in the transmit_model. The
 * keyID for receiver beam patterns identifies the receiver channel associated
 * with each pattern. This is particularly useful in beam level simulations
 * where each channel may have a different pattern model.
 */
class USML_DECLSPEC sensor_model : public platform_model {
   public:
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

    /// Minimum range to valid targets (m).
    double min_range() const {
        read_lock_guard guard(_mutex);
        return _min_range;
    }

    /// Minimum range to valid targets (m).
    void min_range(double value) {
        write_lock_guard guard(_mutex);
        _min_range = value;
    }

    /// Maximum range to valid targets (m).  Set to zero for infinite range.
    double max_range() const {
        read_lock_guard guard(_mutex);
        return _max_range;
    }

    /// Maximum range to valid targets (m).  Set to zero for infinite range.
    void max_range(double value) {
        write_lock_guard guard(_mutex);
        _max_range = value;
    }

    /// List of depression/elevation angles to use in wavefront calculation.
    seq_vector::csptr de_fan() const {
        read_lock_guard guard(_mutex);
        return _de_fan;
    }

    /// List of depression/elevation angles to use in wavefront calculation.
    void de_fan(seq_vector::csptr value) {
        write_lock_guard guard(_mutex);
        _de_fan = value;
    }

    /// List of azimuthal angles  to use in wavefront calculation.
    seq_vector::csptr az_fan() const {
        read_lock_guard guard(_mutex);
        return _az_fan;
    }

    /// List of azimuthal angles  to use in wavefront calculation.
    void az_fan(seq_vector::csptr value) {
        write_lock_guard guard(_mutex);
        _az_fan = value;
    }

    /// Time step between wavefronts (sec).
    double time_step() const {
        read_lock_guard guard(_mutex);
        return _time_step;
    }

    /// Time step between wavefronts (sec).
    void time_step(double value) {
        write_lock_guard guard(_mutex);
        _time_step = value;
    }

    /// Maximum time to propagate wavefront (sec).
    double time_maximum() const {
        read_lock_guard guard(_mutex);
        return _time_maximum;
    }

    /// Maximum time to propagate wavefront (sec).
    void time_maximum(double value) {
        write_lock_guard guard(_mutex);
        _time_maximum = value;
    }

    /**
     * The value of the intensity threshold in dB.
     * Any eigenray or eigenverb with an intensity value that are weaker
     * than this threshold is not sent the listeners.
     */
    double intensity_threshold() const {
        read_lock_guard guard(_mutex);
        return _intensity_threshold;
    }

    /// The value of the intensity threshold in dB.
    void intensity_threshold(double value) {
        write_lock_guard guard(_mutex);
        _intensity_threshold = value;
    }

    /**
     * The maximum number of bottom bounces.
     * Any eigenray or eigenverb with more than this number
     * of bottom bounces is not sent the listeners.
     */
    int max_bottom() const {
        read_lock_guard guard(_mutex);
        return _max_bottom;
    }

    /// The maximum number of bottom bounces.
    void max_bottom(int value) {
        write_lock_guard guard(_mutex);
        _max_bottom = value;
    }

    /**
     * The maximum number of surface bounces.
     * Any eigenray or eigenverb with more than this number
     * of surface bounces is not sent the listeners.
     */
    int max_surface() const {
        read_lock_guard guard(_mutex);
        return _max_surface;
    }

    /// The maximum number of surface bounces.
    void max_surface(int value) {
        write_lock_guard guard(_mutex);
        _max_surface = value;
    }

    /// True if eigenverbs computed for this sensor.
    bool compute_reverb() const {
        read_lock_guard guard(_mutex);
        return _compute_reverb;
    }

    /// True if eigenverbs computed for this sensor.
    void compute_reverb(bool value) {
        write_lock_guard guard(_mutex);
        _compute_reverb = value;
    }

    /// True if platform can receive transmissions from other platforms.
    bool multistatic() const {
        read_lock_guard guard(_mutex);
        return _multistatic;
    }

    /// True if platform can receive transmissions from other platforms.
    void multistatic(bool value) {
        write_lock_guard guard(_mutex);
        _multistatic = value;
    }

    /**
     * Add source beam pattern to this sensor.
     *
     * @param keyID		Identification number.
     * @param pattern   Reference to bp_model.
     */
    size_t src_beam(int keyID, bp_model::csptr pattern);

    /**
     * Find reference to specific source beam model.
     *
     * @param keyID		Identification number.
     */
    bp_model::csptr src_beam(int keyID) const;

    /**
     * Return a list of all source beam keys.
     */
    std::list<int> src_keys() const;

    /**
     * Indicate if this sensor can act as a source.
     *
     * @return True if the sensor has source beams.
     */
    bool is_source() const {
        read_lock_guard guard(_mutex);
        return _src_beams.size() > 0;
    }

    /**
     * Add receiver beam pattern to this sensor.
     *
     * @param keyID		Identification number.
     * @param pattern   Reference to bp_model.
     */
    size_t rcv_beam(int keyID, bp_model::csptr pattern);

    /**
     * Find reference to specific receiver beam model.
     *
     * @param keyID		Identification number.
     */
    bp_model::csptr rcv_beam(int keyID) const;

    /**
     * Return a list of all receiver beam keys.
     */
    std::list<int> rcv_keys() const;

    /**
     * Indicate if this sensor can act as a receiver.
     *
     * @return True if the sensor has receiver beams.
     */
    bool is_receiver() const {
        read_lock_guard guard(_mutex);
        return _rcv_beams.size() > 0;
    }

    /**
     * Add a wavefront listener to this object.
     */
    void add_wavefront_listener(usml::wavegen::wavefront_listener* listener) {
        _listeners.insert(listener);
    }

    /**
     * Remove a wavefront listener to this object.
     */
    void remove_wavefront_listener(
        usml::wavegen::wavefront_listener* listener) {
        _listeners.erase(listener);
    }

    /**
     * Distribute updates from wavefront_generator to all listeners.
     */
    void notify_wavefront_listeners(
        const eigenray_collection::csptr& eigenrays,
        const eigenverb_collection::csptr& eigenverbs);

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

    /// Iterator used to search for specific objects.
    typedef typename beam_map_type::const_iterator beam_iterator;

    /// Minimum range to valid targets (m).
    double _min_range{0.0};

    /// Maximum range to valid targets (m).  Use zero for infinite range.
    double _max_range{0.0};

    /// List of depression/elevation angles to use in wavefront calculation.
    seq_vector::csptr _de_fan{seq_vector::csptr(new seq_rayfan())};

    /// List of azimuthal angles  to use in wavefront calculation.
    seq_vector::csptr _az_fan{
        seq_vector::csptr(new seq_linear(0.0, 10.0, 360.0))};

    /// Time step between wavefronts (sec).
    double _time_step{0.1};

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

    /// True if sensor can receive transmissions from other sensors.
    bool _multistatic{false};

    /// Source beam patterns
    beam_map_type _src_beams;

    /// Receiver beam patterns
    beam_map_type _rcv_beams;

    /// List of active wavefront listeners.
    std::set<wavefront_listener*> _listeners;
};

/// @}
}  // namespace platforms
}  // namespace usml
