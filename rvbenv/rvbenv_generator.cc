/**
 * @file rvbenv_generator.cc
 * Computes reverberation envelopes from eigenverbs.
 */

#include <usml/eigenverbs/eigenverb_collection.h>
#include <usml/eigenverbs/eigenverb_model.h>
#include <usml/ocean/boundary_model.h>
#include <usml/ocean/ocean_model.h>
#include <usml/ocean/ocean_shared.h>
#include <usml/ocean/volume_model.h>
#include <usml/platforms/platform_manager.h>
#include <usml/rvbenv/rvbenv_collection.h>
#include <usml/rvbenv/rvbenv_generator.h>
#include <usml/types/seq_linear.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition.h>
#include <usml/types/wposition1.h>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <cmath>
#include <cstddef>
#include <memory>
#include <vector>

using namespace usml::eigenverbs;
using namespace usml::rvbenv;
using namespace usml::ocean;
using namespace usml::platforms;
using namespace usml::types;

/**
 * Minimum intensity level for valid reverberation contributions (dB).
 */
double rvbenv_generator::intensity_threshold = -300.0;

/**
 * Maximum distance between the peaks of the source and receiver eigenverbs.
 */
double rvbenv_generator::distance_threshold = 6.0;

/**
 * The mutex for static travel_time property.
 */
read_write_lock rvbenv_generator::_travel_time_mutex;

/**
 * Time axis for reverberation calculation.  Defaults to
 * a linear sequence out to 400 sec with a sampling period of 0.1 sec.
 */
std::unique_ptr<const seq_vector> rvbenv_generator::_travel_time(
    new seq_linear(0.0, 0.01, 401));

/**
 * Copies envelope computation parameters from static memory into
 * this specific task.
 */
rvbenv_generator::rvbenv_generator(bistatic_pair* pair, size_t num_azimuths)
    : _done(false),
      _ocean(ocean_shared::current()),
      _bistatic_pair(pair),
      _src_eigenverbs(pair->src_eigenverbs()),
      _rcv_eigenverbs(pair->rcv_eigenverbs()) {
    // setup travel time property for this particular envelope

    read_lock_guard guard(_travel_time_mutex);
    seq_vector::csptr time = seq_vector::build_best(
        travel_time()->data().begin(), travel_time()->size());

    // build structure for rvbenv_collection

    _envelopes = new rvbenv_collection(
        platform_manager::instance()->frequencies(), time,
        pow(10.0, intensity_threshold / 10.0), num_azimuths,
        _src_beam_list.size(), _rcv_beam_list.size(),
        _bistatic_pair->source()->keyID(), _bistatic_pair->receiver()->keyID(),
        _bistatic_pair->source()->position(),
        _bistatic_pair->receiver()->position());
}

