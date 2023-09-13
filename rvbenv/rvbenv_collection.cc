/**
 * @file rvbenv_collection.cc
 * Computes the reverberation envelope time series for all combinations of
 * receiver azimuth, source beam number, receiver beam number.
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
rvbenv_collection::rvbenv_collection(const sensor_pair::sptr& pair,
                                     const seq_vector::csptr& times,
                                     const seq_vector::csptr& freqs,
                                     size_t num_azimuths)
    : _times(times),
      _freqs(freqs),
      _num_azimuths(num_azimuths) {
    // get reference to underlying source and receiver data

    _source = pair->source();
    _receiver = pair->receiver();

    read_lock_guard src_guard(_source->mutex());
    read_lock_guard rcv_guard(_receiver->mutex());

    _source->get_motion(nullptr, &_source_pos, &_source_orient, &_source_speed);
    _receiver->get_motion(nullptr, &_receiver_pos, &_receiver_orient,
                          &_receiver_speed);
    _num_src_beams = _source->src_keys().size();
    _num_rcv_beams = _receiver->rcv_keys().size();

    // create memory for work products

    const auto num_freqs = _freqs->size();
    const auto num_times = _times->size();
    matrix<double> src_beam(num_freqs, _num_src_beams, 1.0);
    matrix<double> rcv_beam(num_freqs, _num_rcv_beams, 1.0);

    _envelopes = new matrix<double>***[_num_azimuths];
    matrix<double>**** pa = _envelopes;
    for (size_t a = 0; a < _num_azimuths; ++a, ++pa) {
        *pa = new matrix<double>**[_num_src_beams];
        matrix<double>*** ps = *pa;
        for (size_t s = 0; s < _num_src_beams; ++s, ++ps) {
            *ps = new matrix<double>*[_num_rcv_beams];
            matrix<double>** pr = *ps;
            for (size_t r = 0; r < _num_rcv_beams; ++r, ++pr) {
                *pr = new matrix<double>(num_freqs, num_times);
                (*pr)->clear();
            }
        }
    }
}

/**
 * Delete dynamic memory in each of the nested dynamic arrays.
 */
rvbenv_collection::~rvbenv_collection() {
    matrix<double>**** pa = _envelopes;
    for (size_t a = 0; a < _num_azimuths; ++a, ++pa) {
        matrix<double>*** ps = *pa;
        for (size_t s = 0; s < _num_src_beams; ++s, ++ps) {
            matrix<double>** pr = *ps;
            for (size_t r = 0; r < _num_rcv_beams; ++r, ++pr) {
                delete *pr;
            }
            delete[] * ps;
        }
        delete[] * pa;
    }
    delete[] _envelopes;
}

/**
 * Adds the intensity contribution for a single bistatic eigenverb.
 */
void rvbenv_collection::add_biverb(const biverb_model::csptr& verb,
                                   const matrix<double>& src_beam,
                                   const matrix<double>& rcv_beam) {
    const auto num_freqs = _freqs->size();
    const auto duration = verb->duration;
    const auto delay = verb->travel_time + verb->duration;

    // find range of time indices to update

    size_t first = _times->find_index(verb->travel_time - 5.0 * duration);
    size_t last = _times->find_index(verb->travel_time + 5.0 * duration) + 1;
    range window(first, last);
    vector<double> vtimes = *_times;
    vector_range<vector<double> > time(vtimes, window);
    vector<double> gaussian = exp(-0.5 * abs2((time - delay) / duration));

    // loop through source and receiver beams

    auto a = verb->az_index;
    for (size_t s = 0; s < _num_src_beams; ++s) {
        for (size_t r = 0; r < _num_rcv_beams; ++r) {
            for (size_t f = 0; f < num_freqs; ++f) {
                matrix_vector_range<matrix<double> > intensity(
                    *_envelopes[a][s][r], range(f, f), window);
                auto scale =
                    verb->power(f) * src_beam(f, s) * rcv_beam(f, r) / duration;
                intensity += scale * gaussian;
            }
        }
    }
}

/**
 * Writes the envelope data to disk
 */
void rvbenv_collection::write_netcdf(const char* filename) const {
    //    auto* nc_file = new NcFile(filename, NcFile::Replace);
    //
    //    // dimensions
    //
    //    NcDim* azimuth_dim = nc_file->add_dim("azimuth", (long)_num_azimuths);
    //    NcDim* src_beam_dim = nc_file->add_dim("src_beam",
    //    (long)_num_src_beams); NcDim* rcv_beam_dim =
    //    nc_file->add_dim("rcv_beam", (long)_num_rcv_beams); NcDim* freq_dim =
    //        nc_file->add_dim("frequency", (long)_envelope_freq->size());
    //    NcDim* time_dim =
    //        nc_file->add_dim("travel_time", (long)_travel_time->size());
    //
    //    // variables
    //
    //    NcVar* freq_var = nc_file->add_var("frequency", ncDouble, freq_dim);
    //    NcVar* time_var = nc_file->add_var("travel_time", ncDouble, time_dim);
    //    NcVar* envelopes_var =
    //        nc_file->add_var("intensity", ncDouble, azimuth_dim, src_beam_dim,
    //                         rcv_beam_dim, freq_dim, time_dim);
    //
    //    // units
    //
    //    time_var->add_att("units", "seconds");
    //    freq_var->add_att("units", "hertz");
    //    envelopes_var->add_att("units", "dB");
    //
    //    // data
    //
    //    freq_var->put(_envelope_freq->data().begin(),
    //    (long)_envelope_freq->size());
    //    time_var->put(_travel_time->data().begin(),
    //    (long)_travel_time->size());
    //
    //    for (size_t a = 0; a < _num_azimuths; ++a) {
    //        for (size_t s = 0; s < _num_src_beams; ++s) {
    //            for (size_t r = 0; r < _num_rcv_beams; ++r) {
    //                for (size_t f = 0; f < _envelope_freq->size(); ++f) {
    //                    matrix_row<matrix<double> > row(*_envelopes[a][s][r],
    //                    f); vector<double> envelope = 10.0 * log10(max(row,
    //                    1e-30)); envelopes_var->set_cur((long)a, (long)s,
    //                    (long)r, (long)f,
    //                                           0L);
    //                    envelopes_var->put(envelope.data().begin(), 1L, 1L,
    //                    1L, 1L,
    //                                       (long)_travel_time->size());
    //                }
    //            }
    //        }
    //    }
    //
    //    // close file
    //
    //    delete nc_file;  // destructor frees all netCDF temp variables
}
