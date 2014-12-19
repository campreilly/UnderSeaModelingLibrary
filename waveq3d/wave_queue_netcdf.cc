/**
 * @file wave_queue_netcdf.cc
 * Recording to netCDF wavefront log
 */
#include <usml/waveq3d/wave_queue.h>

using namespace usml::waveq3d ;

/**
 * Initialize recording to netCDF wavefront log.
 */
void wave_queue::init_netcdf( const char* filename, const char* long_name ) {

    _nc_file = new NcFile( filename, NcFile::Replace );
    _nc_rec = 0 ;
    if ( long_name ) {
        _nc_file->add_att("long_name", long_name ) ;
    }
    _nc_file->add_att("Conventions", "COARDS" ) ;

    // dimensions

    NcDim *freq_dim = _nc_file->add_dim( "frequency", (long) _frequencies->size() ) ;
    NcDim *de_dim   = _nc_file->add_dim( "source_de", (long) _source_de->size() ) ;
    NcDim *az_dim   = _nc_file->add_dim( "source_az", (long) _source_az->size() ) ;
    NcDim *time_dim = _nc_file->add_dim( "travel_time" ) ; // unlimited

    // coordinates

    NcVar *freq_var = _nc_file->add_var( "frequency", ncDouble, freq_dim ) ;
    NcVar *de_var   = _nc_file->add_var( "source_de", ncDouble, de_dim ) ;
    NcVar *az_var   = _nc_file->add_var( "source_az", ncDouble, az_dim ) ;
    _nc_time        = _nc_file->add_var( "travel_time", ncDouble, time_dim ) ;
    _nc_latitude    = _nc_file->add_var( "latitude", ncDouble,
                      time_dim, de_dim, az_dim ) ;
    _nc_longitude   = _nc_file->add_var( "longitude", ncDouble,
                      time_dim, de_dim, az_dim ) ;
    _nc_altitude    = _nc_file->add_var( "altitude", ncDouble,
                      time_dim, de_dim, az_dim ) ;
    _nc_surface     = _nc_file->add_var( "surface", ncShort,
                      time_dim, de_dim, az_dim ) ;
    _nc_bottom      = _nc_file->add_var( "bottom", ncShort,
                      time_dim, de_dim, az_dim ) ;
    _nc_caustic     = _nc_file->add_var( "caustic", ncShort,
                      time_dim, de_dim, az_dim ) ;
    _nc_upper       = _nc_file->add_var( "upper_vertex", ncShort,
                      time_dim, de_dim, az_dim ) ;
    _nc_lower       = _nc_file->add_var( "lower_vertex", ncShort,
                      time_dim, de_dim, az_dim ) ;
    _nc_on_edge     = _nc_file->add_var( "on_edge", ncByte,
                      time_dim, de_dim, az_dim ) ;

    // units

    freq_var->add_att("units", "hertz") ;
    de_var->add_att("units", "degrees") ;
    de_var->add_att("positive", "up") ;
    az_var->add_att("units", "degrees_true") ;
    az_var->add_att("positive", "clockwise") ;
    _nc_time->add_att("units", "seconds") ;
    _nc_latitude->add_att("units", "degrees_north") ;
    _nc_longitude->add_att("units", "degrees_east") ;
    _nc_altitude->add_att("units", "meters") ;
    _nc_altitude->add_att("positive", "up") ;
    _nc_surface->add_att("units", "count") ;
    _nc_bottom->add_att("units", "count") ;
    _nc_caustic->add_att("units", "count") ;
    _nc_upper->add_att("units", "count") ;
    _nc_lower->add_att("units", "count") ;
    _nc_on_edge->add_att("units", "bool") ;

    // coordinate data

    freq_var->put( vector<double>(*_frequencies).data().begin(),
                   (long) _frequencies->size() ) ;
    de_var->put( vector<double>(*_source_de).data().begin(),
		(long) _source_de->size());
    az_var->put( vector<double>(*_source_az).data().begin(),
		(long) _source_az->size());
}

/**
 * Write current record to netCDF wavefront log.
 */
void wave_queue::save_netcdf() {
    NcError( verbose_nonfatal ) ;
    _nc_time->put_rec( &_time, _nc_rec ) ;
    _nc_latitude->put_rec(_curr->position.latitude().data().begin(),_nc_rec);
    _nc_longitude->put_rec(_curr->position.longitude().data().begin(),_nc_rec);
    _nc_altitude->put_rec(_curr->position.altitude().data().begin(),_nc_rec);
    _nc_surface->put_rec(_curr->surface.data().begin(),_nc_rec);
    _nc_bottom->put_rec(_curr->bottom.data().begin(),_nc_rec);
    _nc_caustic->put_rec(_curr->caustic.data().begin(),_nc_rec);
    _nc_upper->put_rec(_curr->upper.data().begin(),_nc_rec);
    _nc_lower->put_rec(_curr->lower.data().begin(),_nc_rec);
    _nc_on_edge->put_rec((const ncbyte*)_curr->on_edge.data().begin(),_nc_rec);
    ++_nc_rec ;
}

/**
 * Close netCDF wavefront log.
 */
void wave_queue::close_netcdf() {
    delete _nc_file ; // destructor frees all netCDF temp variables
    _nc_file = NULL ;
}
