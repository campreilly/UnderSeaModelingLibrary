/**
 * @file rvbenv_generator.cc
 * Computes reverberation envelopes from eigenverbs.
 */

#include <usml/rvbenv/rvbenv_generator.h>

using namespace usml::rvbenv;

/**
 * Initialize model parameters with state of sensor_pair at this time.
 */
rvbenv_generator::rvbenv_generator(const sensor_pair::sptr& pair,
                                   seq_vector::csptr times,
                                   seq_vector::csptr freqs, size_t num_azimuths)
    : _sensor_pair(pair),
      _times(std::move(times)),
      _freqs(std::move(freqs)),
      _num_azimuths(num_azimuths) {}

/**
 * Compute reverberation envelope collection for a bistatic pair.
 */
void rvbenv_generator::run() {
    if (_abort) {
        cout << "task #" << id()
             << " rvbenv_generator *** aborted before execution ***" << endl;
        return;
    }
    cout << "task #" << id()
         << " rvbenv_generator *** aborted before execution ***" << endl;
    cout << "task #" << id()
         << " rvbenv_generator src=" << _sensor_pair->source()->keyID()
         << " rcv=" << _sensor_pair->receiver()->keyID() << endl;

    // initialize workspace for results

    auto* collection =
        new rvbenv_collection(_sensor_pair, _times, _freqs, _num_azimuths);
    const auto num_freqs = _freqs->size();
    const auto num_src_beams = collection->num_src_beams();
    const auto num_rcv_beams = collection->num_rcv_beams();

    vector<double> beam_work(num_freqs, 1.0);
    matrix<double> src_beam(num_freqs, num_src_beams, 1.0);
    matrix<double> rcv_beam(num_freqs, num_rcv_beams, 1.0);

    // loop through eigenverbs for each interface

    auto num_interfaces = collection->biverbs()->num_interfaces();
    for (size_t interface = 0; interface < num_interfaces; ++interface) {
        for (const auto& verb : collection->biverbs()->biverbs(interface)) {
            beam_gain_src(collection, verb->source_de, verb->source_az,
                          beam_work, src_beam);
            beam_gain_rcv(collection, verb->receiver_de, verb->receiver_az,
                          beam_work, rcv_beam);
            collection->add_biverb(verb, src_beam, rcv_beam);
            if (_abort) {
                cout << "task #" << id()
                     << " rvbenv_generator *** aborted during execution ***"
                     << endl;
                return;
            }
        }
    }

    // notify listeners of results

    _collection = rvbenv_collection::csptr(collection);
    _done = true;
    notify_update(&_collection);
}

/**
 * Computes the source beam gain for each frequency and beam number.
 */
void rvbenv_generator::beam_gain_src(const rvbenv_collection* collection,
                                     double de, double az,
                                     vector<double>& beam_work,
                                     matrix<double>& beam) {
    sensor_model::sptr source = collection->source();
    bvector arrival(de, az);
    arrival.rotate(source->orient(), arrival);
    int beam_number = 0;
    for (auto keyID : source->src_keys()) {
        bp_model::csptr bp = source->src_beam(keyID);
        bp->beam_level(arrival, _freqs, &beam_work);
        matrix_column<matrix<double> > col(beam, beam_number);
        col = beam_work;
        ++beam_number;
    }
}

/**
 * Computes the source beam gain for each frequency and beam number.
 */
void rvbenv_generator::beam_gain_rcv(const rvbenv_collection* collection,
                                     double de, double az,
                                     vector<double>& beam_work,
                                     matrix<double>& beam) {
    sensor_model::sptr receiver = collection->receiver();
    bvector arrival(de, az);
    arrival.rotate(receiver->orient(), arrival);
    int beam_number = 0;
    for (auto keyID : receiver->rcv_keys()) {
        bp_model::csptr bp = receiver->rcv_beam(keyID);
        bp->beam_level(arrival, _freqs, &beam_work);
        matrix_column<matrix<double> > col(beam, beam_number);
        col = beam_work;
        ++beam_number;
    }
}
