/*
 * @file eigenverb_collection.cc
 */

#include <usml/types/seq_data.h>
#include <usml/eigenverb/eigenverb_collection.h>
#include <netcdfcpp.h>
#include <boost/foreach.hpp>

using namespace usml::types ;
using namespace usml::eigenverb ;
using namespace boost ;

// meters/degree  60 nmiles/degree * 1852 meters/nmiles
double eigenverb_collection::latitude_scaler = (60.0*1852.0);

/**
 * Builds a box to insert in an rtree and to query the rtree
 */
box eigenverb_collection::build_box(eigenverb verb, int sigma)
{
	double q;
	double latitude;
	double longitude;
	double delta_lat;
	double delta_long;

	q = sqrt(max(verb.length2[0], verb.width2[0]));
	latitude = verb.position.latitude();
	longitude = verb.position.longitude();
	delta_lat = (sigma*q)/latitude_scaler;
	delta_long = (sigma*q)/(latitude_scaler * cos(to_radians(latitude)));

	// create a box, first point bottom left, second point upper right
	box b(point( latitude - delta_lat, longitude - delta_long),
				point(latitude + delta_lat, longitude + delta_long));

	return b;
}

/**
 * Queries the RTree for this collection of eigenverbs at the interface and the
 * spatial box specified the rcv_eigenverb.
 * Results are return via the third parameter.
 */
void eigenverb_collection::query_rtree(size_t interface, eigenverb verb,
											  std::vector<value_pair>& result_s)
{
	int scaling = 1;
	box query_box = build_box(verb, scaling);
	_rtrees[interface].query(bgi::overlaps(query_box),
								std::back_inserter(result_s));
}
/**
 * Generates the rtrees for this collection of eigenverbs.
 */
void eigenverb_collection::generate_rtrees() {

	if (rtrees_ready) return;

	eigenverb verb;
	eigenverb_list::iterator iter;

	for (int n = 0; n < num_interfaces(); ++n){
		// Add to rtree
		for (iter = _collection[n].begin(); iter != _collection[n].end(); ++iter ) {

			verb = *iter;

			box b = build_box(verb);

			// insert value_pair into rtree
			_rtrees[n].insert(std::make_pair(b, iter));
		}
	}
	rtrees_ready = true;
}

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
		NcVar* sound_speed_var = nc_file->add_var("sound_speed", ncDouble, eigenverb_dim);
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
			v = to_degrees(verb.grazing); grazing_var->put(&v, 1);
			v = verb.sound_speed; sound_speed_var->put(&v, 1);
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

/**
 * Reads the eigenverbs for a single interface from a netcdf file.
 */
