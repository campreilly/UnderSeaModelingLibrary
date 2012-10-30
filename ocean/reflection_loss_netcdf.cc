/**
 * @file reflection_loss_netcdf.cc
 * Models plane wave reflection from a bottom province profile.
 */
#include <usml/ocean/reflection_loss_netcdf.h>

using namespace usml::ocean ;

reflection_loss_netcdf::reflection_loss_netcdf(const char* filename) {

/**
 * Opens the netcdf file, [filename], and grabs the following data:
 *
 *	@dim num_types	   : the number of bottom provinces
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
//	NcDim *_n = file.get_dim("num_types");
	NcVar *_lat = file.get_var("lat");
	NcVar *_lon = file.get_var("lon");
	NcVar *bot_num = file.get_var("type");
	NcVar *bot_speed = file.get_var("speed_ratio");
	NcVar *bot_density = file.get_var("density_ratio");
	NcVar *bot_atten = file.get_var("atten");
	NcVar *bot_shear_speed = file.get_var("shear_speed");
	NcVar *bot_shear_atten = file.get_var("shear_atten");

	float latIn[2];
        _lat->get(latIn, 2);
	float lonIn[2];
        _lon->get(lonIn, 2);
	float numIn[2][2];
        bot_num->get(&numIn[0][0], 2, 2);
	float speedIn[2];
        bot_speed->get(speedIn, 2);
	float densityIn[2];
        bot_density->get(densityIn, 2);
	float attenIn[2];
        bot_atten->get(attenIn, 2);
	float shearspdIn[2];
        bot_shear_speed->get(shearspdIn, 2);
    float shearattnIn[2];
        bot_shear_atten->get(shearattnIn, 2);

/***************************************************************
	double bot_prov_dataIn[sizeof(_lat)][sizeof(_lon)];

	data_grid<double,2> bottom_prov_grid;
	bottom_prov_grid->get(bot_prov_dataIn[0][0],)
***************************************************************/

    cout << "The latitudes are: " << latIn[0] << ", " << latIn[1] << endl;
    cout << "The longitutdes are: " << lonIn[0] << ", " << lonIn[1] << endl;
    cout << "The data field is: " << numIn[0][0] << ", " << numIn[1][0] << ", " << numIn[1][0] << ", " << numIn[1][1] << endl;
    cout << "The speeds are: " << speedIn[0] << ", " << speedIn[1] << endl;
    cout << "The densities are: " << densityIn[0] << ", " << densityIn[1] << endl;
    cout << "The attentuations are: " << attenIn[0] << ", " << attenIn[1] << endl;
    cout << "The shear speeds are: " << shearspdIn[0] << ", " << shearspdIn[1] << endl;
    cout << "The shear attentuations are: " << shearattnIn[0] << ", " << shearattnIn[1] << endl;

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
