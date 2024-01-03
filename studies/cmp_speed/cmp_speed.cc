/**
 * @file cmp_speed.cc
 *
 * Compare the speed of the model to the STD14 scenario from CASS.
 * In this scenario, CASS specifies a grid of sound speeds and
 * bottom depths in latitude and longitude coordinates for an
 * area near Hawaii.
 *
 *      - Area 16.2N-24.6N, 164.7-155.4W
 *      - Source: 19.52N 160.5W, 200 meters deep (SW of Hawaii)
 *      - Targets: ring of receivers at 100 km from source
 *      - Frequency: 250 Hz
 *      - Travel Time: 80 seconds
 *      - Time Step: 100 msec
 *      - D/E: [-90,90] as 181 tangent spaced rays
 *      - AZ: [0,360] in 15.0 deg steps
 *
 */

#include <usml/eigenrays/eigenray_collection.h>
#include <usml/netcdf/netcdf_bathy.h>
#include <usml/netcdf/netcdf_profile.h>
#include <usml/ocean/boundary_flat.h>
#include <usml/ocean/boundary_grid.h>
#include <usml/ocean/boundary_model.h>
#include <usml/ocean/ocean_model.h>
#include <usml/ocean/profile_grid.h>
#include <usml/ocean/profile_model.h>
#include <usml/types/data_grid.h>
#include <usml/types/data_grid_bathy.h>
#include <usml/types/data_grid_svp.h>
#include <usml/types/seq_linear.h>
#include <usml/types/seq_rayfan.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition.h>
#include <usml/types/wposition1.h>
#include <usml/ublas/math_traits.h>
#include <usml/waveq3d/wave_queue.h>

#include <boost/timer/timer.hpp>
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
    cout << "=== cmp_speed ===" << endl;

    int num_targets = 100;
    if (argc > 1) {
        num_targets = atoi(argv[1]);
    }

    // define scenario parameters

    seq_vector::csptr freq(new seq_linear(250.0, 1.0, 1));
    wposition1 src_pos(19.52, -160.5, -200.0);
    seq_vector::csptr de(new seq_rayfan(-90.0, 90.0, 181));
    seq_vector::csptr az(new seq_linear(0.0, 15.0, 360.0));
    const double target_depth = 100.0;     // meters
    const double target_range = 100000.0;  // meters
    const double time_max = 80.0;
    const double time_step = 0.100;

    // load STD14 environmental data from netCDF files

    const double lat1 = 16.2;
    const double lat2 = 24.6;
    const double lng1 = -164.4;
    const double lng2 = -155.5;

    cout << "load STD14 environmental profile data from " << USML_STUDIES_DIR
         << "/cmp_speed/std14profile.nc" << endl;
    data_grid<3>::csptr ssp(new netcdf_profile(USML_STUDIES_DIR
                                               "/cmp_speed/std14profile.nc",
                                               0.0, lat1, lat2, lng1, lng2));
    // data_grid<3>::csptr fast_ssp(new data_grid_svp(ssp));
    // profile_grid<3>::csptr profile(new profile_grid<3>(fast_ssp));
    profile_grid<3>::csptr profile(new profile_grid<3>(ssp));

    cout << "load STD14 environmental bathy data from " << USML_STUDIES_DIR
            << "/cmp_speed/std14bathy.nc" << endl;
    data_grid<2>::csptr grid(
        new netcdf_bathy(USML_STUDIES_DIR "/cmp_speed/std14bathy.nc", lat1,
                         lat2, lng1, lng2, wposition::earth_radius));
    // data_grid<2>::csptr fast_grid(new data_grid_bathy(grid));
    // boundary_model::csptr bottom(new boundary_grid<2>(fast_grid));
    boundary_model::csptr bottom(new boundary_grid<2>(grid));

    boundary_model::csptr surface(new boundary_flat());
    ocean_model::csptr ocean(new ocean_model(surface, bottom, profile));

    // initialize eigenray_collection targets and wavefront

    cout << "initialize " << num_targets << " targets" << endl;
    wposition target(num_targets, 1, src_pos.latitude(), src_pos.longitude(),
                     target_depth);
    double angle = TWO_PI / num_targets;
    double bearing_inc = angle;
    for (size_t n = 0; n < num_targets; ++n) {
        wposition1 spot(src_pos, target_range, bearing_inc);
        target.latitude(n, 0, spot.latitude());
        target.longitude(n, 0, spot.longitude());
        target.altitude(n, 0, spot.altitude());
        bearing_inc = bearing_inc + angle;
    }
    eigenray_collection loss(freq, src_pos, target);
    wave_queue wave(ocean, freq, src_pos, de, az, time_step, &target);
    wave.add_eigenray_listener(&loss);

    // propagate wavefront

    cout << "propagate wavefronts for " << time_max << " secs" << endl;
    {
        boost::timer::auto_cpu_timer timer(3, "%w secs\n");
        while (wave.time() < time_max) {
            wave.step();
        }
    }
    cout << endl;
}
