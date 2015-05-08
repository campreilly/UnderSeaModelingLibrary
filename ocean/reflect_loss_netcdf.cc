/**
 * @file reflect_loss_netcdf.cc
 * Models plane wave reflection from a bottom province profile.
 */
#include <usml/ocean/reflect_loss_netcdf.h>
#include <exception>

using namespace usml::ocean ;

reflect_loss_netcdf::reflect_loss_netcdf(const char* filename) {

	NcFile file( filename ) ;
    if (file.is_valid() == 0) {
    	throw std::invalid_argument("file not found") ;
    }
	NcVar *bot_speed = file.get_var("speed_ratio") ;         // bot_speed  : speed ratio of the province
	NcVar *bot_density = file.get_var("density_ratio") ;     // bot_density  : density ratio of the province
	NcVar *bot_atten = file.get_var("atten") ;               // bot_atten  : attenuation value for the province
	NcVar *bot_shear_speed = file.get_var("shear_speed") ;   // bot_shear_speed  : shear speed of the province
	NcVar *bot_shear_atten = file.get_var("shear_atten") ;   // bot_shear_atten  : shear attenuation of the province
	NcVar *lon = file.get_var("longitude") ;                 // lat : latitude in degrees
	NcVar *lat = file.get_var("latitude") ;                  // lon : longitude in degrees
	NcVar *bot_num = file.get_var("type") ;                  // bot_num  : bottom province map

    /** Gets the size of the dimensions to be used to create the data grid */
    int ncid, latid, lonid, types ;
    long latdim, londim, n_types ;
    ncid = ncopen(filename, NC_NOWRITE) ;
	latid = ncdimid(ncid, "latitude") ;
    ncdiminq(ncid, latid, 0, &latdim) ;
    lonid = ncdimid(ncid, "longitude") ;
    ncdiminq(ncid, lonid, 0, &londim) ;
    types = ncdimid(ncid, "speed_ratio") ;
    ncdiminq(ncid, types, 0, &n_types) ;

    /** Extracts the data for all of the variables from the netcdf file and stores them */
    double* latitude = new double[latdim] ;
        lat->get(&latitude[0], latdim) ;
    double* longitude = new double[londim] ;
        lon->get(&longitude[0], londim) ;
    double* type_num = new double[latdim*londim] ;
        bot_num->get(&type_num[0], latdim, londim) ;
    double* speed = new double[n_types] ;
        bot_speed->get(&speed[0], n_types) ;
    double* density = new double[n_types] ;
        bot_density->get(&density[0], n_types) ;
    double* atten = new double[n_types] ;
        bot_atten->get(&atten[0], n_types) ;
    double* shearspd = new double[n_types] ;
        bot_shear_speed->get(&shearspd[0], n_types) ;
    double* shearatten = new double[n_types] ;
        bot_shear_atten->get(&shearatten[0], n_types) ;

    /** Creates a sequence vector of axes that are passed to the data grid constructor */
    const seq_vector* axis[2];
    double latinc = ( latitude[latdim-1] - latitude[0] ) / latdim ;
    axis[0] = new seq_linear(latitude[0], latinc, int(latdim));
    double loninc = ( longitude[londim-1] - longitude[0] ) / londim ;
    axis[1] = new seq_linear(longitude[0], loninc, int(londim));

    /** Creates a data grid with the above assigned axises and populates the grid with the data from the netcdf file */
    _bottom_grid = new data_grid<double,2>(axis) ;
    size_t index[2] ;
    for(int i=0; i<latdim; i++) {
        for(int j=0; j<londim; j++) {
            index[0] = i ;
            index[1] = j ;
            _bottom_grid->data(index, type_num[i*londim+j]) ;
        }
    }

    /** Set the interpolation type to the nearest neighbor and restrict extrapolation */
    for(int i=0; i<2; i++){
        _bottom_grid->interp_type(i, GRID_INTERP_NEAREST) ;
        _bottom_grid->edge_limit(i, true) ;
    }

    /** Builds a vector of reflect_loss_rayleigh values for all bottom province numbers */
    for(int i=0; i<int(n_types); i++) {
        _rayleigh.push_back( new reflect_loss_rayleigh(
                density[i], speed[i], atten[i], shearspd[i], shearatten[i] ) ) ;
    }

    ncclose( ncid ) ;
    delete axis[0] ;
    delete axis[1] ;
	delete[] latitude ;
	delete[] longitude ;
	delete[] speed ;
	delete[] density ;
	delete[] atten ;
	delete[] shearspd ;
	delete[] shearatten ;
	delete[] type_num ;

}

/** Gets a rayleigh reflection loss value for the bottom province number
 * at a specific location then computes the broadband reflection loss and phase change.
 *
 * @param location      Location at which to compute attenuation.
 * @param frequencies   Frequencies over which to compute loss. (Hz)
 * @param angle         Reflection angle relative to the normal (radians).
 * @param amplitude     Change in ray strength in dB (output).
 * @param phase         Change in ray phase in radians (output).
 *                      Phase change not computed if this is NULL.
 */
void reflect_loss_netcdf::reflect_loss(
    const wposition1& location,
    const seq_vector& frequencies, double angle,
    vector<double>* amplitude, vector<double>* phase) {

    double loc[2];
    loc[0] = location.latitude() ;
    loc[1] = location.longitude() ;

    size_t type = _bottom_grid->interpolate(loc) ;
    _rayleigh[type]->reflect_loss(location, frequencies, angle, amplitude, phase ) ;
}

/**
 * Iterates over the rayleigh reflection loss values
 * and deletes them.
 */
reflect_loss_netcdf::~reflect_loss_netcdf() {
	for(std::vector<reflect_loss_rayleigh*>::iterator iter =_rayleigh.begin(); iter != _rayleigh.end(); iter++) {
        delete *iter;
	}
	delete _bottom_grid ;
}