/**
 * Executes the Eigenverb reverberation model.
 */
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void rvbenv_generator::run() {
    // create memory for work products

    seq_vector::csptr freq = _envelopes->envelope_freq();
    const size_t num_freq = freq->size();

    vector<double> scatter(num_freq, 1.0);
    matrix<double> src_beam(num_freq, _envelopes->num_src_beams(), 1.0);
    matrix<double> rcv_beam(num_freq, _envelopes->num_rcv_beams(), 1.0);

    eigenverb_model rcv_verb;
    rcv_verb.frequencies = freq;
    rcv_verb.power = vector<double>(num_freq);

    // loop through eigenverbs for each interface

    for (size_t interface = 0; interface < _rcv_eigenverbs->num_interfaces();
         ++interface) {
        for (const auto& rcv_verb : _rcv_eigenverbs->eigenverbs(interface)) {
            eigenverb_collection::point min_pt(rcv_verb->bounding_box.south,
                                               rcv_verb->bounding_box.west);
            eigenverb_collection::point max_pt(rcv_verb->bounding_box.south,
                                               rcv_verb->bounding_box.west);
            eigenverb_collection::box box(min_pt, max_pt);
            eigenverb_list src_eigenverbs =
                _src_eigenverbs->find_eigenverbs(box, interface);

            for (const auto& src_verb : src_eigenverbs) {
                // determine relative range and bearing between Gaussians
                // skip this combo if source peak too far away

                double bearing;
                const double range =
                    rcv_verb->position.gc_range(src_verb->position, &bearing);
                if (range > distance_threshold *
                                max(rcv_verb->length, rcv_verb->width)) {
                    continue;
                }

                if (range < 1e-6) {
                    bearing = 0;  // fixes bearing = NaN
                }
                bearing -= rcv_verb->direction;  // relative bearing

                const double ys = range * cos(bearing);
                const double ys2 = ys * ys;
                if (abs(ys) > distance_threshold * rcv_verb->length) {
                    continue;
                }

                const double xs = range * sin(bearing);
                const double xs2 = xs * xs;
                if (abs(xs) > distance_threshold * rcv_verb->width) {
                    continue;
                }

                // compute interface scattering strength
                // skip this combo if scattering strength is trivial

                _ocean->scattering(interface, rcv_verb->position, freq,
                                   src_verb->grazing, rcv_verb->grazing,
                                   src_verb->direction, rcv_verb->direction,
                                   &scatter);
                if (std::any_of(scatter.begin(), scatter.end(), [](double v) {
                        return v > intensity_threshold;
                    })) {
                    continue;
                }

                // compute beam levels

                src_beam =
                    beam_gain_src(_bistatic_pair->source(), freq,
                                  src_verb->source_de, src_verb->source_az);
                rcv_beam =
                    beam_gain_rcv(_bistatic_pair->receiver(), freq,
                                  rcv_verb->source_de, rcv_verb->source_az);

                // create envelope contribution

                _envelopes->add_contribution(src_verb, rcv_verb, src_beam,
                                             rcv_beam, scatter, xs2, ys2);
            }
        }
    }
    _rvbenv_collection = rvbenv_collection::csptr(_envelopes);
    notify_update(&_rvbenv_collection);
}

/**
 * Computes the beam_gain
 */
matrix<double> rvbenv_generator::beam_gain_src(
    const sensor_model::sptr& sensor, const seq_vector::csptr& frequencies,
    double de, double az) {
    const auto* sensor_ptr = dynamic_cast<const sensor_model*>(sensor.get());
    auto beam_list = sensor_ptr->src_keys();
    matrix<double> beam_matrix(frequencies->size(), beam_list.size());
    vector<double> level(frequencies->size(), 0.0);

    bvector arrival(de, az);
    arrival.rotate(sensor->orient(), arrival);

    int beam_number = 0;
    for (int keyID : beam_list) {
        bp_model::csptr bp = sensor_ptr->src_beam(keyID);
        bp->beam_level(arrival, frequencies, &level);
        matrix_column<matrix<double> > col(beam_matrix, beam_number);
        col = level;
        ++beam_number;
    }
    return beam_matrix;
}

/**
 * Computes the beam_gain
 */
matrix<double> rvbenv_generator::beam_gain_rcv(
    const sensor_model::sptr& sensor, const seq_vector::csptr& frequencies,
    double de, double az) {
    const auto* sensor_ptr = dynamic_cast<const sensor_model*>(sensor.get());
    auto beam_list = sensor_ptr->rcv_keys();
    matrix<double> beam_matrix(frequencies->size(), beam_list.size());
    vector<double> level(frequencies->size(), 0.0);

    bvector arrival(de, az);
    arrival.rotate(sensor->orient(), arrival);

    int beam_number = 0;
    for (int keyID : beam_list) {
        bp_model::csptr bp = sensor_ptr->rcv_beam(keyID);
        bp->beam_level(arrival, frequencies, &level);
        matrix_column<matrix<double> > col(beam_matrix, beam_number);
        col = level;
        ++beam_number;
    }
    return beam_matrix;
}
