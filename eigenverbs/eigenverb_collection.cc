/**
 * @file eigenverb_collection.cc
 * Collection of eigenverbs in the form of a vector of eigenverb_lists.
 */

#include <ncvalues.h>
#include <netcdfcpp.h>
#include <usml/eigenverbs/eigenverb_collection.h>
#include <usml/types/seq_data.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition1.h>
#include <usml/ublas/math_traits.h>
#include <usml/ublas/vector_math.h>

#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/index/detail/predicates.hpp>
#include <boost/geometry/index/predicates.hpp>
#include <boost/numeric/ublas/storage.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_expression.hpp>
#include <cmath>
#include <iostream>
#include <iterator>
#include <list>
#include <sstream>
#include <string>

using namespace usml::eigenverbs;

/**
 * Scale factor for size of search area in find_eigenverbs().
 */
double eigenverb_collection::search_scale = 1.5;

/**
 * Creates list of eigenverbs for a specific interface.
 */
eigenverb_list eigenverb_collection::eigenverbs(size_t interface) const {
    read_lock_guard guard(_mutex);
    eigenverb_list list;
    for (const auto& pair : _collection[interface]) {
        list.push_back(pair.second);
    }
    return list;
}

/**
 * Adds a new eigenverb to this collection.
 */
void eigenverb_collection::add_eigenverb(eigenverb_model::csptr verb,
                                         size_t interface) {
    write_lock_guard guard(_mutex);
    eigenverb_collection::point center(verb->position.latitude(),
                                       verb->position.longitude());
    _collection[interface].insert(eigenverb_collection::pair(center, verb));
}

/**
 * Finds all of the eigenverbs that intersect the requested area.
 */
eigenverb_list eigenverb_collection::find_eigenverbs(
    const eigenverb_model::csptr& bounding_verb, size_t interface) const {
    read_lock_guard guard(_mutex);

    // compute size of search area

    auto& pos = bounding_verb->position;
    auto direction = bounding_verb->direction;
    wposition1 posA(pos, search_scale * bounding_verb->length, direction);
    wposition1 posB(pos, search_scale * bounding_verb->width,
                    direction + M_PI_2);
    wposition1 posC(pos, search_scale * bounding_verb->length,
                    direction + M_PI);
    wposition1 posD(pos, search_scale * bounding_verb->width,
                    direction + M_PI + M_PI_2);
    bgm::polygon<point> search_area{{{posA.latitude(), posA.longitude()},
                                     {posB.latitude(), posB.longitude()},
                                     {posC.latitude(), posC.longitude()},
                                     {posD.latitude(), posD.longitude()}}};

    // find eigenverbs whose position is within this box

    std::list<eigenverb_collection::pair> pair_list;
    _collection[interface].query(bgi::within(search_area),
                                 std::back_inserter(pair_list));

    // translate to output structure

    eigenverb_list list;
    for (const auto& pair : pair_list) {
        list.push_back(pair.second);
    }
    return list;
}

/**
 * Writes the eigenverbs for an individual interface to a netcdf file.
 */
