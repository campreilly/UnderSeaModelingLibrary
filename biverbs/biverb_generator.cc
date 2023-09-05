/**
 * @file biverb_generator.cc
 * Computes reverberation envelopes from eigenverbs.
 */

#include <usml/bistatic/bistatic_pair.h>
#include <usml/biverbs/biverb_collection.h>
#include <usml/biverbs/biverb_generator.h>
#include <usml/eigenverbs/eigenverb_collection.h>
#include <usml/eigenverbs/eigenverb_model.h>
#include <usml/managed/update_notifier.h>
#include <usml/ocean/ocean_model.h>
#include <usml/ocean/ocean_shared.h>
#include <usml/platforms/platform_manager.h>
#include <usml/threads/thread_task.h>
#include <usml/types/seq_vector.h>

#include <boost/numeric/ublas/vector.hpp>
#include <iostream>
#include <memory>

using namespace usml::eigenverbs;
using namespace usml::biverbs;
using namespace usml::ocean;
using namespace usml::platforms;
using namespace usml::types;

#define DEBUG_BIVERB

/**
 * Copies envelope computation parameters from static memory into
 * this specific task.
 */
biverb_generator::biverb_generator(bistatic_pair* pair)
    : _bistatic_pair(pair),
      _src_eigenverbs(pair->src_eigenverbs()),
      _rcv_eigenverbs(pair->rcv_eigenverbs()) {
    add_listener(pair);
}

/**
 * Executes the Eigenverb reverberation model.
 */
void biverb_generator::run() {
    if (_abort) {
        cout << "task #" << id()
             << " biverb_generator *** aborted before execution ***" << endl;
        return;
    }
    cout << "task #" << id()
         << " biverb_generator: src=" << _bistatic_pair->source()->keyID()
         << " rcv=" << _bistatic_pair->receiver()->keyID() << endl;

    // initialize workspace for results

    auto ocean = ocean_shared::current();
    auto freq = platform_manager::instance()->frequencies();
    const size_t num_freq = platform_manager::instance()->frequencies()->size();
    vector<double> scatter(num_freq, 0.0);
    auto* collection = new biverb_collection(ocean->num_volume());

    // loop through eigenverbs for each interface

    auto num_interfaces = _rcv_eigenverbs->num_interfaces();
    for (size_t interface = 0; interface < num_interfaces; ++interface) {
        for (const auto& rcv_verb : _rcv_eigenverbs->eigenverbs(interface)) {
            eigenverb_list found_verbs =
                _src_eigenverbs->find_eigenverbs(rcv_verb, interface);
            for (const auto& src_verb : found_verbs) {
                ocean->scattering(interface, rcv_verb->position,
                                  rcv_verb->frequencies, src_verb->grazing,
                                  rcv_verb->grazing, src_verb->direction,
                                  rcv_verb->direction, &scatter);
                collection->add_biverb(src_verb, rcv_verb, scatter, interface);
                if (_abort) {
                    cout << "task #" << id()
                         << " biverb_generator *** aborted during execution ***"
                         << endl;
                    return;
                }
            }
        }
    }
    _collection = biverb_collection::csptr(collection);
    _done = true;
    notify_update(&_collection);
    cout << "task #" << id() << " biverb_generator: done" << endl;
}