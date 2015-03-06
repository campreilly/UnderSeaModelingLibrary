/**
 * @file envelope_collection.cc
 */
#include <usml/eigenverb/envelope_collection.h>

using namespace usml::eigenverb ;
using namespace boost ;

/**
 * Constructor
 */
envelope_collection::envelope_collection(
    double resolution,
    size_t num_bins,
    size_t num_az )
    : _envelopes( num_az )
{
    initialize( num_bins, resolution ) ;
}

envelope_collection::~envelope_collection()
{
    BOOST_FOREACH( vector<double>* i, _envelopes )
            if( i ) delete i ;
}

void envelope_collection::initialize(
    size_t size, double resolution )
{
    // Initialize the envelope vectors to 1e-20
	BOOST_FOREACH(vector<double>*& i, _envelopes)
		i = new vector<double>( size, 1e-20 ) ;

    // Initialize the two way travel time vector
    // based on the resolution or number of bins
    // of the envelopes.
    _two_way_time.resize( size ) ;
    _two_way_time.clear() ;
    size_t count = 0 ;
    BOOST_FOREACH( double& i, _two_way_time )
        i = resolution * count++;
}

/**
 * Adds a small time spread gaussian contribution
 * to the envelope in the azimuthal direction.
 */
void envelope_collection::add_gaussian(
    double peak, double travel_time,
    double duration_time, size_t az )
{
    vector<double> time_exp =
            ( _two_way_time - travel_time ) * ( 1.0 / duration_time ) ;
    time_exp = element_prod( time_exp, time_exp ) ;
    vector<double> _time_spread =
            ( peak / ( duration_time * sqrt(TWO_PI) ) ) * exp( -0.5 * time_exp ) ;
    (*_envelopes(az)) += _time_spread ;
}

/**
 * Returns the envelope at the azimuthal index of az
 */
vector<double> envelope_collection::envelopes( size_t az ) const
{
   return (*_envelopes(az)) ;
}

/**
 * Returns the entire reveberation envelope collection
 */
vector<vector<double>*> envelope_collection::envelopes() const
{
    return _envelopes ;
}

/**
 * Saves envelope data to disk
 */
void envelope_collection::write_netcdf( const char* filename )
{
    NcFile* nc_file = new NcFile(filename, NcFile::Replace);
    NcDim* az_dim = nc_file->add_dim("number_envelopes", (long) _envelopes.size() ) ;
    NcDim* time_dim = nc_file->add_dim("time_resolution", (long) _two_way_time.size() ) ;

    // variables
    NcVar* time_var = nc_file->add_var("two_way_time", ncDouble, time_dim);
    NcVar* az_var = nc_file->add_var("azimuth", ncShort, az_dim);
    NcVar* intensity_var = nc_file->add_var("intensity", ncDouble, az_dim, time_dim);
    az_var->add_att("units", "count");
    time_var->add_att("units", "seconds");
    intensity_var->add_att("units", "linear");

    time_var->put(_two_way_time.data().begin(), (long) _two_way_time.size());
    long record = 0 ;
    int count = 0 ;
    BOOST_FOREACH( vector<double>* i, _envelopes )
    {
        az_var->set_cur(record);
        intensity_var->set_cur(record);
        ++record ;
        az_var->put(&count, 1);
        ++count ;
        intensity_var->put(i->data().begin(), 1, (long) _two_way_time.size()) ;
    }
    // close file

    delete nc_file; // destructor frees all netCDF temp variables
}
