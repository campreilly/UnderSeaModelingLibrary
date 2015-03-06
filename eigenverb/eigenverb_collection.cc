/*
 * @file eigenverb_collection.cc
 */

#include <usml/eigenverb/eigenverb_collection.h>
#include <netcdfcpp.h>
#include <boost/foreach.hpp>

using namespace usml::eigenverb ;
using namespace boost ;

/**
 * Cosntructor
 */
eigenverb_collection::eigenverb_collection( size_t layers )
    : _upper( layers ), _lower( layers )
{

}
//eigenverb_collection::eigenverb_collection(
//    double lon, double lat,
//    double lon_range, double lat_range,
//    size_t layers )
//{
//    _bottom = new eigenverb_tree( lon, lat, lon_range, lat_range ) ;
//    _surface = new eigenverb_tree( lon, lat, lon_range, lat_range ) ;
//    if( layers > 0 ) {
//        _upper.resize( layers ) ;
//        _lower.resize( layers ) ;
//        for(size_t i=0; i<layers; ++i) {
//            _upper(i) = new eigenverb_tree( lon, lat, lon_range, lat_range ) ;
//            _lower(i) = new eigenverb_tree( lon, lat, lon_range, lat_range ) ;
//        }
//    }
//}

/**
 * Destructor
 */
eigenverb_collection::~eigenverb_collection()
{
//    if( _bottom )
//        delete _bottom ;
//    if( _surface )
//        delete _surface ;
//    for(size_t i=0; i<_upper.size(); ++i) {
//        if( _upper(i) )
//            delete _upper(i) ;
//        if( _lower(i) )
//            delete _lower(i) ;
//    }
}

/**
 * Adds an eigenverb to the collection
 */
void eigenverb_collection::add_eigenverb(
    eigenverb e, interface_type i )
{
    switch(i) {
        case BOTTOM:
            _bottom.push_back( e ) ;
            break;
        case SURFACE:
            _surface.push_back( e ) ;
            break;
        case VOLUME_UPPER:
        {
            size_t something = 0 ;
            _upper(something).push_back( e ) ;
        }
            break;
        case VOLUME_LOWER:
        {
            size_t something = 0 ;
            _lower(something).push_back( e ) ;
        }
            break;
        default:
            throw std::invalid_argument(
                    "Invalid interface type. Must be one defined in eigenverb.h") ;
            break;
    }
}

/**
 * Returns the list of eigenverbs for the bottom
 * interface
 */
eigenverb_list eigenverb_collection::bottom() const
{
    return _bottom ;
}

/**
 * Returns the list of eigenverbs for the surface
 * interface
 */
eigenverb_list eigenverb_collection::surface() const
{
    return _surface ;
}

/**
 * Returns the list of eigenverbs for the volume
 * upper interface
 */
vector<eigenverb_list> eigenverb_collection::upper() const
{
    return _upper ;
}

/**
 * Returns the list of eigenverbs for the l'th volume
 * upper interface
 */
eigenverb_list eigenverb_collection::upper( size_t l ) const
{
    return _upper(l) ;
}


/**
 * Returns the list of eigenverbs for the volume
 * lower interface
 */
vector<eigenverb_list> eigenverb_collection::lower() const
{
    return _lower ;
}

/**
 * Returns the list of eigenverbs for the l'th volume
 * lower interface
 */
eigenverb_list eigenverb_collection::lower( size_t l ) const
{
    return _lower(l) ;
}

/**
 * Determines if there are volumes layers
 */
bool eigenverb_collection::volume() const {
    if( _upper.size() != 0 )
        return true ;
    return false ;
}

/**
 * Writes eigenverbs to disk
 */
