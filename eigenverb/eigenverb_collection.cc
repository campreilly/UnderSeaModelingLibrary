/*
 * @file eigenverb_collection.cc
 */

#include <usml/eigenverb/eigenverb_collection.h>
#include <netcdfcpp.h>
#include <boost/foreach.hpp>

using namespace usml::eigenverb ;
using namespace boost ;

/**
 * Writes the eigenverbs for an individual interface to a netcdf file.
 */
void eigenverb_collection::write_netcdf(
	const char* filename, size_t interface) const
{
	NcFile* nc_file = new NcFile(filename, NcFile::Replace);
	const eigenverb_list& curr = _collection[interface];

	switch ( interface ) {
	case eigenverb::BOTTOM:
		nc_file->add_att("long_name", "bottom eigenverbs");
		break;
	case eigenverb::SURFACE:
		nc_file->add_att("long_name", "surface eigenverbs");
		break;
	case eigenverb::VOLUME_UPPER:
		nc_file->add_att("long_name", "upper volume eigenverbs");
		nc_file->add_att("layer", 1);
		break;
	case eigenverb::VOLUME_LOWER:
		nc_file->add_att("long_name", "lower volume eigenverbs");
		nc_file->add_att("layer", 1);
		break;
	default:
		{
			size_t layer = interface - eigenverb::VOLUME_UPPER ;
			size_t side = layer % 2 ;
			layer = ( layer / 2 ) + 1 ;
			std::ostringstream oss;
			oss << ((side)?"lower":"upper") << " volume "  << layer << " eigenverbs" ;
			nc_file->add_att("long_name", oss.str().c_str());
			nc_file->add_att("layer", (long)layer);
		}
		break;
	}

	if ( curr.size() > 0 ) {

		// dimensions

		NcDim* eigenverb_dim = nc_file->add_dim("eigenverbs", (long) curr.size()) ;
		NcDim* freq_dim = nc_file->add_dim("frequency", (long) curr.begin()->frequencies->size()) ;

		// variables

		NcVar* time_var = nc_file->add_var("travel_time", ncDouble, eigenverb_dim);
		NcVar* freq_var = nc_file->add_var("frequency", ncDouble, freq_dim);
		NcVar* energy_var = nc_file->add_var("energy", ncDouble, eigenverb_dim, freq_dim);
		NcVar* length_var = nc_file->add_var("length", ncDouble, eigenverb_dim, freq_dim);
		NcVar* width_var = nc_file->add_var("width", ncDouble, eigenverb_dim, freq_dim);
		NcVar* lat_var = nc_file->add_var("latitude", ncDouble, eigenverb_dim);
		NcVar* lng_var = nc_file->add_var("longitude", ncDouble, eigenverb_dim);
		NcVar* alt_var = nc_file->add_var("altitude", ncDouble, eigenverb_dim) ;
		NcVar* direction_var = nc_file->add_var("direction", ncDouble, eigenverb_dim);
		NcVar* grazing_var = nc_file->add_var("grazing_angle", ncDouble, eigenverb_dim);
		NcVar* de_index_var = nc_file->add_var("de_index", ncShort, eigenverb_dim);
		NcVar* az_index_var = nc_file->add_var("az_index", ncShort, eigenverb_dim);
		NcVar* source_de_var = nc_file->add_var("source_de", ncDouble, eigenverb_dim);
		NcVar* source_az_var = nc_file->add_var("source_az", ncDouble, eigenverb_dim);
		NcVar* surface_var = nc_file->add_var("surface", ncShort, eigenverb_dim);
		NcVar* bottom_var = nc_file->add_var("bottom", ncShort, eigenverb_dim);
		NcVar* caustic_var = nc_file->add_var("caustic", ncShort, eigenverb_dim);
		NcVar* upper_var = nc_file->add_var("upper", ncShort, eigenverb_dim);
		NcVar* lower_var = nc_file->add_var("lower", ncShort, eigenverb_dim);

		// units

		time_var->add_att("units", "seconds");
		freq_var->add_att("units", "hertz");
		energy_var->add_att("units", "dB");
		length_var->add_att("units", "meters");
		width_var->add_att("units", "meters");
		lat_var->add_att("units", "degrees_north");
		lng_var->add_att("units", "degrees_east");
		alt_var->add_att("units", "meters");
		direction_var->add_att("units", "degrees_true");
		direction_var->add_att("positive", "clockwise");
		grazing_var->add_att("units", "degrees");
		grazing_var->add_att("positive", "up");
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

		freq_var->put( curr.begin()->frequencies->data().begin(),
				(long) curr.begin()->frequencies->size());
		int record = 0 ;    // current record
		BOOST_FOREACH( eigenverb verb, curr )
		{
			// sets current index

			time_var->set_cur(record);
			energy_var->set_cur(record);
			length_var->set_cur(record);
			width_var->set_cur(record);
			lat_var->set_cur(record);
			lng_var->set_cur(record);
			alt_var->set_cur(record);
			direction_var->set_cur(record);
			grazing_var->set_cur(record);
			de_index_var->set_cur(record);
			az_index_var->set_cur(record);
			source_de_var->set_cur(record);
			source_az_var->set_cur(record);
			surface_var->set_cur(record);
			bottom_var->set_cur(record);
			caustic_var->set_cur(record);
			upper_var->set_cur(record);
			lower_var->set_cur(record);
			++record ;

			// translate units

			vector<double> energy = 10.0*log10(max(verb.energy, 1e-30));
			vector<double> length = sqrt(verb.length2);
			vector<double> width = sqrt(verb.width2);

			// inserts data

			double v ;
			long i ;
			time_var->put(&verb.time, 1);

			energy_var->put(energy.data().begin(), 1, (long) verb.frequencies->size());
			length_var->put(length.data().begin(), 1, (long) verb.frequencies->size());
			width_var->put(width.data().begin(), 1, (long) verb.frequencies->size());
			v = verb.position.latitude(); lat_var->put(&v, 1);
			v = verb.position.longitude(); lng_var->put(&v, 1);
			v = verb.position.altitude(); alt_var->put(&v, 1);
			v = to_degrees(verb.direction) ; direction_var->put(&v, 1);
			v = to_degrees(verb.grazing) ; grazing_var->put(&v, 1);
			i = (long) verb.de_index; de_index_var->put(&i, 1);
			i = (long) verb.az_index; az_index_var->put(&i, 1);
			v = to_degrees(verb.source_de) ; source_de_var->put(&v, 1);
			v = to_degrees(verb.source_az) ; source_az_var->put(&v, 1);
			i = (long) verb.surface; surface_var->put(&i, 1);
			i = (long) verb.bottom; bottom_var->put(&i, 1);
			i = (long) verb.caustic; caustic_var->put(&i, 1);
			i = (long) verb.upper; upper_var->put(&i, 1);
			i = (long) verb.lower; lower_var->put(&i, 1);
		}
	}

    // close file

    delete nc_file; // destructor frees all netCDF temp variables
}
