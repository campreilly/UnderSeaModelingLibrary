/**
 * @file biverb_generator.cc
 * Computes reverberation envelopes from eigenverbs.
 */

#include <usml/biverbs/biverb_collection.h>
#include <usml/biverbs/biverb_generator.h>
#include <usml/eigenverbs/eigenverb_collection.h>
#include <usml/eigenverbs/eigenverb_model.h>
#include <usml/ocean/boundary_model.h>
#include <usml/ocean/ocean_model.h>
#include <usml/ocean/ocean_shared.h>
#include <usml/ocean/volume_model.h>
#include <usml/platforms/platform_manager.h>
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
using namespace usml::biverbs;
using namespace usml::ocean;
using namespace usml::platforms;
using namespace usml::types;

#define DEBUG_BIVERB

/**
 * Minimum intensity level for valid reverberation contributions (ratio).
 */
double biverb_generator::intensity_threshold = 1e-30;

/**
 * Maximum distance between the peaks of the source and receiver eigenverbs.
 */
double biverb_generator::distance_threshold = 6.0;

/**
 * The mutex for static travel_time property.
 */
read_write_lock biverb_generator::_travel_time_mutex;

/**
 * Time axis for reverberation calculation.  Defaults to
 * a linear sequence out to 400 sec with a sampling period of 0.1 sec.
 */
std::unique_ptr<const seq_vector> biverb_generator::_travel_time(
    new seq_linear(0.0, 0.01, 401));

/**
 * Copies envelope computation parameters from static memory into
 * this specific task.
 */
biverb_generator::biverb_generator(bistatic_pair* pair, size_t /*num_azimuths*/)
    : _done(false),

      _bistatic_pair(pair),
      _src_eigenverbs(pair->src_eigenverbs()),
      _rcv_eigenverbs(pair->rcv_eigenverbs()) {}

/**
 * Executes the Eigenverb reverberation model.
 */
void biverb_generator::run() {
    auto ocean = ocean_shared::current();
    const size_t num_freq = platform_manager::instance()->frequencies()->size();
    vector<double> scatter(num_freq, 1.0);
    auto* collection = new biverb_collection(ocean->num_volume());

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
                create_biverb(collection, ocean, interface, scatter, src_verb,
                              rcv_verb);
            }
        }
    }
    _collection = biverb_collection::csptr(collection);
    this->notify_update(&_collection);
}

/**
 * Create biverb from a single combination of source and receiver eigneverbs.
 */
void biverb_generator::create_biverb(biverb_collection* collection,
                                     const ocean_model::csptr& ocean,
                                     size_t interface, vector<double>& scatter,
                                     const eigenverb_model::csptr& src_verb,
                                     const eigenverb_model::csptr& rcv_verb) {
    // determine relative range and bearing between Gaussians
    // skip this combo if source peak too far away

    double bearing;
    const double range =
        rcv_verb->position.gc_range(src_verb->position, &bearing);
    if (range > distance_threshold * max(rcv_verb->length, rcv_verb->width)) {
        return;
    }

    if (range < 1e-6) {
        bearing = 0;  // fixes bearing = NaN
    }
    bearing -= rcv_verb->direction;  // relative bearing

    const double ys = range * cos(bearing);
    const double ys2 = ys * ys;
    if (abs(ys) > distance_threshold * rcv_verb->length) {
        return;
    }

    const double xs = range * sin(bearing);
    const double xs2 = xs * xs;
    if (abs(xs) > distance_threshold * rcv_verb->width) {
        return;
    }

    // compute interface scattering strength
    // skip this combo if scattering strength is trivial

    ocean->scattering(interface, rcv_verb->position, rcv_verb->frequencies,
                      src_verb->grazing, rcv_verb->grazing, src_verb->direction,
                      rcv_verb->direction, &scatter);

    // create bistatic eigenverb contribution

    if (std::any_of(scatter.begin(), scatter.end(),
                    [](double v) { return v > intensity_threshold; })) {
        biverb_model* verb =
            compute_overlap(src_verb, rcv_verb, xs2, ys2, scatter);
        collection->add_biverb(biverb_model::csptr(verb), interface);
    }
}

/**
 * Compute the biverb that results from the overlap of two eigenverbs.
 */