void eigenverb_collection::write_netcdf(const char* filename,
                                        size_t interface) const {
    NcFile nc_file(filename, NcFile::Replace);
    eigenverb_list list = eigenverbs(interface);

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

    if (!list.empty()) {
        // dimensions

        NcDim* eigenverb_dim = nc_file.add_dim("eigenverbs", (long)list.size());
        NcDim* freq_dim = nc_file.add_dim(
            "frequencies", (long)list.begin()->get()->frequencies->size());

        // variables

        NcVar* time_var =
            nc_file.add_var("travel_time", ncDouble, eigenverb_dim);
        NcVar* freq_var = nc_file.add_var("frequencies", ncDouble, freq_dim);
        NcVar* power_var =
            nc_file.add_var("power", ncDouble, eigenverb_dim, freq_dim);
        NcVar* length_var = nc_file.add_var("length", ncDouble, eigenverb_dim);
        NcVar* width_var = nc_file.add_var("width", ncDouble, eigenverb_dim);
        NcVar* lat_var = nc_file.add_var("latitude", ncDouble, eigenverb_dim);
        NcVar* lng_var = nc_file.add_var("longitude", ncDouble, eigenverb_dim);
        NcVar* alt_var = nc_file.add_var("altitude", ncDouble, eigenverb_dim);
        NcVar* direction_var =
            nc_file.add_var("direction", ncDouble, eigenverb_dim);
        NcVar* grazing_var =
            nc_file.add_var("grazing", ncDouble, eigenverb_dim);
        NcVar* sound_speed_var =
            nc_file.add_var("sound_speed", ncDouble, eigenverb_dim);
        NcVar* de_index_var =
            nc_file.add_var("de_index", ncShort, eigenverb_dim);
        NcVar* az_index_var =
            nc_file.add_var("az_index", ncShort, eigenverb_dim);
        NcVar* source_de_var =
            nc_file.add_var("source_de", ncDouble, eigenverb_dim);
        NcVar* source_az_var =
            nc_file.add_var("source_az", ncDouble, eigenverb_dim);
        NcVar* surface_var = nc_file.add_var("surface", ncShort, eigenverb_dim);
        NcVar* bottom_var = nc_file.add_var("bottom", ncShort, eigenverb_dim);
        NcVar* caustic_var = nc_file.add_var("caustic", ncShort, eigenverb_dim);
        NcVar* upper_var = nc_file.add_var("upper", ncShort, eigenverb_dim);
        NcVar* lower_var = nc_file.add_var("lower", ncShort, eigenverb_dim);

        // units

        time_var->add_att("units", "seconds");
        freq_var->add_att("units", "hertz");
        power_var->add_att("units", "dB");
        length_var->add_att("units", "meters");
        width_var->add_att("units", "meters");
        lat_var->add_att("units", "degrees_north");
        lng_var->add_att("units", "degrees_east");
        alt_var->add_att("units", "meters");
        direction_var->add_att("units", "degrees_true");
        direction_var->add_att("positive", "clockwise");
        grazing_var->add_att("units", "degrees");
        grazing_var->add_att("positive", "up");
        sound_speed_var->add_att("units", "m/s");
        de_index_var->add_att("units", "count");
        az_index_var->add_att("units", "count");
        source_de_var->add_att("units", "degrees");
        source_de_var->add_att("positive", "up");
        source_az_var->add_att("units", "degrees_true");
        source_az_var->add_att("positive", "clockwise");
        surface_var->add_att("units", "count");
        bottom_var->add_att("units", "count");
        caustic_var->add_att("units", "count");
        upper_var->add_att("units", "count");
        lower_var->add_att("units", "count");

        // data

        freq_var->put(list.begin()->get()->frequencies->data().begin(),
                      (long)list.begin()->get()->frequencies->size());
        int record = 0;  // current record
        for (const auto& verb : list) {
            // sets current index

            time_var->set_cur(record);
            power_var->set_cur(record);
            length_var->set_cur(record);
            width_var->set_cur(record);
            lat_var->set_cur(record);
            lng_var->set_cur(record);
            alt_var->set_cur(record);
            direction_var->set_cur(record);
            grazing_var->set_cur(record);
            sound_speed_var->set_cur(record);
            de_index_var->set_cur(record);
            az_index_var->set_cur(record);
            source_de_var->set_cur(record);
            source_az_var->set_cur(record);
            surface_var->set_cur(record);
            bottom_var->set_cur(record);
            caustic_var->set_cur(record);
            upper_var->set_cur(record);
            lower_var->set_cur(record);
            ++record;

            // inserts data

            double v;
            long i;
            time_var->put(&verb->travel_time, 1);

            vector<double> power = 10.0 * log10(max(verb->power, 1e-30));
            power_var->put(power.data().begin(), 1,
                           (long)verb->frequencies->size());

            // clang-format off
            v = verb->length;					length_var->put(&v, 1);
            v = verb->width;					width_var->put(&v, 1);
            v = verb->position.latitude();		lat_var->put(&v, 1);
            v = verb->position.longitude();		lng_var->put(&v, 1);
            v = verb->position.altitude();		alt_var->put(&v, 1);
            v = to_degrees(verb->direction);	direction_var->put(&v, 1);
            v = to_degrees(verb->grazing);		grazing_var->put(&v, 1);
            v = verb->sound_speed;				sound_speed_var->put(&v, 1);
            i = (long)verb->de_index;           de_index_var->put(&i, 1);
            i = (long)verb->az_index;           az_index_var->put(&i, 1);
            v = to_degrees(verb->source_de);    source_de_var->put(&v, 1);
            v = to_degrees(verb->source_az);    source_az_var->put(&v, 1);
            i = (long)verb->surface;            surface_var->put(&i, 1);
            i = (long)verb->bottom;             bottom_var->put(&i, 1);
            i = (long)verb->caustic;            caustic_var->put(&i, 1);
            i = (long)verb->upper;              upper_var->put(&i, 1);
            i = (long)verb->lower;              lower_var->put(&i, 1);
            // clang-format on
        }
    }
}

/**
 * Reads the eigenverbs for a single interface from a netcdf file.
 */
