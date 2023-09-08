/**
 * @file bistatic_pair.cc
 * Modeling products for a link between source and receiver.
 */

#include <usml/bistatic/bistatic_pair.h>
#include <usml/biverbs/biverb_collection.h>
#include <usml/biverbs/biverb_generator.h>
#include <usml/eigenrays/eigenray_collection.h>
#include <usml/eigenrays/eigenray_model.h>
#include <usml/eigenverbs/eigenverb_collection.h>
#include <usml/managed/managed_obj.h>
#include <usml/managed/update_notifier.h>
#include <usml/platforms/sensor_model.h>
#include <usml/threads/read_write_lock.h>
#include <usml/threads/thread_controller.h>
#include <usml/threads/thread_pool.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition.h>
#include <usml/types/wvector.h>

#include <boost/numeric/ublas/matrix.hpp>
#include <list>
#include <memory>
#include <sstream>
#include <string>

using namespace usml::bistatic;
using namespace usml::biverbs;
using namespace usml::eigenrays;
using namespace usml::eigenverbs;
using namespace usml::managed;
using namespace usml::platforms;
using namespace usml::threads;
using namespace usml::types;

/**
 * Construct link between source and receiver.
 */
bistatic_pair::bistatic_pair(const sensor_model::sptr& source,
                             const sensor_model::sptr& receiver)
    : managed_obj<std::string, bistatic_pair>(
          generate_hash_key(source->keyID(), receiver->keyID()),
          source->description() + "->" + receiver->description()),
      _source(source),
      _receiver(receiver) {
    auto* source_ptr = dynamic_cast<sensor_model*>(_source.get());
    auto* receiver_ptr = dynamic_cast<sensor_model*>(_receiver.get());
    _compute_reverb = source_ptr->compute_reverb() && receiver_ptr->compute_reverb();
    source_ptr->add_wavefront_listener(this);
    receiver_ptr->add_wavefront_listener(this);
}

/**
 * Destroy connections between source and receiver.
 */
bistatic_pair::~bistatic_pair() {
    auto* source_ptr = dynamic_cast<sensor_model*>(_source.get());
    auto* receiver_ptr = dynamic_cast<sensor_model*>(_receiver.get());
    source_ptr->remove_wavefront_listener(this);
    receiver_ptr->remove_wavefront_listener(this);
}

/**
 * Utility to generate a hash key for the bistatic_template
 */
std::string bistatic_pair::generate_hash_key(int src_id, int rcv_id) {
    std::stringstream key;
    key << src_id << '_' << rcv_id;
    return key.str();
}

/**
 * Queries for the bistatic pair for the complement of the given sensor.
 */
sensor_model::sptr bistatic_pair::complement(
    const sensor_model::sptr& sensor) const {
    if (sensor == _source) {
        return _receiver;
    }
    return _source;
}

/**
 * Notify this pair of eigenray and eigenverb changes for one of its sensors.
 */
void bistatic_pair::update_wavefront_data(
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

    if ( _source != _receiver && sensor->keyID() == _receiver->keyID()) {
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

    _biverb_task = std::make_shared<biverb_generator>(this);
    thread_controller::instance()->run(_biverb_task);
    _biverb_task.reset();  // destroy background task shared pointer
}

/**
 * Notify listeners that this bistatic_pair has been updated.
 */
void bistatic_pair::notify_update(const bistatic_pair* object) const {
    this->update_notifier<bistatic_pair>::notify_update(object);
}

/**
 * Update bistatic eigenverbs using results of biverb_generator.
 */
void bistatic_pair::notify_update(const biverb_collection::csptr* object) {
    write_lock_guard guard(_mutex);
    _biverbs = *object;
    notify_update(this);
}