biverb_model* biverb_generator::compute_overlap(
    const eigenverb_model::csptr& src_verb,
    const eigenverb_model::csptr& rcv_verb, double xs2, double ys2,
    const vector<double>& scatter) {
#ifdef DEBUG_BIVERB
    cout << "biverb_generator::compute_overlap() " << endl
         << "\txs2=" << xs2 << " ys2=" << ys2 << " scatter=" << scatter << endl
         << "\tsrc_verb"
         << " t=" << src_verb->time << " de=" << to_degrees(src_verb->source_de)
         << " az=" << to_degrees(src_verb->source_az)
         << " direction=" << to_degrees(src_verb->direction)
         << " grazing=" << to_degrees(src_verb->grazing) << endl
         << "\tpower=" << 10.0 * log10(src_verb->power)
         << " length=" << src_verb->length << " width=" << src_verb->width
         << endl
         << "\tsurface=" << src_verb->surface << " bottom=" << src_verb->bottom
         << " caustic=" << src_verb->caustic << endl
         << "\trcv_verb"
         << " t=" << rcv_verb->time << " de=" << to_degrees(rcv_verb->source_de)
         << " az=" << to_degrees(rcv_verb->source_az)
         << " direction=" << to_degrees(rcv_verb->direction)
         << " grazing=" << to_degrees(rcv_verb->grazing) << endl
         << "\tpower=" << 10.0 * log10(rcv_verb->power)
         << " length=" << rcv_verb->length << " width=" << rcv_verb->width
         << endl
         << "\tsurface=" << rcv_verb->surface << " bottom=" << rcv_verb->bottom
         << " caustic=" << rcv_verb->caustic << endl;
#endif
    // copy data from source and receiver eigenverbs

    auto* biverb = new biverb_model();
    biverb->time = src_verb->time + rcv_verb->time;
    biverb->frequencies = platform_manager::instance()->frequencies();
    biverb->de_index = rcv_verb->de_index;
    biverb->az_index = rcv_verb->az_index;
    biverb->source_de = src_verb->source_de;
    biverb->source_de = src_verb->source_az;
    biverb->receiver_de = rcv_verb->source_de;
    biverb->receiver_az = rcv_verb->source_az;

    // determine the relative tilt between the projected Gaussians

    const double alpha = src_verb->direction - rcv_verb->direction;
    const double cos2alpha = cos(2.0 * alpha);
    const double sin2alpha = sin(2.0 * alpha);

    // compute commonly used terms in the intersection of the Gaussian profiles

    auto src_length2 = src_verb->length * src_verb->length;
    auto src_width2 = src_verb->width * src_verb->width;
    const double src_sum = src_length2 + src_width2;
    const double src_diff = src_length2 - src_width2;
    const double src_prod = src_length2 * src_width2;

    auto rcv_length2 = rcv_verb->length * rcv_verb->length;
    auto rcv_width2 = rcv_verb->width * rcv_verb->width;
    const double rcv_sum = rcv_length2 + rcv_width2;
    const double rcv_diff = rcv_length2 - rcv_width2;
    const double rcv_prod = rcv_length2 * rcv_width2;

    // compute the scaling of the exponential
    // equations (26) and (28) from the paper

    double det_sr = 0.5 * (2.0 * (src_prod + rcv_prod) + (src_sum * rcv_sum) -
                           (src_diff * rcv_diff) * cos2alpha);
    biverb->power = 0.25 * 0.5 * src_verb->power * rcv_verb->power * scatter;

    // compute the power of the exponential
    // equation (28) from the paper

    const double new_prod = src_diff * cos2alpha;
    const double kappa = -0.25 *
                         (xs2 * (src_sum + new_prod + 2.0 * rcv_length2) +
                          ys2 * (src_sum - new_prod + 2.0 * rcv_width2) -
                          2.0 * sqrt(xs2 * ys2) * src_diff * sin2alpha) /
                         det_sr;
#ifdef DEBUG_BIVERB
    cout << "\tsrc_verb->power=" << src_verb->power
         << " rcv_verb->power=" << rcv_verb->power << endl
         << "\tdet_sr=" << det_sr << " kappa=" << kappa
         << " power=" << (10.0 * log10(biverb->power)) << endl;
#endif
    biverb->power *= exp(kappa) / sqrt(det_sr);

    // compute the square of the duration of the overlap
    // equation (41) from the paper

    det_sr = det_sr / (src_prod * rcv_prod);
    auto sigma = 0.5 *
                 ((1.0 / src_width2 + 1.0 / src_length2) +
                  (1.0 / src_width2 - 1.0 / src_length2) * cos2alpha +
                  2.0 / rcv_width2) /
                 det_sr;

    // combine duration of the overlap with pulse length
    // equation (33) from the paper

    const double factor = cos(rcv_verb->grazing) / rcv_verb->sound_speed;
    biverb->duration = 0.5 * factor * sqrt(sigma);
#ifdef DEBUG_BIVERB
    cout << "\tcontribution duration=" << biverb->duration
         << " power=" << (10.0 * log10(biverb->power)) << endl;
#endif

    // check threshold to avoid calculations for weak signals

    return biverb;
}