void eigenverb_collection::read_netcdf(const char* filename, size_t interface) {
    // open file

    NcFile nc_file(filename, NcFile::ReadOnly);
    if (nc_file.is_valid() == 0U) {
        cout << "Could not open " << filename << endl;
        return;
    }

    // dimensions

    NcDim* eigenverb_dim = nc_file.get_dim("eigenverbs");
    long num_eigenverbs = eigenverb_dim->size();
    NcDim* freq_dim = nc_file.get_dim("frequency");
    long num_freq = freq_dim->size();

    // variables

    NcVar* time_var = nc_file.get_var("travel_time");
    NcVar* freq_var = nc_file.get_var("frequency");
    NcVar* power_var = nc_file.get_var("power");
    NcVar* length_var = nc_file.get_var("length");
    NcVar* width_var = nc_file.get_var("width");
    NcVar* lat_var = nc_file.get_var("latitude");
    NcVar* lng_var = nc_file.get_var("longitude");
    NcVar* alt_var = nc_file.get_var("altitude");
    NcVar* direction_var = nc_file.get_var("direction");
    NcVar* grazing_var = nc_file.get_var("grazing_angle");
    NcVar* sound_speed_var = nc_file.get_var("sound_speed");
    NcVar* de_index_var = nc_file.get_var("de_index");
    NcVar* az_index_var = nc_file.get_var("az_index");
    NcVar* source_de_var = nc_file.get_var("source_de");
    NcVar* source_az_var = nc_file.get_var("source_az");
    NcVar* surface_var = nc_file.get_var("surface");
    NcVar* bottom_var = nc_file.get_var("bottom");
    NcVar* caustic_var = nc_file.get_var("caustic");
    NcVar* upper_var = nc_file.get_var("upper");
    NcVar* lower_var = nc_file.get_var("lower");

    // get frequencies just once

    auto* freq_data = new double[num_freq];
    double* freq_accessor = freq_data;

    for (int rec = 0; rec < num_freq; ++rec) {
        freq_var->set_cur(rec);
        freq_var->get(freq_accessor++, 1, num_freq);
    }
    seq_vector::csptr frequencies(new seq_data(freq_data, num_freq));

    delete[] freq_data;

    // read each record

    auto* data = new double[num_freq];
    for (int rec = 0; rec < num_eigenverbs; ++rec) {
        auto* verb = new eigenverb_model();
        verb->power.resize(num_freq, true);

        // sets current index

        time_var->set_cur(rec);
        power_var->set_cur(rec, 0);
        length_var->set_cur(rec);
        width_var->set_cur(rec);
        lat_var->set_cur(rec);
        lng_var->set_cur(rec);
        alt_var->set_cur(rec);
        direction_var->set_cur(rec);
        grazing_var->set_cur(rec);
        sound_speed_var->set_cur(rec);
        de_index_var->set_cur(rec);
        az_index_var->set_cur(rec);
        source_de_var->set_cur(rec);
        source_az_var->set_cur(rec);
        surface_var->set_cur(rec);
        bottom_var->set_cur(rec);
        caustic_var->set_cur(rec);
        upper_var->set_cur(rec);
        lower_var->set_cur(rec);
        ++rec;

        // get data

        double v;
        long i;
        time_var->get(&v, 1);
        verb->travel_time = v;
        verb->frequencies = frequencies;

        power_var->get(data, 1, num_freq);
        for (size_t n = 0; n < num_freq; ++n) {
            verb->power[n] = pow(10.0, data[n] / 10.0);
        }

        // clang-format off
        length_var->get(&v, 1);        	verb->length = v;
        width_var->get(&v, 1);          verb->width = v;
        lat_var->get(&v, 1);            verb->position.latitude(v);
        lng_var->get(&v, 1);            verb->position.longitude(v);
        alt_var->get(&v, 1);            verb->position.altitude(v);
        direction_var->get(&v, 1);      verb->direction = to_radians(v);
        grazing_var->get(&v, 1);        verb->grazing = to_radians(v);
        sound_speed_var->get(&v, 1); 	verb->sound_speed = v;
        de_index_var->get(&i, 1);       verb->de_index = i;
        az_index_var->get(&i, 1);       verb->az_index = i;
        source_de_var->get(&v, 1);      verb->source_de = to_radians(v);
        source_az_var->get(&v, 1);      verb->source_az = to_radians(v);
        surface_var->get(&i, 1);        verb->surface = int(i);
        bottom_var->get(&i, 1);         verb->bottom = int(i);
        caustic_var->get(&i, 1);        verb->caustic = int(i);
        upper_var->get(&i, 1);          verb->upper = int(i);
        lower_var->get(&i, 1);          verb->lower = int(i);
        // clang-format on

        add_eigenverb(eigenverb_model::csptr(verb), interface);
    }
    delete[] data;
}
