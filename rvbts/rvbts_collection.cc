/**
 * @file rvbts_collection.cc
 * Computes the reverberation time series time series for all combinations of
 * transmit frequency, source beam number, receiver beam number.
 */

#include <ncvalues.h>
#include <netcdfcpp.h>
#include <usml/beampatterns/bp_model.h>
#include <usml/rvbts/rvbts_collection.h>
#include <usml/types/bvector.h>
#include <usml/types/seq_linear.h>
#include <usml/ublas/math_traits.h>
#include <usml/ublas/vector_math.h>

#include <boost/numeric/ublas/fwd.hpp>
#include <boost/numeric/ublas/storage.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_expression.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <cmath>
#include <cstddef>
#include <list>

using namespace usml::rvbts;

/**
 * Initialize model parameters with state of sensor_pair at the time that
 * reverberation generator was created.
 */
rvbts_collection::rvbts_collection(
    const sensor_model::sptr& source, const wposition1 source_pos,
    const orientation& source_orient, const double source_speed,
    const sensor_model::sptr& receiver, const wposition1 receiver_pos,
    const orientation& receiver_orient, const double receiver_speed,
    const seq_vector::csptr& travel_times)
    : _source(source),
      _source_pos(source_pos),
      _source_orient(source_orient),
      _source_speed(source_speed),
      _receiver(receiver),
      _receiver_pos(receiver_pos),
      _receiver_orient(receiver_orient),
      _receiver_speed(receiver_speed),
      _travel_times(travel_times),
      _time_series(receiver->rcv_num_keys(), travel_times->size()) {}

/**
 * Adds the intensity contribution for a single bistatic eigenverb.
 *
 * TODO Add the ability to compute Doppler shifts.
 * TODO Add the ability to compute element level phase delays between receivers.
 */
void rvbts_collection::add_biverb(const biverb_model::csptr &verb,
                                  const transmit_model::csptr& transmit,
                                  const bvector& steering) {
    static const double SQRT_TWO_PI = sqrt(TWO_PI);

    // find range of time indices to update

    const auto duration = verb->duration;
    const auto delay = transmit->delay + verb->travel_time + duration;
    size_t first = _travel_times->find_index(delay - 5.0 * duration);
    size_t last = _travel_times->find_index(delay + 5.0 * duration) + 1;
    range window(first, last);

    // update Gaussian time series in this window

    vector<double> times = _travel_times->data();
    vector_range<vector<double> > tau(times, window);
    vector<double> gaussian =
        exp(-0.5 * abs2((tau - delay) / duration)) / (duration * SQRT_TWO_PI);

    // interpolate eigenverb power

    double verb_level = verb->power[0];
    if (verb->frequencies->size() > 1) {
        double freq = transmit->fcenter;
        const seq_vector &axis = *(verb->frequencies);
        size_t index = axis.find_nearest(freq);
        if (index >= axis.size()) {
            --index;
        }
        double u = (freq - axis[index]) / axis.increment(index);
        verb_level = u * verb->power[index + 1] + (1 - u) * verb->power[index];
    }

    // compute source level for this transmission

    bp_model::csptr src_beam = _source->src_beam(transmit->transmit_mode);
    const seq_vector::csptr frequencies(
        new seq_linear(transmit->fcenter, 1.0, 1));
    vector<double> level(1);
    bvector src_arrival(verb->source_de, verb->source_az);
    src_arrival.rotate(_source_orient, src_arrival);
    src_beam->beam_level(src_arrival, frequencies, &level, steering);
    double src_level = transmit->source_level + level[0];

    // add Gaussian to each receiver channel

    for (int rcv : this->_receiver->rcv_keys()) {
        // compute received level for this transmission

        bp_model::csptr rcv_beam = _receiver->rcv_beam(rcv);
        bvector rcv_arrival(verb->receiver_de, verb->receiver_az);
        rcv_arrival.rotate(_receiver_orient, rcv_arrival);
        bvector rcv_steering = _receiver->rcv_steering(rcv);
        rcv_beam->beam_level(rcv_arrival, frequencies, &level, rcv_steering);
        double rcv_level = src_level + verb_level + level[0];

        // add scaled Gaussian to each result in time window

        for (size_t n = 0; n < gaussian.size(); ++n) {
            auto t = n + first - 1;
            _time_series(rcv, t) += rcv_level * gaussian[n];
        }
    }
}

/**
 * Writes reverberation time series data to disk.
 */
