/**
 * @file rvbts_generator.cc
 * Computes reverberation envelopes from eigenverbs.
 */

#include <usml/rvbts/rvbts_generator.h>

using namespace usml::rvbts;

/**
 * Initialize model parameters with state of sensor_pair at this time.
 */
rvbts_generator::rvbts_generator(const sensor_model::sptr& source,
                                   const sensor_model::sptr& receiver,
                                   const biverb_collection::csptr& biverbs,
                                   const seq_vector::csptr& frequencies,
                                   const seq_vector::csptr& travel_times)
    : _source(source),
      _receiver(receiver),
      _biverbs(biverbs),
      _frequencies(frequencies),
      _travel_times(travel_times) {}

/**
 * Compute reverberation envelope collection for a bistatic pair.
 */
void rvbts_generator::run() {
    if (_abort) {
        cout << "task #" << id()
             << " rvbts_generator *** aborted before execution ***" << endl;
        return;
    }
    cout << "task #" << id() << " rvbts_generator src=" << _source->keyID()
         << " rcv=" << _receiver->keyID() << endl;

    // initialize workspace for results

    auto* collection =
        new rvbts_collection(_source, _receiver, _travel_times, _frequencies);
    const auto num_freqs = _frequencies->size();
    const auto num_src_beams = collection->num_src_beams();
    const auto num_rcv_beams = collection->num_rcv_beams();

    vector<double> beam_work(num_freqs, 1.0);
    matrix<double> src_beam(num_freqs, num_src_beams, 1.0);
    matrix<double> rcv_beam(num_freqs, num_rcv_beams, 1.0);

    // loop through eigenverbs for each interface

    auto num_interfaces = _biverbs->num_interfaces();
    for (size_t interface = 0; interface < num_interfaces; ++interface) {
        for (const auto& verb : _biverbs->biverbs(interface)) {
            beam_gain_src(collection, verb->source_de, verb->source_az,
                          beam_work, src_beam);
            beam_gain_rcv(collection, verb->receiver_de, verb->receiver_az,
                          beam_work, rcv_beam);
            collection->add_biverb(verb, src_beam, rcv_beam);
            if (_abort) {
                cout << "task #" << id()
                     << " rvbts_generator *** aborted during execution ***"
                     << endl;
                return;
            }
        }
    }

    // notify listeners of results

    rvbts_collection::csptr result(collection);
    _done = true;
    notify_update(&result);
    cout << "task #" << id() << " rvbts_generator: done" << endl;
}

/**
 * Computes the source beam gain as a function of DE and AZ for each frequency
 * and beam number.
 */
void rvbts_generator::beam_gain_src(const rvbts_collection* collection,
                                     double de, double az,
                                     vector<double>& beam_work,
                                     matrix<double>& beam) {
    sensor_model::sptr source = collection->source();
    bvector arrival(de, az);
    arrival.rotate(collection->source_orient(), arrival);
    int beam_number = 0;
    for (auto keyID : source->src_keys()) {
        bp_model::csptr bp = source->src_beam(keyID);
        bp->beam_level(arrival, _frequencies, &beam_work);
        matrix_column<matrix<double> > col(beam, beam_number);
        col = beam_work;
        ++beam_number;
    }
}

/**
 * Computes the source beam gain as a function of DE and AZ for each frequency
 * and beam number.
 */
void rvbts_generator::beam_gain_rcv(const rvbts_collection* collection,
                                     double de, double az,
                                     vector<double>& beam_work,
                                     matrix<double>& beam) {
    sensor_model::sptr receiver = collection->receiver();
    bvector arrival(de, az);
    arrival.rotate(collection->receiver_orient(), arrival);
    int beam_number = 0;
    for (auto keyID : receiver->rcv_keys()) {
        bp_model::csptr bp = receiver->rcv_beam(keyID);
        bvector steering = receiver->rcv_steering(keyID);
        bp->beam_level(arrival, _frequencies, &beam_work, steering);
        matrix_column<matrix<double> > col(beam, beam_number);
        col = beam_work;
        ++beam_number;
    }
}
