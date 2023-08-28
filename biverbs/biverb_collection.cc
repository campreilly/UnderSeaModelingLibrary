/*
 * @file biverb_collection.cc
 */

#include <netcdfcpp.h>
#include <usml/biverbs/biverb_collection.h>
#include <usml/biverbs/biverb_model.h>
#include <usml/eigenverbs/eigenverb_model.h>
#include <usml/platforms/platform_manager.h>
#include <usml/threads/read_write_lock.h>

#include <cstddef>
#include <list>
#include <sstream>
#include <string>

using namespace usml::biverbs;
using namespace usml::eigenverbs;
using namespace usml::platforms;

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

    // add to collection

    auto verb = biverb_model::csptr(biverb);
    _collection[interface].insert({verb->time, verb});
}

/**
 * Writes the biverbs for an individual interface to a netcdf file.
 */
void biverb_collection::write_netcdf(const char* filename,
                                     size_t interface) const {
    NcFile nc_file(filename, NcFile::Replace);
    biverb_list list = biverbs(interface);

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

    //    if (!list.empty()) {
    //        // dimensions
    //
    //        NcDim* eigenverb_dim = nc_file.add_dim("eigenverbs",
    //        (long)list.size()); NcDim* freq_dim = nc_file.add_dim(
    //            "frequency", (long)list.begin()->get()->frequencies->size());
    //
    //        // variables
    //
    //        NcVar* time_var =
    //            nc_file.add_var("travel_time", ncDouble, eigenverb_dim);
    //        NcVar* freq_var = nc_file.add_var("frequency", ncDouble,
    //        freq_dim); NcVar* power_var =
    //            nc_file.add_var("power", ncDouble, eigenverb_dim, freq_dim);
    //        NcVar* length_var = nc_file.add_var("length", ncDouble,
    //        eigenverb_dim); NcVar* width_var = nc_file.add_var("width",
    //        ncDouble, eigenverb_dim); NcVar* lat_var =
    //        nc_file.add_var("latitude", ncDouble, eigenverb_dim); NcVar*
    //        lng_var = nc_file.add_var("longitude", ncDouble, eigenverb_dim);
    //        NcVar* alt_var = nc_file.add_var("altitude", ncDouble,
    //        eigenverb_dim); NcVar* direction_var =
    //            nc_file.add_var("direction", ncDouble, eigenverb_dim);
    //        NcVar* grazing_var =
    //            nc_file.add_var("grazing_angle", ncDouble, eigenverb_dim);
    //        NcVar* sound_speed_var =
    //            nc_file.add_var("sound_speed", ncDouble, eigenverb_dim);
    //        NcVar* de_index_var =
    //            nc_file.add_var("de_index", ncShort, eigenverb_dim);
    //        NcVar* az_index_var =
    //            nc_file.add_var("az_index", ncShort, eigenverb_dim);
    //        NcVar* source_de_var =
    //            nc_file.add_var("source_de", ncDouble, eigenverb_dim);
    //        NcVar* source_az_var =
    //            nc_file.add_var("source_az", ncDouble, eigenverb_dim);
    //        NcVar* surface_var = nc_file.add_var("surface", ncShort,
    //        eigenverb_dim); NcVar* bottom_var = nc_file.add_var("bottom",
    //        ncShort, eigenverb_dim); NcVar* caustic_var =
    //        nc_file.add_var("caustic", ncShort, eigenverb_dim); NcVar*
    //        upper_var = nc_file.add_var("upper", ncShort, eigenverb_dim);
    //        NcVar* lower_var = nc_file.add_var("lower", ncShort,
    //        eigenverb_dim); NcVar* north_var =
    //            nc_file.add_var("bounding_north", ncDouble, eigenverb_dim);
    //        NcVar* south_var =
    //            nc_file.add_var("bounding_south", ncDouble, eigenverb_dim);
    //        NcVar* east_var =
    //            nc_file.add_var("bounding_east", ncDouble, eigenverb_dim);
    //        NcVar* west_var =
    //            nc_file.add_var("bounding_west", ncDouble, eigenverb_dim);
    //
    //        // units
    //
    //        time_var->add_att("units", "seconds");
    //        freq_var->add_att("units", "hertz");
    //        power_var->add_att("units", "dB");
    //        length_var->add_att("units", "meters");
    //        width_var->add_att("units", "meters");
    //        lat_var->add_att("units", "degrees_north");
    //        lng_var->add_att("units", "degrees_east");
    //        alt_var->add_att("units", "meters");
    //        direction_var->add_att("units", "degrees_true");
    //        direction_var->add_att("positive", "clockwise");
    //        grazing_var->add_att("units", "degrees");
    //        grazing_var->add_att("positive", "up");
    //        sound_speed_var->add_att("units", "m/s");
    //        de_index_var->add_att("units", "count");
    //        az_index_var->add_att("units", "count");
    //        source_de_var->add_att("units", "degrees");
    //        source_de_var->add_att("positive", "up");
    //        source_az_var->add_att("units", "degrees_true");
    //        source_az_var->add_att("positive", "clockwise");
    //        surface_var->add_att("units", "count");
    //        bottom_var->add_att("units", "count");
    //        caustic_var->add_att("units", "count");
    //        upper_var->add_att("units", "count");
    //        lower_var->add_att("units", "count");
    //        north_var->add_att("units", "degrees_north");
    //        south_var->add_att("units", "degrees_north");
    //        east_var->add_att("units", "degrees_east");
    //        west_var->add_att("units", "degrees_east");
    //
    //        // data
    //
    //        freq_var->put(list.begin()->get()->frequencies->data().begin(),
    //                      (long)list.begin()->get()->frequencies->size());
    //        int record = 0;  // current record
    //        for (const auto& verb : list) {
    //            // sets current index
    //
    //            time_var->set_cur(record);
    //            power_var->set_cur(record);
    //            length_var->set_cur(record);
    //            width_var->set_cur(record);
    //            lat_var->set_cur(record);
    //            lng_var->set_cur(record);
    //            alt_var->set_cur(record);
    //            direction_var->set_cur(record);
    //            grazing_var->set_cur(record);
    //            sound_speed_var->set_cur(record);
    //            de_index_var->set_cur(record);
    //            az_index_var->set_cur(record);
    //            source_de_var->set_cur(record);
    //            source_az_var->set_cur(record);
    //            surface_var->set_cur(record);
    //            bottom_var->set_cur(record);
    //            caustic_var->set_cur(record);
    //            upper_var->set_cur(record);
    //            lower_var->set_cur(record);
    //            north_var->set_cur(record);
    //            south_var->set_cur(record);
    //            east_var->set_cur(record);
    //            west_var->set_cur(record);
    //            ++record;
    //
    //            // inserts data
    //
    //            double v;
    //            long i;
    //            time_var->put(&verb->time, 1);
    //
    //            vector<double> power = 10.0 * log10(max(verb->power, 1e-30));
    //            power_var->put(power.data().begin(), 1,
    //                           (long)verb->frequencies->size());
    //
    //            // clang-format off
    //            v = verb->length;
    //            length_var->put(&v, 1); v = verb->width;
    //            width_var->put(&v, 1); v = verb->position.latitude();
    //            lat_var->put(&v, 1); v = verb->position.longitude();
    //            lng_var->put(&v, 1); v = verb->position.altitude();
    //            alt_var->put(&v, 1); v = to_degrees(verb->direction);
    //            direction_var->put(&v, 1); v = to_degrees(verb->grazing);
    //            grazing_var->put(&v, 1); v = verb->sound_speed;
    //            sound_speed_var->put(&v, 1); i = (long)verb->de_index;
    //            de_index_var->put(&i, 1); i = (long)verb->az_index;
    //            az_index_var->put(&i, 1); v = to_degrees(verb->source_de);
    //            source_de_var->put(&v, 1); v = to_degrees(verb->source_az);
    //            source_az_var->put(&v, 1); i = (long)verb->surface;
    //            surface_var->put(&i, 1); i = (long)verb->bottom;
    //            bottom_var->put(&i, 1); i = (long)verb->caustic;
    //            caustic_var->put(&i, 1); i = (long)verb->upper;
    //            upper_var->put(&i, 1); i = (long)verb->lower;
    //            lower_var->put(&i, 1); v = verb->bounding_box.north;
    //            north_var->put(&v, 1); v = verb->bounding_box.south;
    //            south_var->put(&v, 1); v = verb->bounding_box.east;
    //            east_var->put(&v, 1); v = verb->bounding_box.west;
    //            west_var->put(&v, 1);
    //            // clang-format on
    //        }
    //    }
}
