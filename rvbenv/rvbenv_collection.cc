/**
 * @file rvbenv_collection.cc
 * Computes the reverberation envelope time series for all combinations of
 * receiver azimuth, source beam number, receiver beam number.
 */

#include <ncvalues.h>
#include <netcdfcpp.h>
#include <usml/rvbenv/rvbenv_collection.h>
#include <usml/types/seq_data.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition1.h>
#include <usml/ublas/vector_math.h>

#include <algorithm>
#include <boost/numeric/ublas/expression_types.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/storage.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <cmath>
#include <complex>
#include <cstddef>
#include <memory>
#include <utility>

using namespace usml::types;
using namespace usml::rvbenv;

/**
 * Reserve memory in which to store results as a series of
 * nested dynamic arrays.
 */
rvbenv_collection::rvbenv_collection(seq_vector::csptr  envelope_freq,
                                     seq_vector::csptr  travel_time,
                                     double threshold, size_t num_azimuths,
                                     size_t num_src_beams, size_t num_rcv_beams,
                                     int source_id, int receiver_id,
                                     wposition1 src_position,
                                     wposition1 rcv_position)
    : _envelope_freq(std::move(envelope_freq)),
      _travel_time(std::move(travel_time)),
      _threshold(threshold),
      _num_azimuths(num_azimuths),
      _num_src_beams(num_src_beams),
      _num_rcv_beams(num_rcv_beams),
      _slant_range(0.0),
      _source_id(source_id),
      _receiver_id(receiver_id),
      _source_position(src_position),
      _receiver_position(rcv_position),
      _rvbenv_model(_envelope_freq, _travel_time, _threshold) {
    _slant_range = _receiver_position.distance(_source_position);

    _envelopes = new matrix<double>***[_num_azimuths];
    matrix<double>**** pa = _envelopes;
    for (size_t a = 0; a < _num_azimuths; ++a, ++pa) {
        *pa = new matrix<double>**[_num_src_beams];
        matrix<double>*** ps = *pa;
        for (size_t s = 0; s < _num_src_beams; ++s, ++ps) {
            *ps = new matrix<double>*[_num_rcv_beams];
            matrix<double>** pr = *ps;
            for (size_t r = 0; r < _num_rcv_beams; ++r, ++pr) {
                *pr = new matrix<double>(_envelope_freq->size(),
                                         _travel_time->size());
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
 * Adds the intensity contribution for a single combination of source
 * and receiver eigenverbs.
 */
void rvbenv_collection::add_contribution(const eigenverb_model::csptr& src_verb,
                                         const eigenverb_model::csptr& rcv_verb,
                                         const matrix<double>& src_beam,
                                         const matrix<double>& rcv_beam,
                                         const vector<double>& scatter,
                                         double xs2, double ys2) {
    size_t azimuth = rcv_verb->az_index;
    bool ok =
        _rvbenv_model.compute_intensity(src_verb, rcv_verb, scatter, xs2, ys2);
    if (ok) {
        for (size_t s = 0; s < src_beam.size2(); ++s) {
            for (size_t r = 0; r < rcv_beam.size2(); ++r) {
                for (size_t f = 0; f < _envelope_freq->size(); ++f) {
                    matrix_row<matrix<double> > intensity(
                        _rvbenv_model.intensity(), f);
                    matrix_row<matrix<double> > envelope(
                        *_envelopes[azimuth][s][r], f);
                    envelope += src_beam(f, s) * rcv_beam(f, r) * intensity;
                }
            }
        }
    }
}

/**
 * Updates the rvbenv_collection data with the parameters provided.
 */
void rvbenv_collection::dead_reckon(double delta_time, double slant_range,
                                    double prev_range) {
    // Set new slant_range
    _slant_range = slant_range;

    // Shift the time series
    vector<double> temp_data = (*_travel_time);
    temp_data = temp_data + delta_time;
    _travel_time = seq_vector::csptr(new seq_data(temp_data));

    {  // Scope for lock

        // Perform copy and intensity update
        double gain = slant_range / prev_range;
        gain *= gain;

        write_lock_guard guard(this->_envelopes_mutex);
        // Copy "this" envelopes to new_collection
        matrix<double>**** pa = _envelopes;
        for (size_t a = 0; a < _num_azimuths; ++a, ++pa) {
            matrix<double>*** ps = *pa;
            for (size_t s = 0; s < _num_src_beams; ++s, ++ps) {
                matrix<double>** pr = *ps;
                for (size_t r = 0; r < _num_rcv_beams; ++r, ++pr) {
                    (**pr) = this->envelope(a, s, r);
                    (**pr) *= gain;
                }
            }
        }
    }
}

/**
 * Writes the envelope data to disk
 */
void rvbenv_collection::write_netcdf(const char* filename) const {
    auto* nc_file = new NcFile(filename, NcFile::Replace);

    // dimensions

    NcDim* azimuth_dim = nc_file->add_dim("azimuth", (long)_num_azimuths);
    NcDim* src_beam_dim = nc_file->add_dim("src_beam", (long)_num_src_beams);
    NcDim* rcv_beam_dim = nc_file->add_dim("rcv_beam", (long)_num_rcv_beams);
    NcDim* freq_dim =
        nc_file->add_dim("frequency", (long)_envelope_freq->size());
    NcDim* time_dim =
        nc_file->add_dim("travel_time", (long)_travel_time->size());

    // variables

    NcVar* threshold_var = nc_file->add_var("threshold", ncDouble);
    NcVar* freq_var = nc_file->add_var("frequency", ncDouble, freq_dim);
    NcVar* time_var = nc_file->add_var("travel_time", ncDouble, time_dim);
    NcVar* envelopes_var =
        nc_file->add_var("intensity", ncDouble, azimuth_dim, src_beam_dim,
                         rcv_beam_dim, freq_dim, time_dim);

    // units

    threshold_var->add_att("units", "dB");
    time_var->add_att("units", "seconds");
    freq_var->add_att("units", "hertz");
    envelopes_var->add_att("units", "dB");

    // data

    threshold_var->put(&_threshold);
    freq_var->put(_envelope_freq->data().begin(), (long)_envelope_freq->size());
    time_var->put(_travel_time->data().begin(), (long)_travel_time->size());

    for (size_t a = 0; a < _num_azimuths; ++a) {
        for (size_t s = 0; s < _num_src_beams; ++s) {
            for (size_t r = 0; r < _num_rcv_beams; ++r) {
                for (size_t f = 0; f < _envelope_freq->size(); ++f) {
                    matrix_row<matrix<double> > row(*_envelopes[a][s][r], f);
                    vector<double> envelope = 10.0 * log10(max(row, 1e-30));
                    envelopes_var->set_cur((long)a, (long)s, (long)r, (long)f,
                                           0L);
                    envelopes_var->put(envelope.data().begin(), 1L, 1L, 1L, 1L,
                                       (long)_travel_time->size());
                }
            }
        }
    }

    // close file

    delete nc_file;  // destructor frees all netCDF temp variables
}
