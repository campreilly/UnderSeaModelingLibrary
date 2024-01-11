/**
 * @file rvbts_generator.cc
 * Background task to compute reverberation time series for a bistatic pair.
 */

#include <usml/managed/managed_obj.h>
#include <usml/platforms/platform_model.h>
#include <usml/rvbts/rvbts_generator.h>
#include <usml/types/bvector.h>
#include <usml/types/seq_linear.h>

#include <boost/numeric/ublas/expression_types.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_expression.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <iostream>
#include <list>

using namespace usml::rvbts;

/**
 * Initialize model parameters with state of sensor_pair at this time.
 */
rvbts_generator::rvbts_generator(const sensor_pair::sptr& pair,
                                 const sensor_model::sptr& source,
                                 const sensor_model::sptr& receiver,
                                 const biverb_collection::csptr& biverbs)
    : _description(pair->description()),
      _source(source),
      _source_pos(source->position()),
      _source_orient(source->orient()),
      _source_speed(source->speed()),
      _transmit_schedule(source->transmit_schedule()),
      _receiver(receiver),
      _receiver_pos(receiver->position()),
      _receiver_orient(receiver->orient()),
      _receiver_speed(receiver->speed()),
      _travel_times(new seq_linear(receiver->time_minimum(),
                                   1.0 / receiver->fsample(),
                                   receiver->time_maximum())),
      _biverbs(biverbs),
      _source_steering(compute_src_steering()) {
    add_listener(pair.get());
}

/**
 * Compute source steerings for each transmit waveform.
 */
matrix<double> rvbts_generator::compute_src_steering() const {
    // compute matrix of ordered steerings relative to host

    matrix<double> steering(3, _transmit_schedule.size());
    int n = 0;
    for (const auto& transmit : _transmit_schedule) {
        bvector ordered(transmit->orderedDE, transmit->orderedAZ);
        steering(0, n) = ordered.front();
        steering(1, n) = ordered.right();
        steering(2, n) = ordered.up();
        ++n;
    }

    // use these steerings if sensor has no host

    const platform_model* host = _source->host();
    if (host == nullptr) {
        return steering;
    }

    // convert steerings to world coordinates using orientation of host

    while (host->host() != nullptr) {
        host = host->host();
    }
    steering = prod(host->orient().rotation(), steering);

    // convert steerings from world to array coordinates

    steering = prod(trans(_source_orient.rotation()), steering);
    return steering;
}

/**
 * Compute reverberation time series for a bistatic pair.
 */
void rvbts_generator::run() {
    if (_abort) {
        cout << "task #" << id()
             << " rvbts_generator: *** aborted before execution ***" << endl;
        return;
    }

    auto* collection =
        new rvbts_collection(_source, _source_pos, _source_orient,
                             _source_speed, _receiver, _receiver_pos,
                             _receiver_orient, _receiver_speed, _travel_times);
    rvbts_collection::csptr result(collection);

    cout << "task #" << id() << " rvbts_generator: " << _description<< endl;

    // loop through eigenverbs for each interface

    auto num_interfaces = _biverbs->num_interfaces();
    for (size_t interface = 0; interface < num_interfaces; ++interface) {
        auto verb_list = _biverbs->biverbs(interface);
        for (const auto& verb : verb_list) {
            int n = 0;
            for (const auto& transmit : _transmit_schedule) {
                bvector steering(
                    matrix_column<matrix<double> >(_source_steering, n));
                collection->add_biverb(verb, transmit, steering);
                if (_abort) {
                    cout << "task #" << id()
                         << " rvbts_generator *** aborted during execution ***"
                         << endl;
                    return;
                }
                ++n;
            }
        }
    }

    // notify listeners of results

    _done = true;
    notify_update(&result);
    cout << "task #" << id() << " rvbts_generator: done" << endl;
}
