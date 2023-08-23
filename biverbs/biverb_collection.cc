/*
 * @file biverb_collection.cc
 */

#include <netcdfcpp.h>
#include <usml/biverbs/biverb_collection.h>
#include <usml/biverbs/biverb_model.h>
#include <usml/eigenverbs/eigenverb_model.h>
#include <usml/threads/read_write_lock.h>

#include <cstddef>
#include <list>
#include <sstream>
#include <string>

using namespace usml::biverbs;
using namespace usml::eigenverbs;

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
void biverb_collection::add_biverb(const biverb_model::csptr& verb,
                                         size_t interface) {
    write_lock_guard guard(_mutex);
    _collection[interface].insert({verb->time,verb});
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
//        NcDim* eigenverb_dim = nc_file.add_dim("eigenverbs", (long)list.size());
//        NcDim* freq_dim = nc_file.add_dim(
//            "frequency", (long)list.begin()->get()->frequencies->size());
//
//        // variables
//
//        NcVar* time_var =
//            nc_file.add_var("travel_time", ncDouble, eigenverb_dim);
//        NcVar* freq_var = nc_file.add_var("frequency", ncDouble, freq_dim);
//        NcVar* power_var =
//            nc_file.add_var("power", ncDouble, eigenverb_dim, freq_dim);
//        NcVar* length_var = nc_file.add_var("length", ncDouble, eigenverb_dim);
//        NcVar* width_var = nc_file.add_var("width", ncDouble, eigenverb_dim);
//        NcVar* lat_var = nc_file.add_var("latitude", ncDouble, eigenverb_dim);
//        NcVar* lng_var = nc_file.add_var("longitude", ncDouble, eigenverb_dim);
//        NcVar* alt_var = nc_file.add_var("altitude", ncDouble, eigenverb_dim);
//        NcVar* direction_var =
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
//        NcVar* surface_var = nc_file.add_var("surface", ncShort, eigenverb_dim);
//        NcVar* bottom_var = nc_file.add_var("bottom", ncShort, eigenverb_dim);
//        NcVar* caustic_var = nc_file.add_var("caustic", ncShort, eigenverb_dim);
//        NcVar* upper_var = nc_file.add_var("upper", ncShort, eigenverb_dim);
//        NcVar* lower_var = nc_file.add_var("lower", ncShort, eigenverb_dim);
//        NcVar* north_var =
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
//            v = verb->length;					length_var->put(&v, 1);
//            v = verb->width;					width_var->put(&v, 1);
//            v = verb->position.latitude();		lat_var->put(&v, 1);
//            v = verb->position.longitude();		lng_var->put(&v, 1);
//            v = verb->position.altitude();		alt_var->put(&v, 1);
//            v = to_degrees(verb->direction);	direction_var->put(&v, 1);
//            v = to_degrees(verb->grazing);		grazing_var->put(&v, 1);
//            v = verb->sound_speed;				sound_speed_var->put(&v, 1);
//            i = (long)verb->de_index;           de_index_var->put(&i, 1);
//            i = (long)verb->az_index;           az_index_var->put(&i, 1);
//            v = to_degrees(verb->source_de);    source_de_var->put(&v, 1);
//            v = to_degrees(verb->source_az);    source_az_var->put(&v, 1);
//            i = (long)verb->surface;            surface_var->put(&i, 1);
//            i = (long)verb->bottom;             bottom_var->put(&i, 1);
//            i = (long)verb->caustic;            caustic_var->put(&i, 1);
//            i = (long)verb->upper;              upper_var->put(&i, 1);
//            i = (long)verb->lower;              lower_var->put(&i, 1);
//            v = verb->bounding_box.north;       north_var->put(&v, 1);
//            v = verb->bounding_box.south;       south_var->put(&v, 1);
//            v = verb->bounding_box.east;        east_var->put(&v, 1);
//            v = verb->bounding_box.west;        west_var->put(&v, 1);
//            // clang-format on
//        }
//    }
}