void eigenverb_collection::write_netcdf(
    const char* filename, interface_type i )
{
    NcFile* nc_file = new NcFile(filename, NcFile::Replace);
    eigenverb_list curr ;
    switch(i) {
        case BOTTOM:
        {
            curr = _bottom ;
            nc_file->add_att("long_name", "Bottom eigenverbs") ;
            break ;
        }
        case SURFACE:
        {
            curr = _surface ;
            nc_file->add_att("long_name", "Surface eigenverbs") ;
            break ;
        }
//        case UPPER:
//        {
//            nc_file->add_att("long_name", "Upper Volume eigenverbs") ;
//            break ;
//        }
//        case LOWER:
//        {
//            nc_file->add_att("long_name", "Lower Volume eigenverbs") ;
//            break ;
//        }
        default:
            throw std::invalid_argument("Invalid interface type.") ;
            break ;
    }

    // dimensions
    NcDim* freq_dim = nc_file->add_dim("frequency", (long) curr.begin()->frequencies->size()) ;
    NcDim* eigenverb_dim = nc_file->add_dim("eigenverbs", (long) curr.size()) ;

    // variables
    NcVar* freq_var = nc_file->add_var("frequency", ncDouble, freq_dim);
    NcVar* time_var = nc_file->add_var("travel_time", ncDouble, eigenverb_dim);
    NcVar* intensity_var = nc_file->add_var("intensity", ncDouble, eigenverb_dim, freq_dim);
    NcVar* grazing_var = nc_file->add_var("grazing_angle", ncDouble, eigenverb_dim);
    NcVar* distance_var = nc_file->add_var("distance", ncDouble, eigenverb_dim);
    NcVar* lat_var = nc_file->add_var("latitude", ncDouble, eigenverb_dim);
    NcVar* lng_var = nc_file->add_var("longitude", ncDouble, eigenverb_dim);
    NcVar* alt_var = nc_file->add_var("altitude", ncDouble, eigenverb_dim) ;
    NcVar* nrho_var = nc_file->add_var("ndir_rho", ncDouble, eigenverb_dim);
    NcVar* ntheta_var = nc_file->add_var("ndir_theta", ncDouble, eigenverb_dim);
    NcVar* nphi_var = nc_file->add_var("ndir_phi", ncDouble, eigenverb_dim);
    NcVar* de_index_var = nc_file->add_var("de_index", ncShort, eigenverb_dim);
    NcVar* az_index_var = nc_file->add_var("az_index", ncShort, eigenverb_dim);
    NcVar* launch_de_var = nc_file->add_var("launch_de", ncDouble, eigenverb_dim);
    NcVar* launch_az_var = nc_file->add_var("launch_az", ncDouble, eigenverb_dim);
    NcVar* sigma_de_var = nc_file->add_var("sigma_de", ncDouble, eigenverb_dim);
    NcVar* sigma_az_var = nc_file->add_var("sigma_az", ncDouble, eigenverb_dim);
//    NcVar* sigma_de_var = nc_file->add_var("sigma_de", ncDouble, eigenverb_dim, freq_dim);
//    NcVar* sigma_az_var = nc_file->add_var("sigma_az", ncDouble, eigenverb_dim, freq_dim);
    NcVar* speed_var = nc_file->add_var("sound_speed", ncDouble, eigenverb_dim);
    NcVar* surface_var = nc_file->add_var("surface", ncShort, eigenverb_dim);
    NcVar* bottom_var = nc_file->add_var("bottom", ncShort, eigenverb_dim);

    // units
    time_var->add_att("units", "seconds");
    intensity_var->add_att("units", "linear");
    grazing_var->add_att("units", "radians");
    distance_var->add_att("units", "meters");
    lat_var->add_att("units", "degrees_north");
    lng_var->add_att("units", "degrees_east");
    alt_var->add_att("units", "meters");
    alt_var->add_att("positive", "up");
    freq_var->add_att("units", "hertz");
    nrho_var->add_att("units", "m/s");
    ntheta_var->add_att("units", "rad/s");
    nphi_var->add_att("units", "rad/s");
    de_index_var->add_att("units", "index");
    az_index_var->add_att("units", "index");
    launch_de_var->add_att("units", "degrees");
    launch_de_var->add_att("positive", "up");
    launch_az_var->add_att("units", "degrees_true");
    launch_az_var->add_att("positive", "clockwise");
    sigma_de_var->add_att("units", "meters");
    sigma_az_var->add_att("units", "meters");
    speed_var->add_att("units", "m/s");
    surface_var->add_att("units", "count");
    bottom_var->add_att("units", "count");

    freq_var->put(curr.begin()->frequencies->data().begin(),
            (long) curr.begin()->frequencies->size());
    int record = 0 ;    // current record
    BOOST_FOREACH( eigenverb e, curr )
    {
        // sets current index
        time_var->set_cur(record);
        intensity_var->set_cur(record);
        grazing_var->set_cur(record);
        distance_var->set_cur(record);
        lat_var->set_cur(record);
        lng_var->set_cur(record);
        alt_var->set_cur(record);
        nrho_var->set_cur(record);
        ntheta_var->set_cur(record);
        nphi_var->set_cur(record);
        de_index_var->set_cur(record);
        az_index_var->set_cur(record);
        launch_de_var->set_cur(record);
        launch_az_var->set_cur(record);
        sigma_de_var->set_cur(record);
        sigma_az_var->set_cur(record);
        speed_var->set_cur(record);
        surface_var->set_cur(record);
        bottom_var->set_cur(record);
        ++record ;

        // inserts data
        time_var->put(&e.travel_time, 1);
        intensity_var->put(e.intensity.data().begin(),
                1, (long) e.frequencies->size());
        grazing_var->put(&e.grazing, 1);
        distance_var->put(&e.distance, 1);
        double v = e.position.latitude(); lat_var->put(&v, 1);
        v = e.position.longitude(); lng_var->put(&v, 1);
        v = e.position.altitude(); alt_var->put(&v, 1);
        v = e.direction.rho(); nrho_var->put(&v, 1);
        v = e.direction.theta(); ntheta_var->put(&v, 1);
        v = e.direction.phi(); nphi_var->put(&v, 1);
        long i = e.de_index; de_index_var->put(&i, 1);
        i = e.az_index; az_index_var->put(&i, 1);
        launch_de_var->put(&e.launch_de, 1);
        launch_az_var->put(&e.launch_az, 1);
        sigma_de_var->put(&e.sigma_de, 1);
        sigma_az_var->put(&e.sigma_az, 1);
//        sigma_de_var->put(e.sigma_de.data().begin(), 1, freq_dim);
//        sigma_az_var->put(e.sigma_az.data().begin(), 1, freq_dim);
        speed_var->put(&e.sound_speed, 1);
        i = e.surface; surface_var->put(&i, 1);
        i = e.bottom; bottom_var->put(&i, 1);
    }

    // close file

    delete nc_file; // destructor frees all netCDF temp variables
}
