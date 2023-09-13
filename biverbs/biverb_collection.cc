/*
 * @file biverb_collection.cc
 */

#include <netcdfcpp.h>
#include <usml/biverbs/biverb_collection.h>
#include <usml/biverbs/biverb_model.h>
#include <usml/eigenverbs/eigenverb_model.h>
#include <usml/sensors/sensor_manager.h>
#include <usml/threads/read_write_lock.h>

#include <cstddef>
#include <list>
#include <sstream>
#include <string>

using namespace usml::biverbs;

#define DEBUG_BIVERB

/**
 * Threshold for minimum biverb power.
 */
double biverb_collection::power_threshold = 1e-20;

/**
 * Creates list of biverbs for a specific interface.
 */
biverb_list biverb_collection::biverbs(size_t interface) const {
    read_lock_guard guard(_mutex);
    biverb_list list;
    for (const auto& pair : _collection[interface]) {
        list.push_back(pair.second);
    }
    return list;
}

/**
 * Adds a new biverb to this collection.
 */
void biverb_collection::add_biverb(const eigenverb_model::csptr& src_verb,
                                   const eigenverb_model::csptr& rcv_verb,
                                   const vector<double>& scatter,
                                   size_t interface) {
    write_lock_guard guard(_mutex);

    // determine relative range and bearing between Gaussians

    double bearing;
    const double range =
        rcv_verb->position.gc_range(src_verb->position, &bearing);

    if (range < 1e-6) {
        bearing = 0;  // fixes bearing = NaN
    }
    bearing -= rcv_verb->direction;  // relative bearing

    const double ys = range * cos(bearing);
    const double ys2 = ys * ys;

    const double xs = range * sin(bearing);
    const double xs2 = xs * xs;

#ifdef DEBUG_BIVERB
    cout << "biverb_generator::compute_overlap() " << endl
         << "\txs2=" << xs2 << " ys2=" << ys2 << " scatter=" << scatter << endl
         << "\tsrc_verb"
         << " t=" << src_verb->travel_time
         << " de=" << to_degrees(src_verb->source_de)
         << " az=" << to_degrees(src_verb->source_az)
         << " direction=" << to_degrees(src_verb->direction)
         << " grazing=" << to_degrees(src_verb->grazing) << endl
         << "\tpower=" << 10.0 * log10(src_verb->power)
         << " length=" << src_verb->length << " width=" << src_verb->width
         << " surface=" << src_verb->surface << " bottom=" << src_verb->bottom
         << " caustic=" << src_verb->caustic << endl
         << "\trcv_verb"
         << " t=" << rcv_verb->travel_time
         << " de=" << to_degrees(rcv_verb->source_de)
         << " az=" << to_degrees(rcv_verb->source_az)
         << " direction=" << to_degrees(rcv_verb->direction)
         << " grazing=" << to_degrees(rcv_verb->grazing) << endl
         << "\tpower=" << 10.0 * log10(rcv_verb->power)
         << " length=" << rcv_verb->length << " width=" << rcv_verb->width
         << " surface=" << rcv_verb->surface << " bottom=" << rcv_verb->bottom
         << " caustic=" << rcv_verb->caustic << endl;
#endif
    // copy data from source and receiver eigenverbs

    auto* biverb = new biverb_model();
    biverb->travel_time = src_verb->travel_time + rcv_verb->travel_time;
    biverb->frequencies = rcv_verb->frequencies;
    biverb->de_index = rcv_verb->de_index;
    biverb->az_index = rcv_verb->az_index;

    biverb->source_de = src_verb->source_de;
    biverb->source_az = src_verb->source_az;

    biverb->source_surface = src_verb->surface;
    biverb->source_bottom = src_verb->bottom;
    biverb->source_caustic = src_verb->caustic;
    biverb->source_upper = src_verb->upper;
    biverb->source_lower = src_verb->lower;

    biverb->receiver_de = rcv_verb->source_de;
    biverb->receiver_az = rcv_verb->source_az;

    biverb->receiver_surface = rcv_verb->surface;
    biverb->receiver_bottom = rcv_verb->bottom;
    biverb->receiver_caustic = rcv_verb->caustic;
    biverb->receiver_upper = rcv_verb->upper;
    biverb->receiver_lower = rcv_verb->lower;

    // determine the relative tilt between the projected Gaussians

    const double alpha = src_verb->direction - rcv_verb->direction;
    const double cos2alpha = cos(2.0 * alpha);
    const double sin2alpha = sin(2.0 * alpha);

    // compute commonly used terms in the intersection of the Gaussian
    // profiles

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
    cout << "\tdet_sr=" << det_sr << " kappa=" << kappa
         << " scatter=" << (10.0 * log10(scatter))
         << " power=" << (10.0 * log10(biverb->power)) << endl;
#endif
    const double overlap_scale = exp(kappa) / sqrt(det_sr);
    biverb->power *= overlap_scale;

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

    // add to collection

    if (norm_inf(biverb->power) >= power_threshold) {
        auto verb = biverb_model::csptr(biverb);
        _collection[interface].insert({verb->travel_time, verb});
    }
}