void rvbts_collection::write_netcdf(const char *filename) const {
    auto *nc_file = new NcFile(filename, NcFile::Replace);

    auto num_channels = (long)_time_series.size1();
    auto num_times = (long)_time_series.size2();

    // dimensions

    NcDim *channels_dim = nc_file->add_dim("channels", num_channels);
    NcDim *time_dim = nc_file->add_dim("travel_time", num_times);

    // variables

    NcVar *src_id_var = nc_file->add_var("sourceID", ncLong);
    NcVar *src_lat_var = nc_file->add_var("source_latitude", ncDouble);
    NcVar *src_lng_var = nc_file->add_var("source_longitude", ncDouble);
    NcVar *src_alt_var = nc_file->add_var("source_altitude", ncDouble);

    NcVar *src_yaw_var = nc_file->add_var("source_yaw", ncDouble);
    NcVar *src_pitch_var = nc_file->add_var("source_pitch", ncDouble);
    NcVar *src_roll_var = nc_file->add_var("source_roll", ncDouble);

    NcVar *src_speed_var = nc_file->add_var("source_speed", ncDouble);

    NcVar *rcv_id_var = nc_file->add_var("receiverID", ncLong);
    NcVar *rcv_lat_var = nc_file->add_var("receiver_latitude", ncDouble);
    NcVar *rcv_lng_var = nc_file->add_var("receiver_longitude", ncDouble);
    NcVar *rcv_alt_var = nc_file->add_var("receiver_altitude", ncDouble);

    NcVar *rcv_yaw_var = nc_file->add_var("receiver_yaw", ncDouble);
    NcVar *rcv_pitch_var = nc_file->add_var("receiver_pitch", ncDouble);
    NcVar *rcv_roll_var = nc_file->add_var("receiver_roll", ncDouble);

    NcVar *rcv_speed_var = nc_file->add_var("receiver_speed", ncDouble);

    NcVar *channels_var = nc_file->add_var("channels", ncDouble, channels_dim);
    NcVar *time_var = nc_file->add_var("travel_time", ncDouble, time_dim);
    NcVar *time_series_var =
        nc_file->add_var("time_series", ncDouble, channels_dim, time_dim);

    // units

    time_var->add_att("units", "seconds");

    src_yaw_var->add_att("units", "degrees");
    src_pitch_var->add_att("units", "degrees");
    src_roll_var->add_att("units", "degrees");
    src_speed_var->add_att("units", "m/s");

    rcv_yaw_var->add_att("units", "degrees");
    rcv_pitch_var->add_att("units", "degrees");
    rcv_roll_var->add_att("units", "degrees");
    rcv_speed_var->add_att("units", "m/s");

    // write source parameters

    // clang-format off
    int n;
    double v;
    n = _source->keyID();			src_id_var->put(&n);
    v = _source_pos.latitude(); 	src_lat_var->put(&v);
    v = _source_pos.longitude();	src_lng_var->put(&v);
    v = _source_pos.altitude(); 	src_alt_var->put(&v);

    orientation src_orient(_source_orient);
    v = src_orient.yaw(); 			src_yaw_var->put(&v);
    v = src_orient.pitch();			src_pitch_var->put(&v);
    v = src_orient.roll(); 			src_roll_var->put(&v);

    v = _source_speed; 				src_speed_var->put(&v);

    // write receiver parameters

    n = _receiver->keyID();			rcv_id_var->put(&n);
    v = _receiver_pos.latitude(); 	rcv_lat_var->put(&v);
    v = _receiver_pos.longitude();	rcv_lng_var->put(&v);
    v = _receiver_pos.altitude(); 	rcv_alt_var->put(&v);

    orientation rcv_orient(_receiver_orient);
    v = rcv_orient.yaw(); 			rcv_yaw_var->put(&v);
    v = rcv_orient.pitch();			rcv_pitch_var->put(&v);
    v = rcv_orient.roll(); 			rcv_roll_var->put(&v);

    v = _receiver_speed; 			rcv_speed_var->put(&v);
    // clang-format on

    // data

    seq_linear channels(0.0, 1.0, (size_t)num_channels);
    channels_var->put(channels.data().begin(), num_channels);
    time_var->put(_travel_times->data().begin(), num_times);
    time_series_var->put(_time_series.data().begin(), num_channels * num_times);

    // close file and free all netCDF temp variables

    delete nc_file;
}
