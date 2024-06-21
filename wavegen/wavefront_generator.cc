/**
 * @file wavefront_generator.cc
 * Generates eigenrays and eigenverbs for the reverberation model.
 */

#include <usml/eigenrays/eigenray_collection.h>
#include <usml/eigenverbs/eigenverb_collection.h>
#include <usml/managed/managed_obj.h>
#include <usml/ocean/ocean_shared.h>
#include <usml/platforms/platform_model.h>
#include <usml/sensors/sensor_model.h>
#include <usml/wavegen/wavefront_generator.h>
#include <usml/waveq3d/wave_queue.h>
#include <usml/waveq3d/wave_thresholds.h>

#include <iostream>
#include <memory>
#include <string>

using namespace usml::wavegen;
using namespace usml::waveq3d;

/**
 * Automatically recomputes acoustic data when platform motion exceeds position
 * or orientation thresholds.
 */
wavefront_generator::wavefront_generator(
    sensor_model* source, const wposition& target_positions,
    const matrix<uint64_t>& targetIDs, const seq_vector::csptr& frequencies,
    const seq_vector::csptr& de_fan, const seq_vector::csptr& az_fan,
    double time_step, double time_maximum, double intensity_threshold,
    int max_bottom, int max_surface)
    : _ocean(ocean_shared::current()),
      _source(source),
      _source_position(source->position()),
      _target_positions(target_positions),
      _targetIDs(targetIDs),
      _frequencies(frequencies),
      _de_fan(de_fan),
      _az_fan(az_fan),
      _time_step(time_step),
      _time_maximum(time_maximum),
      _intensity_threshold(intensity_threshold),
      _max_bottom(max_bottom),
      _max_surface(max_surface) {}

/**
 * Executes the WaveQ3D propagation model.
 */
void wavefront_generator::run() {
    // check to see if task has already been aborted

    if (_abort) {
        cout << "task #" << id()
             << " wavefront_generator *** aborted before execution ***" << endl;
        return;
    }

    // create a new wavefront

    cout << "task #" << id()
         << " wavefront_generator: " << _source->description() << " for "
         << _time_maximum << " secs" << endl;
    wave_queue wave(_ocean, _frequencies, _source_position, _de_fan, _az_fan,
                    _time_step, &_target_positions);
    wave.intensity_threshold(_intensity_threshold);
    wave.max_bottom(_max_bottom);
    wave.max_surface(_max_surface);

    // create listener to store eigenrays, if targets exist

    auto* eigenrays = new eigenray_collection(_frequencies, _source_position,
                                              _target_positions,
                                              _source->keyID(), _targetIDs);
    if (_targetIDs.size1() > 0 && _targetIDs.size2() > 0) {
        wave.add_eigenray_listener(eigenrays);
    }

    // create listener to store eigenverbs

    auto* eigenverbs = new eigenverb_collection(_ocean->num_volume());
    if (_source->compute_reverb()) {
        wave.add_eigenverb_listener(eigenverbs);
    }

    // propagate wavefront to build eigenrays and eigenverbs

    while (wave.time() < _time_maximum) {
        wave.step();
        if (_abort) {
            cout << "task #" << id()
                 << " wavefront_generator *** aborted during execution ***"
                 << endl;
            return;
        }
    }
    if (eigenrays != nullptr) {
        eigenrays->sum_eigenrays();
    }

    // distribute eigenrays and eigenverbs to listeners

    _done = true;
    _source->notify_wavefront_listeners(
        _source, eigenray_collection::csptr(eigenrays),
        eigenverb_collection::csptr(eigenverbs));
    cout << "task #" << id() << " wavefront_generator: done" << endl;
}
