/**
 * @file reflection_loss_netcdf.cc
 * Models plane wave reflection from a bottom province profile.
 */
#include <usml/ocean/reflection_loss_netcdf.h>
#include <usml/types/data_grid.h>

using namespace usml::ocean ;

reflection_loss_netcdf::reflection_loss_netcdf(const char* filename) {

/**
 * Opens the netcdf file, [filename], and grabs the following data:
 *
 *	@var lat	   : latitude in degrees
 *	@var lon	   : longitude in degrees
 *	@var type	   : predetermined bottom province number
 *	@var speed_ratio   : specific speed ratio for [type]
 *	@var density_ratio : specific density ratio for [type]
 *	@var atten	   : attenuation value for [type]
 *	@var shear_speed   : specific shear speed for [type]
 *	@var shear_atten   : specific speed atten for [type]
 *
 */

	NcFile file( filename );
	NcVar *_lat = file.get_var("lat");
	NcVar *_lon = file.get_var("lon");
//	NcVar *bot_num = file.get_var("type");
	NcVar *bot_speed = file.get_var("speed_ratio");
	NcVar *bot_density = file.get_var("density_ratio");
	NcVar *bot_atten = file.get_var("atten");
	NcVar *bot_shear_speed = file.get_var("shear_speed");
	NcVar *bot_shear_atten = file.get_var("shear_atten");

    /** Gets the size of the dimensions to be used to create the data grid */
    int ncid, latid, lonid, provid;
    long latdim, londim, n_types;
    ncid = ncopen(filename, NC_NOWRITE);
	latid = ncdimid(ncid, "lat");
    ncdiminq(ncid, latid, 0, &latdim);
    lonid = ncdimid(ncid, "lon");
    ncdiminq(ncid, lonid, 0, &londim);
    provid = ncdimid(ncid, "num_types");
    ncdiminq(ncid, provid, 0, &n_types);

    /** Extracts the data for all of the variables from the netcdf file and stores them */
    double* latitude = new double[latdim];
        _lat->get(&latitude[0], latdim);
    double* longitude = new double[londim];
        _lon->get(&longitude[0], londim);
    double* speed = new double[n_types];
        bot_speed->get(&speed[0], n_types);
    double* density = new double[n_types];
        bot_density->get(&density[0], n_types);
    double* atten = new double[n_types];
        bot_atten->get(&atten[0], n_types);
    double* shearspd = new double[n_types];
        bot_shear_speed->get(&shearspd[0], n_types);
    double* shearatten = new double[n_types];
        bot_shear_atten->get(&shearatten[0], n_types);

//    data_grid<double, 2>* bottom_prov_grid;
//    double inc = ( latitude[latdim] - latitude[0] ) / latdim ;
//    _axis[0] = new seq_linear(to_colatitude(int(latitude[0])), to_radians(int(inc)), latdim);

/***************************************************************
	double bot_prov_dataIn[sizeof(_lat)][sizeof(_lon)];

	data_grid<double,2> bottom_prov_grid;
	bottom_prov_grid->get(bot_prov_dataIn[0][0],)
***************************************************************/

//private:

//	reflection_loss_rayleigh rayleigh[_n];

//for(int i=0; i<_n; i++) {
//	rayleigh[i] =  reflect_loss_rayleigh(
//	        double bot_density[i], double bot_speed[i], double bot_atten[i],
//	        double bot_shear_speed[i], double bot_shear_atten[i] ) ;
//}

	// build the data grid with the axis of lat and lon and the innerds as the prov #s

	// reflection_loss( const wposition1& location, const seq_vector& frequencies, double angle, blah......)
	// find the prov # for above location from data grid
	// rayleigh[above prov #].reflection_loss(all that jazz);

	delete[] latitude;
	delete[] longitude;
	delete[] speed;
	delete[] density;
	delete[] atten;
	delete[] shearspd;
	delete[] shearatten;

}

/**
 * Computes the broadband reflection loss and phase change.
 *
 * @param location      Location at which to compute attenuation.
 * @param frequencies   Frequencies over which to compute loss. (Hz)
 * @param angle         Reflection angle relative to the normal (radians).
 * @param amplitude     Change in ray strength in dB (output).
 * @param phase         Change in ray phase in radians (output).
 *                      Phase change not computed if this is NULL.
 */
    void reflection_loss_netcdf::reflect_loss(
        const wposition1& location,
        const seq_vector& frequencies, double angle,
        vector<double>* amplitude, vector<double>* phase) {}
