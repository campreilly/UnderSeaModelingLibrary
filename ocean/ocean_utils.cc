/**
 * @file ocean_utils.cc
 * Ocean generation utilities.
 */

#include <usml/netcdf/netcdf_bathy.h>
#include <usml/netcdf/netcdf_woa.h>
#include <usml/ocean/attenuation_constant.h>
#include <usml/ocean/attenuation_model.h>
#include <usml/ocean/boundary_flat.h>
#include <usml/ocean/boundary_grid.h>
#include <usml/ocean/boundary_model.h>
#include <usml/ocean/data_grid_mackenzie.h>
#include <usml/ocean/ocean_model.h>
#include <usml/ocean/ocean_shared.h>
#include <usml/ocean/ocean_utils.h>
#include <usml/ocean/profile_grid.h>
#include <usml/ocean/profile_linear.h>
#include <usml/ocean/profile_model.h>
#include <usml/ocean/reflect_loss_constant.h>
#include <usml/ocean/reflect_loss_eckart.h>
#include <usml/ocean/reflect_loss_model.h>
#include <usml/ocean/reflect_loss_rayleigh.h>
#include <usml/ocean/scattering_constant.h>
#include <usml/ocean/scattering_lambert.h>
#include <usml/types/data_grid.h>

using namespace usml::netcdf;
using namespace usml::ocean;

/**
 * Creates a isovelocity ocean with no absorption and a flat bottom.
 */
void ocean_utils::make_iso(double depth, double bottom_loss) {
    // build ocean surface model

    boundary_model::csptr surface(new boundary_flat());

    // build ocean bottom model

    reflect_loss_model::csptr botloss(new reflect_loss_constant(bottom_loss));
    scattering_model::csptr scattering(new scattering_constant(-30.0));
    boundary_model::csptr bottom(
        new boundary_flat(-depth, botloss, scattering));

    // build sound speed model

    attenuation_model::csptr attn(new attenuation_constant(0.0));
    profile_model::csptr profile(new profile_linear(1500.0,attn));

    ocean_model::csptr ocean(new ocean_model(surface, bottom, profile));
    ocean_shared::update(ocean);
}

/**
 * Creates a simple, but realistic, ocean from the databases delivered with
 * USML.
 */
void ocean_utils::make_basic(double south, double north, double west,
                             double east, int month, double wind_speed,
                             bottom_type_enum bottom_type) {
    // build ocean surface model
    // TODO Implement default surface scattering model

    reflect_loss_model::csptr surfloss(new reflect_loss_eckart(wind_speed));
    scattering_model::csptr surfscat(new scattering_lambert());
    boundary_model::csptr surface(new boundary_flat(0.0, surfloss, surfscat));

    // load bathymetry from ETOPO1 database

    reflect_loss_model::csptr botloss(new reflect_loss_rayleigh(bottom_type));
    scattering_model::csptr botscat(new scattering_lambert());
    data_grid<2>::csptr grid(
        new netcdf_bathy(USML_DATA_DIR "/bathymetry/ETOPO1_Ice_g_gmt4.grd",
                         south, north, west, east));
    boundary_grid<2>::csptr bottom(
        new boundary_grid<2>(grid, botloss, botscat));

    // build sound velocity profile from World Ocean Atlas data

    netcdf_woa::csptr temperature(
        new netcdf_woa(USML_DATA_DIR "/woa09/temperature_seasonal_1deg.nc",
                       USML_DATA_DIR "/woa09/temperature_monthly_1deg.nc",
                       month, south, north, west, east));

    netcdf_woa::csptr salinity(
        new netcdf_woa(USML_DATA_DIR "/woa09/salinity_seasonal_1deg.nc",
                       USML_DATA_DIR "/woa09/salinity_monthly_1deg.nc", month,
                       south, north, west, east));

    data_grid<3>::csptr ssp(new data_grid_mackenzie(temperature, salinity));
    profile_grid<3>::csptr profile(new profile_grid<3>(ssp));

    // create shared ocean

    ocean_model::csptr ocean(new ocean_model(surface, bottom, profile));
    ocean_shared::update(ocean);
}