/**
 * Writes the biverbs for an individual interface to a netcdf file.
 */
void biverb_collection::write_netcdf(const char* filename,
                                     size_t interface) const {
    read_lock_guard guard(_mutex);
    NcFile nc_file(filename, NcFile::Replace);

    switch (interface) {
        case eigenverb_model::BOTTOM:
            nc_file.add_att("long_name", "bottom eigenverbs");
            break;
        case eigenverb_model::SURFACE:
            nc_file.add_att("long_name", "surface eigenverbs");
            break;
        case eigenverb_model::VOLUME_UPPER:
            nc_file.add_att("long_name", "upper volume eigenverbs");
            nc_file.add_att("layer", 1);
            break;
        case eigenverb_model::VOLUME_LOWER:
            nc_file.add_att("long_name", "lower volume eigenverbs");
            nc_file.add_att("layer", 1);
            break;
        default: {
            size_t layer = interface - eigenverb_model::VOLUME_UPPER;
            size_t side = layer % 2;
            layer = (layer / 2) + 1;
            std::ostringstream oss;
            oss << ((side) != 0U ? "lower" : "upper") << " volume " << layer
                << " eigenverbs";
            nc_file.add_att("long_name", oss.str().c_str());
            nc_file.add_att("layer", (long)layer);
        } break;
    }

    biverb_list list = biverbs(interface);
    if (list.empty()) {
        return;
    }

    // dimensions

    NcDim* eigenverb_dim = nc_file.add_dim("eigenverbs", (long)list.size());
    NcDim* freq_dim = nc_file.add_dim(
        "frequencies", (long)list.begin()->get()->frequencies->size());

    // variables

    // clang-format off
	NcVar* time_var = nc_file.add_var("travel_time", ncDouble, eigenverb_dim);
	NcVar* freq_var = nc_file.add_var("frequencies", ncDouble, freq_dim);
	NcVar* power_var = nc_file.add_var("power", ncDouble, eigenverb_dim, freq_dim);
	NcVar* duration_var = nc_file.add_var("duration", ncDouble, eigenverb_dim);
	NcVar* de_index_var = nc_file.add_var("de_index", ncShort, eigenverb_dim);
	NcVar* az_index_var = nc_file.add_var("az_index", ncShort, eigenverb_dim);

	NcVar* source_de_var = nc_file.add_var("source_de", ncDouble, eigenverb_dim);
	NcVar* source_az_var = nc_file.add_var("source_az", ncDouble, eigenverb_dim);
	NcVar* source_surface_var = nc_file.add_var("source_surface", ncShort, eigenverb_dim);
	NcVar* source_bottom_var = nc_file.add_var("source_bottom", ncShort, eigenverb_dim);
	NcVar* source_caustic_var = nc_file.add_var("source_caustic", ncShort, eigenverb_dim);
	NcVar* source_upper_var = nc_file.add_var("source_upper", ncShort, eigenverb_dim);
	NcVar* source_lower_var = nc_file.add_var("source_lower", ncShort, eigenverb_dim);

	NcVar* receiver_de_var = nc_file.add_var("receiver_de", ncDouble, eigenverb_dim);
	NcVar* receiver_az_var = nc_file.add_var("receiver_az", ncDouble, eigenverb_dim);
	NcVar* receiver_surface_var = nc_file.add_var("receiver_surface", ncShort, eigenverb_dim);
	NcVar* receiver_bottom_var = nc_file.add_var("receiver_bottom", ncShort, eigenverb_dim);
	NcVar* receiver_caustic_var = nc_file.add_var("receiver_caustic", ncShort, eigenverb_dim);
	NcVar* receiver_upper_var = nc_file.add_var("receiver_upper", ncShort, eigenverb_dim);
	NcVar* receiver_lower_var = nc_file.add_var("receiver_lower", ncShort, eigenverb_dim);
    // clang-format on

    // units

    time_var->add_att("units", "seconds");
    freq_var->add_att("units", "hertz");
    power_var->add_att("units", "dB");
    duration_var->add_att("units", "s");
    de_index_var->add_att("units", "count");
    az_index_var->add_att("units", "count");

    source_de_var->add_att("units", "degrees");
    source_de_var->add_att("positive", "up");
    source_az_var->add_att("units", "degrees_true");
    source_az_var->add_att("positive", "clockwise");

    source_surface_var->add_att("units", "count");
    source_bottom_var->add_att("units", "count");
    source_caustic_var->add_att("units", "count");
    source_upper_var->add_att("units", "count");
    source_lower_var->add_att("units", "count");

    receiver_de_var->add_att("units", "degrees");
    receiver_de_var->add_att("positive", "up");
    receiver_az_var->add_att("units", "degrees_true");
    receiver_az_var->add_att("positive", "clockwise");

    receiver_surface_var->add_att("units", "count");
    receiver_bottom_var->add_att("units", "count");
    receiver_caustic_var->add_att("units", "count");
    receiver_upper_var->add_att("units", "count");
    receiver_lower_var->add_att("units", "count");

    // data

    freq_var->put(list.begin()->get()->frequencies->data().begin(),
                  (long)list.begin()->get()->frequencies->size());
    int record = 0;  // current record
    for (const auto& verb : list) {
        // sets current index

        time_var->set_cur(record);
        power_var->set_cur(record);
        duration_var->set_cur(record);

        de_index_var->set_cur(record);
        az_index_var->set_cur(record);

        source_de_var->set_cur(record);
        source_az_var->set_cur(record);

        source_surface_var->set_cur(record);
        source_bottom_var->set_cur(record);
        source_caustic_var->set_cur(record);
        source_upper_var->set_cur(record);
        source_lower_var->set_cur(record);

        source_de_var->set_cur(record);
        source_az_var->set_cur(record);

        source_surface_var->set_cur(record);
        source_bottom_var->set_cur(record);
        source_caustic_var->set_cur(record);
        source_upper_var->set_cur(record);
        source_lower_var->set_cur(record);

        receiver_de_var->set_cur(record);
        receiver_az_var->set_cur(record);

        receiver_surface_var->set_cur(record);
        receiver_bottom_var->set_cur(record);
        receiver_caustic_var->set_cur(record);
        receiver_upper_var->set_cur(record);
        receiver_lower_var->set_cur(record);

        ++record;

        // inserts data

        double v;
        long i;
        time_var->put(&verb->travel_time, 1);

        vector<double> power = 10.0 * log10(max(verb->power, 1e-30));
        power_var->put(power.data().begin(), 1,
                       (long)verb->frequencies->size());

        // clang-format off
		v = verb->duration; 				duration_var->put(&v, 1);
		i = (long)verb->de_index; 			de_index_var->put(&i, 1);
		i = (long)verb->az_index; 			az_index_var->put(&i, 1);

		v = to_degrees(verb->source_de); 	source_de_var->put(&v, 1);
		v = to_degrees(verb->source_az); 	source_az_var->put(&v, 1);

		i = (long)verb->source_surface;		source_surface_var->put(&i, 1);
		i = (long)verb->source_bottom;		source_bottom_var->put(&i, 1);
		i = (long)verb->source_caustic;		source_caustic_var->put(&i, 1);
		i = (long)verb->source_upper;		source_upper_var->put(&i, 1);
		i = (long)verb->source_lower;		source_lower_var->put(&i, 1);

		v = to_degrees(verb->receiver_de); 	receiver_de_var->put(&v, 1);
		v = to_degrees(verb->receiver_az); 	receiver_az_var->put(&v, 1);

		i = (long)verb->receiver_surface;	receiver_surface_var->put(&i, 1);
		i = (long)verb->receiver_bottom;	receiver_bottom_var->put(&i, 1);
		i = (long)verb->receiver_caustic;	receiver_caustic_var->put(&i, 1);
		i = (long)verb->receiver_upper;		receiver_upper_var->put(&i, 1);
		i = (long)verb->receiver_lower;		receiver_lower_var->put(&i, 1);
        // clang-format on
    }
}
