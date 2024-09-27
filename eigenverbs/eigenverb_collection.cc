/**
 * @file eigenverb_collection.cc
 * Collection of eigenverbs in the form of a vector of eigenverb_lists.
 */

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
#include <netcdf>
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
    netCDF::NcFile nc_file(filename, netCDF::NcFile::replace);
    eigenverb_list list = eigenverbs(interface);
    size_t num_freq = list.begin()->get()->frequencies->size();

    switch (interface) {
        case eigenverb_model::BOTTOM:
            nc_file.putAtt("long_name", "bottom eigenverbs");
            break;
        case eigenverb_model::SURFACE:
            nc_file.putAtt("long_name", "surface eigenverbs");
            break;
        case eigenverb_model::VOLUME_UPPER:
            nc_file.putAtt("long_name", "upper volume eigenverbs");
            nc_file.putAtt("layer", netCDF::NcInt(), 1);
            break;
        case eigenverb_model::VOLUME_LOWER:
            nc_file.putAtt("long_name", "lower volume eigenverbs");
            nc_file.putAtt("layer", netCDF::NcInt(), 1);
            break;
        default: {
            size_t layer = interface - eigenverb_model::VOLUME_UPPER;
            size_t side = layer % 2;
            layer = (layer / 2) + 1;
            std::ostringstream oss;
            oss << ((side) != 0U ? "lower" : "upper") << " volume " << layer
                << " eigenverbs";
            nc_file.putAtt("long_name", oss.str());
            nc_file.putAtt("layer", netCDF::NcInt(), (int)layer);
        } break;
    }

    if (!list.empty()) {
        // clang-format off
        // dimensions

        netCDF::NcDim eigenverb_dim = nc_file.addDim("eigenverbs", list.size());
        netCDF::NcDim freq_dim = nc_file.addDim("frequencies", num_freq);
        const std::vector<netCDF::NcDim> verb_freq_dims{eigenverb_dim, freq_dim};

        // variables

        netCDF::NcVar time_var = nc_file.addVar("travel_time", netCDF::NcDouble(), eigenverb_dim);
        netCDF::NcVar freq_var = nc_file.addVar("frequencies", netCDF::NcDouble(), freq_dim);
        netCDF::NcVar power_var = nc_file.addVar("power", netCDF::NcDouble(), verb_freq_dims);
        netCDF::NcVar length_var = nc_file.addVar("length", netCDF::NcDouble(), eigenverb_dim);
        netCDF::NcVar width_var = nc_file.addVar("width", netCDF::NcDouble(), eigenverb_dim);
        netCDF::NcVar lat_var = nc_file.addVar("latitude", netCDF::NcDouble(), eigenverb_dim);
        netCDF::NcVar lng_var = nc_file.addVar("longitude", netCDF::NcDouble(), eigenverb_dim);
        netCDF::NcVar alt_var = nc_file.addVar("altitude", netCDF::NcDouble(), eigenverb_dim);
        netCDF::NcVar direction_var = nc_file.addVar("direction", netCDF::NcDouble(), eigenverb_dim);
        netCDF::NcVar grazing_var = nc_file.addVar("grazing", netCDF::NcDouble(), eigenverb_dim);
        netCDF::NcVar sound_speed_var = nc_file.addVar("sound_speed", netCDF::NcDouble(), eigenverb_dim);
        netCDF::NcVar de_index_var = nc_file.addVar("de_index", netCDF::NcShort(), eigenverb_dim);
        netCDF::NcVar az_index_var = nc_file.addVar("az_index", netCDF::NcShort(), eigenverb_dim);
        netCDF::NcVar source_de_var = nc_file.addVar("source_de", netCDF::NcDouble(), eigenverb_dim);
        netCDF::NcVar source_az_var = nc_file.addVar("source_az", netCDF::NcDouble(), eigenverb_dim);
        netCDF::NcVar surface_var = nc_file.addVar("surface", netCDF::NcShort(), eigenverb_dim);
        netCDF::NcVar bottom_var = nc_file.addVar("bottom", netCDF::NcShort(), eigenverb_dim);
        netCDF::NcVar caustic_var = nc_file.addVar("caustic", netCDF::NcShort(), eigenverb_dim);
        netCDF::NcVar upper_var = nc_file.addVar("upper", netCDF::NcShort(), eigenverb_dim);
        netCDF::NcVar lower_var = nc_file.addVar("lower", netCDF::NcShort(), eigenverb_dim);
        // clang-format on

        // units

        time_var.putAtt("units", "seconds");
        freq_var.putAtt("units", "hertz");
        power_var.putAtt("units", "dB");
        length_var.putAtt("units", "meters");
        width_var.putAtt("units", "meters");
        lat_var.putAtt("units", "degrees_north");
        lng_var.putAtt("units", "degrees_east");
        alt_var.putAtt("units", "meters");
        direction_var.putAtt("units", "degrees_true");
        direction_var.putAtt("positive", "clockwise");
        grazing_var.putAtt("units", "degrees");
        grazing_var.putAtt("positive", "up");
        sound_speed_var.putAtt("units", "m/s");
        de_index_var.putAtt("units", "count");
        az_index_var.putAtt("units", "count");
        source_de_var.putAtt("units", "degrees");
        source_de_var.putAtt("positive", "up");
        source_az_var.putAtt("units", "degrees_true");
        source_az_var.putAtt("positive", "clockwise");
        surface_var.putAtt("units", "count");
        bottom_var.putAtt("units", "count");
        caustic_var.putAtt("units", "count");
        upper_var.putAtt("units", "count");
        lower_var.putAtt("units", "count");

        // data

        freq_var.putVar(list.begin()->get()->frequencies->data().begin());

        int record = 0;  // current record
        std::vector<size_t> index(1);
        std::vector<size_t> rec_freq_index(2);
        rec_freq_index[0] = 0;
        rec_freq_index[1] = 0;
        std::vector<size_t> rec_freq_count(2);
        rec_freq_count[0] = 1;
        rec_freq_count[1] = num_freq;

        for (const auto& verb : list) {
            double v;
            long i;
            size_t s;
            index[0] = record;
            rec_freq_index[0] = record++;

            time_var.putVar(index, &verb->travel_time);

            vector<double> power = 10.0 * log10(max(verb->power, 1e-30));
            power_var.putVar(rec_freq_index, rec_freq_count,
                             power.data().begin());

            // clang-format off
            v = verb->length;					length_var.putVar(index, &v);
            v = verb->width;					width_var.putVar(index, &v);
            v = verb->position.latitude();		lat_var.putVar(index, &v);
            v = verb->position.longitude();		lng_var.putVar(index, &v);
            v = verb->position.altitude();		alt_var.putVar(index, &v);
            v = to_degrees(verb->direction);	direction_var.putVar(index, &v);
            v = to_degrees(verb->grazing);		grazing_var.putVar(index, &v);
            v = verb->sound_speed;				sound_speed_var.putVar(index, &v);
            s = verb->de_index;           		de_index_var.putVar(index, &s);
            s = verb->az_index;           		az_index_var.putVar(index, &s);
            v = to_degrees(verb->source_de);	source_de_var.putVar(index, &v);
            v = to_degrees(verb->source_az);    source_az_var.putVar(index, &v);
            i = verb->surface;            		surface_var.putVar(index, &i);
            i = verb->bottom;             		bottom_var.putVar(index, &i);
            i = verb->caustic;            		caustic_var.putVar(index, &i);
            i = verb->upper;              		upper_var.putVar(index, &i);
            i = verb->lower;              		lower_var.putVar(index, &i);
            // clang-format on
        }
    }
}

