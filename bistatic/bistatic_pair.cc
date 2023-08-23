/**
 * @file bistatic_pair.cc
 * Modeling products for a link between source and receiver.
 */

#include <usml/bistatic/bistatic_pair.h>
#include <usml/biverbs/biverb_generator.h>
#include <usml/eigenrays/eigenray_collection.h>
#include <usml/eigenrays/eigenray_model.h>
#include <usml/eigenverbs/eigenverb_collection.h>
#include <usml/managed/managed_obj.h>
#include <usml/managed/update_notifier.h>
#include <usml/platforms/platform_model.h>
#include <usml/platforms/sensor_model.h>
#include <usml/rvbenv/rvbenv_generator.h>
#include <usml/threads/read_write_lock.h>
#include <usml/threads/thread_controller.h>
#include <usml/threads/thread_pool.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition.h>
#include <usml/types/wposition1.h>
#include <usml/types/wvector.h>
#include <usml/types/wvector1.h>

#include <boost/numeric/ublas/matrix.hpp>
#include <list>
#include <memory>
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
    bool is_receiver = sensor == _receiver.get() && _source != _receiver;

    // eigenray collection has eigenray list for all targets near this sensor
    // find the eigenray list specific to this pair

    auto targetID = _receiver->keyID();
    if (is_receiver) {
        targetID = _source->keyID();
    }
    auto raylist = eigenrays->find_eigenrays(targetID);

    // swap source/receiver sense of eigenrays, if needed

    if (is_receiver) {
        eigenray_list new_list;
        for (const auto& ray : raylist) {
            auto* copy = new eigenray_model(*ray);
            std::swap(copy->source_de, copy->target_de);
            std::swap(copy->source_az, copy->target_az);
            new_list.push_back(eigenray_model::csptr(copy));
        }
        raylist = new_list;  // replace original list
    }

    // create new collection with just the rays for a single target

    wposition rcv_position(1, 1);
    rcv_position.rho(0, 0, _receiver->position().rho());
    rcv_position.theta(0, 0, _receiver->position().theta());
    rcv_position.phi(0, 0, _receiver->position().phi());

    matrix<platform_model::key_type> rcv_id(1, 1);
    rcv_id(0, 0) = _receiver->keyID();

    auto* collection =
        new eigenray_collection(eigenrays->frequencies(), _source->position(),
                                &rcv_position, sensor->keyID(), rcv_id);
    for (const auto& ray : raylist) {
        collection->add_eigenray(0, 0, ray);
    }

    _dirpaths = eigenray_collection::csptr(collection);

    // update eigenverb contributions

    if (_source == _receiver) {
        _rcv_eigenverbs = eigenverbs;
        _src_eigenverbs = _rcv_eigenverbs;
    } else if (is_receiver) {
        _rcv_eigenverbs = eigenverbs;
    } else {
        _src_eigenverbs = eigenverbs;
    }

    // compute bistatic eigenverbs if source and receiver eigenverbs available

    if (_src_eigenverbs != nullptr && _rcv_eigenverbs != nullptr) {
        std::shared_ptr<biverb_generator> task(
            new biverb_generator(this, sensor->az_fan()->size()));
        thread_controller::instance()->run(task);
    }

    // notify bitatic_pair listeners of updates

    notify_update(this);
}
