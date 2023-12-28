/**
 * @file sensor_pair.cc
 * Modeling products for a link between source and receiver.
 */

#include <usml/biverbs/biverb_generator.h>
#include <usml/eigenrays/eigenray_model.h>
#include <usml/platforms/platform_model.h>
#include <usml/rvbenv/rvbenv_generator.h>
#include <usml/sensors/sensor_manager.h>
#include <usml/sensors/sensor_pair.h>
#include <usml/threads/thread_controller.h>
#include <usml/threads/thread_pool.h>
#include <usml/threads/thread_task.h>
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
          source->description() + "->" + receiver->description()),
      _source(source),
      _receiver(receiver) {
    _compute_reverb = source->compute_reverb() && receiver->compute_reverb();
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
std::string sensor_pair::generate_hash_key(int src_id, int rcv_id) {
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
 * Notify this pair of eigenray and eigenverb changes for one of its sensors.
 */
void sensor_pair::update_wavefront_data(
    const sensor_model* sensor, eigenray_collection::csptr eigenrays,
    eigenverb_collection::csptr eigenverbs) {
    write_lock_guard guard(_mutex);

    // abort previous biverb generator if it exists

    if (_biverb_task != nullptr) {
        _biverb_task->abort();
    }

    // eigenray collection has eigenray list for all targets near this sensor
    // find the eigenray list specific to this pair

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

    // create new collection with just the rays for a single target

    matrix<int> receiverID(1, 1);
    receiverID(0, 0) = targetID;

    auto* collection = new eigenray_collection(
        eigenrays->frequencies(), _source->position(), wposition(receiver_pos),
        sourceID, receiverID, eigenrays->coherent());
    for (const auto& ray : raylist) {
        collection->add_eigenray(0, 0, ray);
    }
    collection->sum_eigenrays();
    _dirpaths = eigenray_collection::csptr(collection);

    // execute notify_update() early if biverbs never computed

    if (!_compute_reverb) {
        notify_update(this);
        return;
    }

    // update eigenverb contributions

    if (_source == _receiver) {
        _src_eigenverbs = eigenverbs;
        _rcv_eigenverbs = eigenverbs;
    } else if (sensor->keyID() == _receiver->keyID()) {
        _rcv_eigenverbs = eigenverbs;
    } else {
        _src_eigenverbs = eigenverbs;
    }

    // execute notify_update() early if biverbs can't be computed yet

    if (_src_eigenverbs == nullptr || _rcv_eigenverbs == nullptr) {
        notify_update(this);
        return;
    }

    // launch a new bistatic eigenverb generator background task

    eigenverb_collection::csptr src_verbs = _src_eigenverbs;
    eigenverb_collection::csptr rcv_verbs = _rcv_eigenverbs;
    _biverb_task = std::make_shared<biverb_generator>(this,src_verbs,rcv_verbs);
    thread_controller::instance()->run(_biverb_task);
    _biverb_task.reset();  // destroy background task shared pointer
}

/**
 * Update bistatic eigenverbs using results of biverb_generator.
 */
void sensor_pair::notify_update(const biverb_collection::csptr* object) {
    write_lock_guard guard(_mutex);
    _biverbs = *object;

    // abort previous biverb generator if it exists

    if (_rvbenv_task != nullptr) {
    	_rvbenv_task->abort();
    }

    // launch a new reverberation envelope generator background task

    seq_vector::csptr frequencies = sensor_manager::instance()->frequencies();
    seq_vector::csptr travel_times(
        new seq_linear(0.0, _receiver->time_step(), _receiver->time_maximum()));

    _rvbenv_task = std::make_shared<rvbenv_generator>(
        _source, _receiver, _biverbs, frequencies, travel_times);
    thread_controller::instance()->run(_rvbenv_task);
    _rvbenv_task.reset();  // destroy background task shared pointer
//    notify_update(this);
}

/**
 * Update bistatic eigenverbs using results of rvbenv_generator.
 */
void sensor_pair::notify_update(const rvbenv_collection::csptr* object) {
    write_lock_guard guard(_mutex);
    _rvbenv = *object;
    notify_update(this);
}

/**
 * Notify listeners that this sensor_pair has been updated.
 */
void sensor_pair::notify_update(const sensor_pair* object) const {
    this->update_notifier<sensor_pair>::notify_update(object);
}