/**
 * Reads the eigenverbs for a single interface from a netcdf file.
 */
void eigenverb_collection::read_netcdf(const char* filename, size_t interface) {
    netCDF::NcFile nc_file(filename, netCDF::NcFile::read);

    // dimensions

    netCDF::NcDim eigenverb_dim = nc_file.getDim("eigenverbs");
    size_t num_eigenverbs = eigenverb_dim.getSize();
    netCDF::NcDim freq_dim = nc_file.getDim("frequency");
    size_t num_freq = freq_dim.getSize();

    // variables

    netCDF::NcVar time_var = nc_file.getVar("travel_time");
    netCDF::NcVar freq_var = nc_file.getVar("frequency");
    netCDF::NcVar power_var = nc_file.getVar("power");
    netCDF::NcVar length_var = nc_file.getVar("length");
    netCDF::NcVar width_var = nc_file.getVar("width");
    netCDF::NcVar lat_var = nc_file.getVar("latitude");
    netCDF::NcVar lng_var = nc_file.getVar("longitude");
    netCDF::NcVar alt_var = nc_file.getVar("altitude");
    netCDF::NcVar direction_var = nc_file.getVar("direction");
    netCDF::NcVar grazing_var = nc_file.getVar("grazing_angle");
    netCDF::NcVar sound_speed_var = nc_file.getVar("sound_speed");
    netCDF::NcVar de_index_var = nc_file.getVar("de_index");
    netCDF::NcVar az_index_var = nc_file.getVar("az_index");
    netCDF::NcVar source_de_var = nc_file.getVar("source_de");
    netCDF::NcVar source_az_var = nc_file.getVar("source_az");
    netCDF::NcVar surface_var = nc_file.getVar("surface");
    netCDF::NcVar bottom_var = nc_file.getVar("bottom");
    netCDF::NcVar caustic_var = nc_file.getVar("caustic");
    netCDF::NcVar upper_var = nc_file.getVar("upper");
    netCDF::NcVar lower_var = nc_file.getVar("lower");

    // get frequencies just once

    auto* freq_data = new double[num_freq];
    freq_var.getVar(freq_data);
    seq_vector::csptr frequencies(new seq_data(freq_data, num_freq));
    delete[] freq_data;

    // read each record

    std::vector<size_t> index(1);
    std::vector<size_t> rec_freq_index(2);
    rec_freq_index[0] = 0;
    rec_freq_index[1] = 0;
    std::vector<size_t> rec_freq_count(2);
    rec_freq_count[0] = 1;
    rec_freq_count[1] = num_freq;

    auto* power = new double[num_freq];
    for (size_t record = 0; record < num_eigenverbs; ++record) {
        auto* verb = new eigenverb_model();
        verb->power.resize(num_freq, true);
        index[0] = record;
        rec_freq_index[0] = record;

        // get data

        double v;
        long i;
        time_var.getVar(index, &v);
        verb->travel_time = v;
        verb->frequencies = frequencies;

        power_var.getVar(rec_freq_index, rec_freq_count, power);
        for (size_t n = 0; n < num_freq; ++n) {
            verb->power[n] = pow(10.0, power[n] / 10.0);
        }

        // clang-format off
        length_var.getVar(index, &v);    	verb->length = v;
        width_var.getVar(index, &v);		verb->width = v;
        lat_var.getVar(index, &v);          verb->position.latitude(v);
        lng_var.getVar(index, &v);          verb->position.longitude(v);
        alt_var.getVar(index, &v);          verb->position.altitude(v);
        direction_var.getVar(index, &v);    verb->direction = to_radians(v);
        grazing_var.getVar(index, &v);      verb->grazing = to_radians(v);
        sound_speed_var.getVar(index, &v); 	verb->sound_speed = v;
        de_index_var.getVar(index, &i);     verb->de_index = i;
        az_index_var.getVar(index, &i);     verb->az_index = i;
        source_de_var.getVar(index, &v);    verb->source_de = to_radians(v);
        source_az_var.getVar(index, &v);    verb->source_az = to_radians(v);
        surface_var.getVar(index, &i);      verb->surface = int(i);
        bottom_var.getVar(index, &i);       verb->bottom = int(i);
        caustic_var.getVar(index, &i);      verb->caustic = int(i);
        upper_var.getVar(index, &i);        verb->upper = int(i);
        lower_var.getVar(index, &i);        verb->lower = int(i);
        // clang-format on

        add_eigenverb(eigenverb_model::csptr(verb), interface);
    }
    delete[] power;
}
