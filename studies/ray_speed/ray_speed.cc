/**
 * @file ray_speed.cc
 *
 * Measure the speed of the model in a realistic scenario.
 * Uses the World Ocean Atlas and the ETOPO1 databases to construct a
 * real world environment near the Malta escarpment south-east of Sicily.
 *
 *      - Area 30N-46N, 8W-37E (entire Mediterranean Sea)
 *      - Month: December
 *      - Source: 36N, 16.0E, 100 meters deep
 *      - Targets: 6x6 grid +/- 2.5 degrees around source
 *      - Frequency: 3000 Hz
 *      - Travel Time: 60 seconds
 *      - Time Step: 50 msec
 *      - D/E: [-90,90] as 181 tangent spaced rays
 *      - AZ: [0,360] in 5.0 deg steps
 *
 */

#include <usml/netcdf/netcdf_bathy.h>
#include <usml/netcdf/netcdf_woa.h>
#include <usml/ocean/boundary_flat.h>
#include <usml/ocean/boundary_grid.h>
#include <usml/ocean/boundary_model.h>
#include <usml/ocean/data_grid_mackenzie.h>
#include <usml/ocean/ocean_model.h>
#include <usml/ocean/profile_grid.h>
#include <usml/ocean/profile_model.h>
#include <usml/types/data_grid.h>
#include <usml/types/seq_linear.h>
#include <usml/types/seq_log.h>
#include <usml/types/seq_rayfan.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition.h>
#include <usml/types/wposition1.h>
#include <usml/types/wvector.h>
#include <usml/ublas/randgen.h>
#include <usml/waveq3d/wave_queue.h>

#include <boost/timer/timer.hpp>
#include <usml/eigenrays/eigenray_collection.h>
#include <cstddef>
#include <cstdlib>
#include <iostream>

using namespace usml::waveq3d;
using namespace usml::netcdf;
using namespace usml::ocean;

/**
 * Command line interface.
 */
int main(int argc, char* argv[]) {
    cout << "=== ray_speed ===" << endl;

    int num_targets = 100;
    if (argc > 1) {
        num_targets = atoi(argv[1]);
    }

    // define scenario parameters

    int month = 12;            // December
    const double lat1 = 30.0;  // entire Mediterranean Sea
    const double lat2 = 46.0;
    const double lng1 = -8.0;
    const double lng2 = 37.0;

    wposition1 pos(36.0, 16.0, -10.0);
    seq_vector::csptr de(new seq_rayfan(-90.0, 90.0, 181));
    seq_vector::csptr az(new seq_linear(0.0, 15.0, 360.0));
    const double time_max = 60.0;
    const double time_step = 0.100;
    seq_vector::csptr freq(new seq_log(3000.0, 1.0, 1));

    // build sound velocity profile from World Ocean Atlas data

    cout << "load temperature & salinity data from World Ocean Atlas" << endl;
    data_grid<3>::csptr temperature(
        new netcdf_woa(USML_DATA_DIR "/woa09/temperature_seasonal_1deg.nc",
                       USML_DATA_DIR "/woa09/temperature_monthly_1deg.nc",
                       month, lat1, lat2, lng1, lng2));
    data_grid<3>::csptr salinity(
        new netcdf_woa(USML_DATA_DIR "/woa09/salinity_seasonal_1deg.nc",
                       USML_DATA_DIR "/woa09/salinity_monthly_1deg.nc", month,
                       lat1, lat2, lng1, lng2));
    data_grid<3>::csptr ssp(new data_grid_mackenzie(temperature, salinity));
    profile_grid<3>::csptr profile(new profile_grid<3>(ssp));

    // load bathymetry from ETOPO1 database

    cout << "load bathymetry from ETOPO1 database" << endl;
    data_grid<2>::csptr grid(
        new netcdf_bathy(USML_DATA_DIR "/bathymetry/ETOPO1_Ice_g_gmt4.grd",
                         lat1, lat2, lng1, lng2));
    boundary_grid<2>::csptr bottom(new boundary_grid<2>(grid));

    // combine sound speed and bathymetry into ocean model

    boundary_model::csptr surface(new boundary_flat());
    ocean_model::csptr ocean(new ocean_model(surface, bottom, profile));

    // initialize eigenray_collection targets and wavefront

    cout << "initialize " << num_targets << " targets" << endl;
    randgen random;
    random.seed(0);  // fix the initial seed
    wposition targets(num_targets, 1, pos.latitude(), pos.longitude(),
                      pos.altitude());
    for (size_t n = 0; n < targets.size1(); ++n) {
        targets.latitude(n, 0, pos.latitude() + random.uniform() - 0.5);
        targets.longitude(n, 0, pos.longitude() + random.uniform() - 0.5);
    }
    eigenray_collection eigenrays(freq, pos, targets);
    wave_queue wave(ocean, freq, pos, de, az, time_step, &targets);
    wave.add_eigenray_listener(&eigenrays);

    // propagate wavefront

    cout << "propagate wavefronts for " << time_max << " secs" << endl;
    {
        boost::timer::auto_cpu_timer timer(3, "%w secs");
        while (wave.time() < time_max) {
            wave.step();
        }
    }
    cout << endl;
}
