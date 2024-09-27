/**
 * @file sensor_pair.cc
 * Modeling products for a link between source and receiver.
 */

#include <usml/biverbs/biverb_generator.h>
#include <usml/eigenrays/eigenray_model.h>
#include <usml/managed/manager_template.h>
#include <usml/platforms/platform_model.h>
#include <usml/rvbts/rvbts_generator.h>
#include <usml/sensors/sensor_manager.h>
#include <usml/sensors/sensor_pair.h>
#include <usml/threads/thread_controller.h>
#include <usml/threads/thread_pool.h>
#include <usml/threads/thread_task.h>
#include <usml/types/seq_linear.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition.h>
#include <usml/types/wposition1.h>

#include <boost/numeric/ublas/matrix.hpp>
#include <sstream>

using namespace usml::sensors;

/**
 * Construct link between source and receiver.
 */
sensor_pair::sensor_pair(const sensor_model::sptr& source,
                         const sensor_model::sptr& receiver)
    : managed_obj<std::string, sensor_pair>(
          generate_hash_key(source->keyID(), receiver->keyID()),
          source->description() + " -> " + receiver->description()),
      _source(source),
      _receiver(receiver),
      _compute_reverb(source->compute_reverb() && receiver->compute_reverb()) {
    source->set_needs_update();
    receiver->set_needs_update();
    source->add_wavefront_listener(this);
    receiver->add_wavefront_listener(this);
}

/**
 * Destroy connections between source and receiver.
 */
sensor_pair::~sensor_pair() {
    source()->remove_wavefront_listener(this);
    receiver()->remove_wavefront_listener(this);
}

/**
 * Utility to generate a hash key for the bistatic_template
 */
std::string sensor_pair::generate_hash_key(uint64_t src_id, uint64_t rcv_id) {
    std::stringstream key;
    key << src_id << '_' << rcv_id;
    return key.str();
}

/**
 * Queries for the bistatic pair for the complement of the given sensor.
 */
sensor_model::sptr sensor_pair::complement(
    const sensor_model::sptr& sensor) const {
    read_lock_guard guard(_mutex);
    if (sensor == _source) {
        return _receiver;
    }
    return _source;
}

/**
 * Update eigenrays and eigenverbs using results of the wavefront_generator
 * background task.
 */
void sensor_pair::update_wavefront_data(
    const sensor_model* sensor, eigenray_collection::csptr eigenrays,
    eigenverb_collection::csptr eigenverbs) {
    bool notify_early{true};
    {
        write_lock_guard guard(_mutex);

        // eigenray collection has eigenray list for all targets near this
        // sensor find the eigenray list specific to this pair

        auto sourceID = _source->keyID();
        auto targetID = _receiver->keyID();
        wposition1 source_pos(_source->position());
        wposition1 receiver_pos(_receiver->position());
        eigenray_list raylist = eigenrays->find_eigenrays(targetID);

        // swap source/receiver sense of direct path eigenrays, if needed

        if (_source != _receiver && sensor->keyID() == _receiver->keyID()) {
            sourceID = _receiver->keyID();
            targetID = _source->keyID();
            source_pos = _receiver->position();
            receiver_pos = _source->position();
            eigenray_list new_list;
            for (const auto& ray : eigenrays->find_eigenrays(targetID)) {
                auto* copy = new eigenray_model(*ray);
                std::swap(copy->source_de, copy->target_de);
                std::swap(copy->source_az, copy->target_az);
                new_list.push_back(eigenray_model::csptr(copy));
            }
            raylist = new_list;  // replace original list
        }

        // create new direct path collection with just rays for a single target

        matrix<uint64_t> receiverID(1, 1);
        receiverID(0, 0) = targetID;

        auto* collection = new eigenray_collection(
            eigenrays->frequencies(), _source->position(),
            wposition(receiver_pos), sourceID, receiverID,
            eigenrays->coherent());
        for (const auto& ray : raylist) {
            collection->add_eigenray(0, 0, ray);
        }
        collection->sum_eigenrays();
        _dirpaths = eigenray_collection::csptr(collection);

        // update eigenverb contributions

        if (_compute_reverb) {
            notify_early = false;
            if (_source == _receiver) {
                _src_eigenverbs = eigenverbs;
                _rcv_eigenverbs = eigenverbs;
            } else if (sensor->keyID() == _receiver->keyID()) {
                _rcv_eigenverbs = eigenverbs;
            } else {
                _src_eigenverbs = eigenverbs;
            }

            // launch a new bistatic eigenverb generator background task

            if (_src_eigenverbs != nullptr && _rcv_eigenverbs != nullptr) {
                if (_biverb_task != nullptr) {  // abort incomplete tasks
                    _biverb_task->abort();
                }
                sensor_pair::sptr reference =
                    sensor_manager::instance()->find(keyID());
                eigenverb_collection::csptr src_verbs = _src_eigenverbs;
                eigenverb_collection::csptr rcv_verbs = _rcv_eigenverbs;
                _biverb_task = std::make_shared<biverb_generator>(
                    reference, src_verbs, rcv_verbs);
                thread_controller::instance()->run(_biverb_task);
                _biverb_task.reset();  // destroy background task shared pointer
            }
        }
    }
    if (notify_early) {
        notify_update(this);
    }
}

/**
 * Update bistatic eigenverbs using results of the biverb_generator
 * background task.
 */
void sensor_pair::notify_update(const biverb_collection::csptr* object) {
    bool notify_early{true};
    {
        write_lock_guard guard(_mutex);
        _biverbs = *object;
        if (_source->transmit_schedule().empty()) {
            return;
        }

        // compute treverb from transmit schedule

        const double treverb_min = 0.1;
        double treverb = 0.0;
        for (const auto& transmit : _source->transmit_schedule()) {
            if (treverb == 0.0) {
                treverb = transmit->duration;
            } else {
                treverb = std::min(treverb, transmit->duration);
            }
        }
        treverb = std::max(treverb_min, treverb / 2.0);

        // launch a new reverberation time series generator background task

        notify_early = false;
        if (_rvbts_task != nullptr) {  // abort incomplete tasks
            _rvbts_task->abort();
        }
        sensor_pair::sptr reference = sensor_manager::instance()->find(keyID());
        _rvbts_task = std::make_shared<rvbts_generator>(
            reference, _source, _receiver, treverb, _biverbs);
        thread_controller::instance()->run(_rvbts_task);
        _rvbts_task.reset();  // destroy background task shared pointer
    }
    if (notify_early) {
        notify_update(this);
    }
}

/**
 * Update reverberation time series using results of the rvbts_generator
 * background task.
 */
void sensor_pair::notify_update(const rvbts_collection::csptr* object) {
    {
        write_lock_guard guard(_mutex);
        _rvbts = *object;
    }
    notify_update(this);
}

/**
 * Notify listeners that acoustic data for this sensor_pair has been updated.
 */
void sensor_pair::notify_update(const sensor_pair* object) const {
    this->update_notifier<sensor_pair>::notify_update(object);
}