eigenverb_list eigenverb_collection::read_netcdf(const char* filename, size_t interface)
{
	// return data
	eigenverb_list eigenverbs;
	// Open file
	NcFile* nc_file = new NcFile(filename, NcFile::ReadOnly);

	if (!nc_file->is_valid()){
		cout << "Could not open file!" << endl;
		return eigenverbs;
	}

	NcAtt* long_name_att =  nc_file->get_att("long_name");
	char*  long_name;

	long_name = long_name_att->as_string(0);

	delete long_name_att;

	if (strcmp(long_name, "bottom eigenverbs") == 0) {
		interface = eigenverb::BOTTOM;
	} else if (strcmp(long_name, "surface eigenverbs") == 0) {
		interface = eigenverb::SURFACE;
	} else if (strcmp(long_name, "upper volume eigenverbs") == 0) {
		interface = eigenverb::VOLUME_UPPER;
	} else if (strcmp(long_name, "upper volume eigenverbs") == 0) {
		interface = eigenverb::VOLUME_LOWER;
	} else {
		return eigenverbs;
	}

	// dimensions

	NcDim* eigenverb_dim = nc_file->get_dim("eigenverbs") ;
	long num_eigenverbs = eigenverb_dim->size();
	NcDim* freq_dim = nc_file->get_dim("frequency") ;
	long num_freq = freq_dim->size();

	// variables

	NcVar* time_var = nc_file->get_var("travel_time");
	NcVar* freq_var = nc_file->get_var("frequency" );
	NcVar* energy_var = nc_file->get_var("energy") ;
	NcVar* length_var = nc_file->get_var("length") ;
	NcVar* width_var = nc_file->get_var("width") ;
	NcVar* lat_var = nc_file->get_var("latitude") ;
	NcVar* lng_var = nc_file->get_var("longitude") ;
	NcVar* alt_var = nc_file->get_var("altitude") ;
	NcVar* direction_var = nc_file->get_var("direction") ;
	NcVar* grazing_var = nc_file->get_var("grazing_angle") ;
	NcVar* sound_speed_var = nc_file->get_var("sound_speed");
	NcVar* de_index_var = nc_file->get_var("de_index") ;
	NcVar* az_index_var = nc_file->get_var("az_index") ;
	NcVar* source_de_var = nc_file->get_var("source_de") ;
	NcVar* source_az_var = nc_file->get_var("source_az") ;
	NcVar* surface_var = nc_file->get_var("surface") ;
	NcVar* bottom_var = nc_file->get_var("bottom") ;
	NcVar* caustic_var = nc_file->get_var("caustic") ;
	NcVar* upper_var = nc_file->get_var("upper") ;
	NcVar* lower_var = nc_file->get_var("lower") ;


	double* freq_data = new double[num_freq];
	double* freq_accessor = freq_data;
	seq_vector* frequencies = NULL;

	// Get frequencies just once
	for (int rec = 0; rec < num_freq; ++rec )
	{
		freq_var->set_cur(rec);
		freq_var->get(freq_accessor++, 1, num_freq);
	}
	frequencies = new seq_data(freq_data, num_freq);

	// data storage
	eigenverb verb;
	verb.energy.resize(num_freq);
	verb.length2.resize(num_freq);
	verb.width2.resize(num_freq);

	double* data = new double[num_freq];

	// current record
	for (int rec = 0; rec < num_eigenverbs; ++rec )
	{
		// sets current index
		time_var->set_cur(rec);
		energy_var->set_cur(rec, 0);
		length_var->set_cur(rec, 0);
		width_var->set_cur(rec, 0);
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
		++rec ;

		// translate units

		vector<double> energy ; //  = 10.0*log10(max(verb.energy, 1e-30));
		vector<double> length ;
		vector<double> width ;

		// get data

		double v ;
		long i ;
		time_var->get(&v, 1);
		verb.time = v;
		verb.frequencies = frequencies;

		energy_var->get(data, 1, num_freq);
		double* energy_accessor = data;
		double tmp;
		BOOST_FOREACH (double& d, verb.energy) {
			tmp = *energy_accessor++; // Get dB
			d = pow(10.0, (tmp/10.0)); // convert to linear intensity
		}

		length_var->get(data, 1, num_freq);
		double* length_accessor = data;
		BOOST_FOREACH (double& d, verb.length2) {
			d = *length_accessor++;
			d = d*d;
		}

		width_var->get(data, 1, (long) num_freq);
		double* width_accessor = data;
		BOOST_FOREACH (double& d, verb.width2) {
			d = *width_accessor++;
			d = d*d;
		}

		lat_var->get(&v, 1);
		verb.position.latitude(v) ;
		lng_var->get(&v, 1);
		verb.position.longitude(v) ;
		alt_var->get(&v, 1);
		verb.position.altitude(v) ;
		direction_var->get(&v, 1);
		verb.direction = to_radians(v) ;
		grazing_var->get(&v, 1);
		verb.grazing = to_radians(v) ;
		sound_speed_var->get(&v, 1);
		verb.sound_speed = v ;
		de_index_var->get(&i, 1);
		verb.de_index = i;
		az_index_var->get(&i, 1);
		verb.az_index = i;
		source_de_var->get(&v, 1);
		verb.source_de = to_radians(v) ;
	    source_az_var->get(&v, 1);
	    verb.source_az = to_radians(v) ;
	    surface_var->get(&i, 1);
		verb.surface = i;
		bottom_var->get(&i, 1);
		verb.bottom = i;
		caustic_var->get(&i, 1);
		verb.caustic = i;
		upper_var->get(&i, 1);
		verb.upper = i;
		lower_var->get(&i, 1);
		verb.lower = i;

		eigenverbs.push_back(verb);
	}

	delete nc_file;

	delete [] data;
	delete [] freq_data;
	delete frequencies;

    return eigenverbs;
}

