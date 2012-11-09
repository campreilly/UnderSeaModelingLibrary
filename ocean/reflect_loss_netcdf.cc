/**
 * @file reflection_loss_netcdf.cc
 * Models plane wave reflection from a bottom province profile.
 */
#include <usml/ocean/reflect_loss_netcdf.h>

using namespace usml::ocean ;

reflect_loss_netcdf::reflect_loss_netcdf(const char* filename) {

/**
 * Opens the netcdf file, [filename], and grabs the following data:
 *
 *	@var lat            : latitude in degrees
 *	@var lon            : longitude in degrees
 *	@var type           : predetermined bottom province number
 *	@var speed_ratio    : specific speed ratio for [type]
 *	@var density_ratio  : specific density ratio for [type]
 *	@var atten          : attenuation value for [type]
 *	@var shear_speed    : specific shear speed for [type]
 *	@var shear_atten    : specific speed atten for [type]
 *
 */

	NcFile file( filename );
	NcVar *_lat = file.get_var("lat");
	NcVar *_lon = file.get_var("lon");
	NcVar *bot_num = file.get_var("type");
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
    double* prov_num = new double[latdim*londim];
        bot_num->get(&prov_num[0], latdim*londim);
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

    /** Creates a sequence vector of axises that are passed to the data grid constructor */
    seq_vector* axis[2];
    double latinc = ( latitude[latdim] - latitude[0] ) / latdim ;
    axis[0] = new seq_linear(to_colatitude(latitude[0]), to_radians(latinc), int(latdim));
    double loninc = ( longitude[latdim] - longitude[0] ) / londim ;
    axis[1] = new seq_linear(to_colatitude(longitude[0]), to_radians(loninc), int(londim));

    /** Creates a data grid with the above assigned axises and populates the grid with the data from the netcdf file */
    province = new data_grid<double,2>(axis);
    unsigned index[2];
    for(int i=0; i<latdim; i++) {
        for(int j=0; j<londim; j++) {
            index[0] = i;
            index[1] = j;
            province->data(index,prov_num[i*latdim+j]);
        }
    }
    /** Set the interpolation type to the nearest neighbor and restrict extrapolation */
    for(int i=0; i<2; i++){
        province->interp_type(i, GRID_INTERP_NEAREST);
        province->edge_limit(i, true);
    }

    /** Builds a vector of reflect_loss_rayleigh values for all bottom province numbers */
    for(int i=0; i<int(n_types); i++) {
        rayleigh.push_back( new reflect_loss_rayleigh( density[i], speed[i], atten[i], shearspd[i], shearatten[i] ) );
    }

	delete[] latitude;
	delete[] longitude;
	delete[] speed;
	delete[] density;
	delete[] atten;
	delete[] shearspd;
	delete[] shearatten;
	delete[] prov_num;
	for(int i=0; i<n_types; i++) {
        delete[] rayleigh[i];
	}

}

/** Creates a rayleigh reflection loss value for the bottom province number at a specific location */
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
    void reflect_loss_netcdf::reflect_loss(
        const wposition1& location,
        const seq_vector& frequencies, double angle,
        vector<double>* amplitude, vector<double>* phase) {

        double loc[2];
        loc[0] = location.latitude();
        loc[1] = location.longitude();

        unsigned prov = province->interpolate(loc);
        rayleigh[prov]->reflect_loss(location, frequencies, angle,
			amplitude, phase );
    }
