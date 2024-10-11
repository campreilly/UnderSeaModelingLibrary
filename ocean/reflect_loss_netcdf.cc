/**
 * @file reflect_loss_netcdf.cc
 * Builds rayleigh models for an imported netcdf bottom province file.
 */
#include <usml/ocean/reflect_loss_netcdf.h>
#include <usml/types/gen_grid.h>

#include <exception>
#include <netcdf>

using namespace usml::ocean;

/**
 * Loads bottom province data from a netCDF formatted file.
 */
reflect_loss_netcdf::reflect_loss_netcdf(const char* filename) {
    netCDF::NcFile file(filename, netCDF::NcFile::read);

    netCDF::NcVar bot_speed = file.getVar("speed_ratio");
    netCDF::NcVar bot_density = file.getVar("density_ratio");
    netCDF::NcVar bot_atten = file.getVar("atten");
    netCDF::NcVar bot_shear_speed = file.getVar("shear_speed");
    netCDF::NcVar bot_shear_atten = file.getVar("shear_atten");
    netCDF::NcVar lon = file.getVar("longitude");
    netCDF::NcVar lat = file.getVar("latitude");
    netCDF::NcVar bot_num = file.getVar("type");

    // gets the size of the dimensions to be used to create the data grid

    int ncid = ncopen(filename, NC_NOWRITE);

    int latid = ncdimid(ncid, "latitude");
    long latdim;
    ncdiminq(ncid, latid, nullptr, &latdim);

    int lonid = ncdimid(ncid, "longitude");
    long londim;
    ncdiminq(ncid, lonid, nullptr, &londim);

    int types = ncdimid(ncid, "speed_ratio");
    long n_types;
    ncdiminq(ncid, types, nullptr, &n_types);

    ncclose(ncid);

    // extracts the data for all of the variables from the netcdf file

    auto* latitude = new double[latdim];
    auto* longitude = new double[londim];
    auto* type_num = new double[latdim * londim];
    auto* speed = new double[n_types];
    auto* density = new double[n_types];
    auto* atten = new double[n_types];
    auto* shearspd = new double[n_types];
    auto* shearatten = new double[n_types];

    lat.getVar(latitude);
    lon.getVar(longitude);
    bot_num.getVar(type_num);
    bot_speed.getVar(speed);
    bot_density.getVar(density);
    bot_atten.getVar(atten);
    bot_shear_speed.getVar(shearspd);
    bot_shear_atten.getVar(shearatten);

    // creates a sequence vector of axes that are passed to data grid

    seq_vector::csptr axis[2];
    double latinc = (latitude[latdim - 1] - latitude[0]) / double(latdim);
    double loninc = (longitude[londim - 1] - longitude[0]) / double(londim);
    auto* seq_lat = new seq_linear(latitude[0], latinc, int(latdim));
    auto* seq_lon = new seq_linear(longitude[0], loninc, int(londim));
    axis[0] = seq_vector::csptr(seq_lat);
    axis[1] = seq_vector::csptr(seq_lon);

    // creates a data grid and populates the data from the netcdf file

    auto* grid = new gen_grid<2>(axis);
    size_t index[2];
    for (int i = 0; i < latdim; i++) {
        for (int j = 0; j < londim; j++) {
            index[0] = i;
            index[1] = j;
            grid->setdata(index, type_num[i * londim + j]);
        }
    }
    _bottom_grid = data_grid<2>::csptr(grid);

    // set the interpolation type to the nearest neighbor and restrict extrap

    for (int i = 0; i < 2; i++) {
        grid->interp_type(i, interp_enum::nearest);
        grid->edge_limit(i, true);
    }

    // builds a vector of reflect_loss_rayleigh for all bottom province numbers

    for (int i = 0; i < int(n_types); i++) {
        auto* model = new reflect_loss_rayleigh(density[i], speed[i], atten[i],
                                                shearspd[i], shearatten[i]);
        _loss_model.push_back(reflect_loss_model::csptr(model));
    }

    delete[] latitude;
    delete[] longitude;
    delete[] speed;
    delete[] density;
    delete[] atten;
    delete[] shearspd;
    delete[] shearatten;
    delete[] type_num;
}

/**
 * Gets a Rayleigh reflection loss value for the bottom province number
 * at a specific location then computes the broadband reflection loss and phase
 * change.
 *
 * @param location      Location at which to compute attenuation.
 * @param frequencies   Frequencies over which to compute loss. (Hz)
 * @param angle         Reflection angle relative to the normal (radians).
 * @param amplitude     Change in ray strength in dB (output).
 * @param phase         Change in ray phase in radians (output).
 *                      Phase change not computed if this is nullptr.
 */
void reflect_loss_netcdf::reflect_loss(const wposition1& location,
                                       const seq_vector::csptr& frequencies,
                                       double angle, vector<double>* amplitude,
                                       vector<double>* phase) const {
    double loc[2];
    loc[0] = location.latitude();
    loc[1] = location.longitude();

    auto type = (size_t)_bottom_grid->interpolate(loc);
    _loss_model[type]->reflect_loss(location, frequencies, angle, amplitude,
                                    phase);
}
