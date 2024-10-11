/**
 * @file rvbts_collection.cc
 * Computes the reverberation time series time series for all combinations of
 * transmit frequency, source beam number, receiver beam number.
 */

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
#include <netcdf>

using namespace usml::rvbts;

/**
 * Threshold for minimum envelope power.
 */
double rvbts_collection::power_threshold = 1e-20;

/**
 * Initialize model parameters with state of sensor_pair at the time that
 * reverberation generator was created.
 */
rvbts_collection::rvbts_collection(
    const sensor_model::sptr &source, const wposition1 source_pos,
    const orientation &source_orient, const double source_speed,
    const sensor_model::sptr &receiver, const wposition1 receiver_pos,
    const orientation &receiver_orient, const double receiver_speed,
    const seq_vector::csptr &travel_times)
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
 */
void rvbts_collection::add_biverb(const biverb_model::csptr &verb,
                                  const transmit_model::csptr &transmit,
                                  const bvector &steering) {
    static const double SQRT_TWO_PI = sqrt(TWO_PI);

    // find range of time indices to update

    const auto duration = verb->duration + transmit->duration;
    const auto delay = transmit->delay + verb->travel_time + duration;
    size_t first = _travel_times->find_index(delay - 5.0 * duration);
    size_t last = _travel_times->find_index(delay + 5.0 * duration);
    range window(first, last);

    // update Gaussian time series in this window

    vector<double> times = _travel_times->data();
    vector_range<vector<double> > tau(times, window);
    vector<double> gaussian =
        exp(-0.5 * abs2((tau - delay) / duration)) / (duration * SQRT_TWO_PI);

    // interpolate eigenverb power

    double verb_level = verb->power[0] * transmit->duration;
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
    if (src_level < power_threshold) {
        return;
    }

    // add Gaussian to each receiver channel

    for (int rcv : this->_receiver->rcv_keys()) {
        // compute received level for this transmission

        bp_model::csptr rcv_beam = _receiver->rcv_beam(rcv);
        bvector rcv_arrival(verb->receiver_de, verb->receiver_az);
        rcv_arrival.rotate(_receiver_orient, rcv_arrival);
        bvector rcv_steering = _receiver->rcv_steering(rcv);
        rcv_beam->beam_level(rcv_arrival, frequencies, &level, rcv_steering);
        double rcv_level = src_level + verb_level + level[0];
        if (rcv_level < power_threshold) {
            continue;
        }

        // add scaled Gaussian to each result in time window

        for (size_t n = 0; n < gaussian.size(); ++n) {
            auto t = n + first;
            _time_series(rcv, t) += rcv_level * gaussian[n];
        }
    }
}

/**
 * Writes reverberation time series data to disk.
 */
void rvbts_collection::write_netcdf(const char *filename) const {
	netCDF::NcFile nc_file(filename, netCDF::NcFile::replace);

    auto num_channels = (long)_time_series.size1();
    auto num_times = (long)_time_series.size2();

    // dimensions

    netCDF::NcDim channels_dim = nc_file.addDim("channels", num_channels);
    netCDF::NcDim time_dim = nc_file.addDim("travel_time", num_times);
    const std::vector<netCDF::NcDim> chan_time_dims{channels_dim, time_dim};

    // variables

    // clang-format off
    netCDF::NcVar src_id_var = nc_file.addVar("sourceID", netCDF::NcUint64());
    netCDF::NcVar src_lat_var = nc_file.addVar("source_latitude", netCDF::NcDouble());
    netCDF::NcVar src_lng_var = nc_file.addVar("source_longitude", netCDF::NcDouble());
    netCDF::NcVar src_alt_var = nc_file.addVar("source_altitude", netCDF::NcDouble());

    netCDF::NcVar src_yaw_var = nc_file.addVar("source_yaw", netCDF::NcDouble());
    netCDF::NcVar src_pitch_var = nc_file.addVar("source_pitch", netCDF::NcDouble());
    netCDF::NcVar src_roll_var = nc_file.addVar("source_roll", netCDF::NcDouble());

    netCDF::NcVar src_speed_var = nc_file.addVar("source_speed", netCDF::NcDouble());

    netCDF::NcVar rcv_id_var = nc_file.addVar("receiverID", netCDF::NcUint64());
    netCDF::NcVar rcv_lat_var = nc_file.addVar("receiver_latitude", netCDF::NcDouble());
    netCDF::NcVar rcv_lng_var = nc_file.addVar("receiver_longitude", netCDF::NcDouble());
    netCDF::NcVar rcv_alt_var = nc_file.addVar("receiver_altitude", netCDF::NcDouble());

    netCDF::NcVar rcv_yaw_var = nc_file.addVar("receiver_yaw", netCDF::NcDouble());
    netCDF::NcVar rcv_pitch_var = nc_file.addVar("receiver_pitch", netCDF::NcDouble());
    netCDF::NcVar rcv_roll_var = nc_file.addVar("receiver_roll", netCDF::NcDouble());

    netCDF::NcVar rcv_speed_var = nc_file.addVar("receiver_speed", netCDF::NcDouble());

    netCDF::NcVar channels_var = nc_file.addVar("channels", netCDF::NcDouble(), channels_dim);
    netCDF::NcVar time_var = nc_file.addVar("travel_time", netCDF::NcDouble(), time_dim);
    netCDF::NcVar time_series_var = nc_file.addVar("time_series", netCDF::NcDouble(), chan_time_dims);
    // clang-format on

    // units

    time_var.putAtt("units", "seconds");

    src_yaw_var.putAtt("units", "degrees");
    src_pitch_var.putAtt("units", "degrees");
    src_roll_var.putAtt("units", "degrees");
    src_speed_var.putAtt("units", "m/s");

    rcv_yaw_var.putAtt("units", "degrees");
    rcv_pitch_var.putAtt("units", "degrees");
    rcv_roll_var.putAtt("units", "degrees");
    rcv_speed_var.putAtt("units", "m/s");

    // write source parameters

    // clang-format off
    size_t n;
    double v;
    n = _source->keyID();			src_id_var.putVar(&n);
    v = _source_pos.latitude(); 	src_lat_var.putVar(&v);
    v = _source_pos.longitude();	src_lng_var.putVar(&v);
    v = _source_pos.altitude(); 	src_alt_var.putVar(&v);

    orientation src_orient(_source_orient);
    v = src_orient.yaw(); 			src_yaw_var.putVar(&v);
    v = src_orient.pitch();			src_pitch_var.putVar(&v);
    v = src_orient.roll(); 			src_roll_var.putVar(&v);

    v = _source_speed; 				src_speed_var.putVar(&v);

    // write receiver parameters

    n = _receiver->keyID();			rcv_id_var.putVar(&n);
    v = _receiver_pos.latitude(); 	rcv_lat_var.putVar(&v);
    v = _receiver_pos.longitude();	rcv_lng_var.putVar(&v);
    v = _receiver_pos.altitude(); 	rcv_alt_var.putVar(&v);

    orientation rcv_orient(_receiver_orient);
    v = rcv_orient.yaw(); 			rcv_yaw_var.putVar(&v);
    v = rcv_orient.pitch();			rcv_pitch_var.putVar(&v);
    v = rcv_orient.roll(); 			rcv_roll_var.putVar(&v);

    v = _receiver_speed; 			rcv_speed_var.putVar(&v);
    // clang-format on

    // data

    seq_linear channels(0.0, 1.0, (size_t)num_channels);
    channels_var.putVar(channels.data().begin());
    time_var.putVar(_travel_times->data().begin());
    time_series_var.putVar(_time_series.data().begin());
}
