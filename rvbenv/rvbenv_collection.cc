/**
 * @file rvbenv_collection.cc
 * Computes the reverberation envelope time series for all combinations of
 * transmit frequency, source beam number, receiver beam number.
 */

#include <usml/platforms/platform_model.h>
#include <usml/rvbenv/rvbenv_collection.h>
#include <usml/sensors/sensor_model.h>
#include <usml/ublas/vector_math.h>

#include <algorithm>
#include <boost/numeric/ublas/expression_types.hpp>
#include <boost/numeric/ublas/fwd.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/storage.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_expression.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <list>

using namespace usml::rvbenv;

/**
 * Initialize model with data from a sensor_pair.
 */
rvbenv_collection::rvbenv_collection(const sensor_model::sptr& source,
                                     const sensor_model::sptr& receiver,
                                     const seq_vector::csptr& travel_times,
                                     const seq_vector::csptr& frequencies)
    : _travel_times(travel_times),
      _frequencies(frequencies),
      _source(source),
      _receiver(receiver),
      _num_src_beams(_source->src_keys().size()),
      _num_rcv_beams(_receiver->rcv_keys().size()),
      _source_pos(_source->position()),
      _receiver_pos(_source->position()),
      _source_orient(_source->orient()),
      _receiver_orient(_receiver->orient()),
      _source_speed(_source->speed()),
      _receiver_speed(_receiver->speed()) {
    const auto num_freqs = _frequencies->size();
    const auto num_times = _travel_times->size();

    _envelopes = new matrix<double>**[_num_src_beams];
    matrix<double>*** ps = _envelopes;
    for (size_t s = 0; s < _num_src_beams; ++s, ++ps) {
        *ps = new matrix<double>*[_num_rcv_beams];
        matrix<double>** pr = *ps;
        for (size_t r = 0; r < _num_rcv_beams; ++r, ++pr) {
            *pr = new matrix<double>(num_freqs, num_times);
            (*pr)->clear();
        }
    }
}

/**
 * Delete dynamic memory in each of the nested dynamic arrays.
 */
rvbenv_collection::~rvbenv_collection() {
    matrix<double>*** ps = _envelopes;
    for (size_t s = 0; s < _num_src_beams; ++s, ++ps) {
        matrix<double>** pr = *ps;
        for (size_t r = 0; r < _num_rcv_beams; ++r, ++pr) {
            delete *pr;
        }
        delete[] *ps;
    }
    delete[] _envelopes;
}

/**
 * Adds the intensity contribution for a single bistatic eigenverb.
 */
void rvbenv_collection::add_biverb(const biverb_model::csptr& verb,
                                   const matrix<double>& src_beam,
                                   const matrix<double>& rcv_beam) {
    const auto num_freqs = _frequencies->size();
    const auto duration = verb->duration;
    const auto delay = verb->travel_time + duration;
    static const double SQRT_TWO_PI = sqrt(TWO_PI);

    // find range of time indices to update

    size_t first = _travel_times->find_index(delay - 5.0 * duration);
    size_t last = _travel_times->find_index(delay + 5.0 * duration) + 1;
    range window(first, last);

    // update Gaussian levels in this time window

    vector<double> times = _travel_times->data();
    vector_range<vector<double> > tau(times, window);
    vector<double> gaussian =
        exp(-0.5 * abs2((tau - delay) / duration)) / (duration * SQRT_TWO_PI);

    // add Gaussian to each source beam, receiver beam, and frequency

    for (size_t s = 0; s < _num_src_beams; ++s) {
        for (size_t r = 0; r < _num_rcv_beams; ++r) {
            matrix<double>& intensity = *_envelopes[s][r];
            for (size_t f = 0; f < num_freqs; ++f) {
                // compute eigenverb total power for this source beam, receiver
                // beam, and frequency combination
                auto scale = verb->power(f) * src_beam(f, s) * rcv_beam(f, r);

                // add scaled Gaussian to each intensity in time window
                for (size_t n = 0; n < gaussian.size(); ++n) {
                    auto t = n + first - 1;
                    intensity(f, t) += scale * gaussian[n];
                }
            }
        }
    }
}

/**
 * Writes reverberation envelope data to disk.
 */
void rvbenv_collection::write_netcdf(const char* filename) const {
    read_lock_guard guard(_mutex);
    auto* nc_file = new NcFile(filename, NcFile::Replace);

    // dimensions

    NcDim* src_beam_dim = nc_file->add_dim("src_beam", (long)_num_src_beams);
    NcDim* rcv_beam_dim = nc_file->add_dim("rcv_beam", (long)_num_rcv_beams);
    NcDim* freq_dim =
        nc_file->add_dim("frequencies", (long)_frequencies->size());
    NcDim* time_dim =
        nc_file->add_dim("travel_time", (long)_travel_times->size());

    // variables

    NcVar* freq_var = nc_file->add_var("frequencies", ncDouble, freq_dim);
    NcVar* time_var = nc_file->add_var("travel_time", ncDouble, time_dim);
    NcVar* envelopes_var = nc_file->add_var("intensity", ncDouble, src_beam_dim,
                                            rcv_beam_dim, freq_dim, time_dim);

    // units

    time_var->add_att("units", "seconds");
    freq_var->add_att("units", "hertz");
    envelopes_var->add_att("units", "dB");

    // data

    freq_var->put(_frequencies->data().begin(), (long)_frequencies->size());
    time_var->put(_travel_times->data().begin(), (long)_travel_times->size());

    for (size_t s = 0; s < _num_src_beams; ++s) {
        for (size_t r = 0; r < _num_rcv_beams; ++r) {
            for (size_t f = 0; f < _frequencies->size(); ++f) {
                matrix_row<matrix<double> > row(*_envelopes[s][r], f);
                vector<double> envelope = 10.0 * log10(max(row, 1e-30));
                envelopes_var->set_cur((long)s, (long)r, (long)f, 0L);
                envelopes_var->put(envelope.data().begin(), 1L, 1L, 1L,
                                   (long)_travel_times->size());
            }
        }
    }

    // close file and free all netCDF temp variables

    delete nc_file;
}
