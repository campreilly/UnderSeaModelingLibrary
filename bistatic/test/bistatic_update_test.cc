/**
 * @example bistatic/test/bistatic_update_test.cc
 */

#include <usml/beampatterns/beampatterns.h>
#include <usml/bistatic/bistatic.h>
#include <usml/netcdf/netcdf_files.h>
#include <usml/ocean/ocean.h>
#include <usml/platforms/platforms.h>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(bistatic_update_test)

using namespace boost::unit_test;
using namespace usml::eigenverbs;
using namespace usml::eigenrays;
using namespace usml::netcdf;
using namespace usml::ocean;
using namespace usml::platforms;
using namespace usml::bistatic;

/**
 * Listen for eigenray updates on sensor.
 */
class pair_listener : public update_listener<bistatic_pair> {
   public:
    /**
     * Notify listeners of updates to bistatic_pair.
     *
     * @param pair  Reference to updated bistatic_pair.
     */
    void notify_update(const bistatic_pair* pair) override {
        cout << "bistatic_update_test::notify_update "
             << pair->source()->description() << " -> "
             << pair->receiver()->description() << endl;
    }
};
pair_listener test_listener;

/**
 * Load environmental data for area of operations into shared ocean.
 */
void build_ocean() {
    int month = 8;             // August
    const double lat1 = 34.0;  // Malta Escarpment
    const double lat2 = 38.0;
    const double lng1 = 15.0;
    const double lng2 = 19.0;

    // load bathymetry from ETOPO1 database

    data_grid<2>::csptr grid(
        new netcdf_bathy(USML_DATA_DIR "/bathymetry/ETOPO1_Ice_g_gmt4.grd",
                         lat1, lat2, lng1, lng2));
    data_grid_bathy::csptr fast_grid(new data_grid_bathy(grid));
    cout << "load bathymetry from ETOPO1 database" << endl;
    boundary_grid<2>::csptr bottom(new boundary_grid<2>(fast_grid));

    // build sound velocity profile from World Ocean Atlas data

    cout << "load temperature & salinity data from World Ocean Atlas" << endl;
    netcdf_woa::csptr temperature(
        new netcdf_woa(USML_DATA_DIR "/woa09/temperature_seasonal_1deg.nc",
                       USML_DATA_DIR "/woa09/temperature_monthly_1deg.nc",
                       month, lat1, lat2, lng1, lng2));
    temperature.get()->write_netcdf("temperature.nc");
    netcdf_woa::csptr salinity(
        new netcdf_woa(USML_DATA_DIR "/woa09/salinity_seasonal_1deg.nc",
                       USML_DATA_DIR "/woa09/salinity_monthly_1deg.nc", month,
                       lat1, lat2, lng1, lng2));
    salinity.get()->write_netcdf("salinity.nc");

    data_grid<3>::csptr ssp(new data_grid_mackenzie(temperature, salinity));
    ssp.get()->write_netcdf("ssp.nc");
    profile_grid<3>::csptr profile(new profile_grid<3>(ssp));

    // create shared ocean

    boundary_model::csptr surface(new boundary_flat());
    ocean_model::csptr ocean(new ocean_model(surface, bottom, profile));
    ocean_shared::update(ocean);
}

/**
 * Load five sensor locations into platform_manager from a static database of
 * positions off the Malta Escarpment around 36:00N and 17:00E. The minimum
 * range for each platform is set to 1 meter to see if the wavefront_generator
 * can exclude targets from its list.
 */
void build_platforms() {
    auto* platform_mgr = platform_manager::instance();
    auto* bistatic_mgr = bistatic_manager::instance();

    // define frequencies for calculation

    seq_vector::csptr freq(new seq_linear(900.0, 10.0, 1000.0));
    platform_mgr->frequencies(freq);

    // static database of sensor locations (latitude,longitude,altitude)

    // clang-format off
    static double pos[][3] = {
		{35.9, 17.0, -100},
		{36.0, 17.0, -100},
		{36.1, 17.0, -100},
		{36.1, 17.0, -500},
		{36.0, 17.1, -100},
    };
    // clang-format on

    // create platform with single omni sensor

    auto num_sites = 5;
    auto max_time = 8.0;
    for (platform_model::key_type site = 1; site <= num_sites; ++site) {
        std::ostringstream name;
        name << "site" << site;
        const int index = site - 1;

        cout << "add sensor " << name.str() << " (" << pos[index][0] << ","
             << pos[index][1] << "," << pos[index][2] << ")" << endl;
        wposition1 position(pos[index][0], pos[index][1], pos[index][2]);
        auto* sensor = new sensor_model(site, name.str(), 0.0, position);
        auto platform = platform_model::sptr(sensor);

        sensor->time_maximum(max_time);
        sensor->compute_reverb(false);
        if (site > 1) {
            sensor->multistatic(true);
        }
        auto beam = bp_model::csptr(new bp_omni());
        sensor->src_beam(0, beam);
        sensor->rcv_beam(0, beam);

        platform_mgr->add(platform);
        bistatic_mgr->add_sensor(platform, &test_listener);
        platform->update(0.0, platform_model::FORCE_UPDATE);
    }
}

/**
 * @ingroup bistatic_test
 * @{
 */

/**
 * This test computes eigenrays from a receiver to all sources.
 * It is used as a fundamental test to see if entries can be created
 * and if notifications work properly.
 */
BOOST_AUTO_TEST_CASE(setup_platforms) {
    cout << "=== bistatic_update_test: setup_platforms ===" << endl;

    // build scenario and wait for setup to complete

    build_ocean();
    build_platforms();
    while (thread_task::num_active() > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // display final setup

    auto* bistatic_mgr = bistatic_manager::instance();
    cout << endl << "*** pairs ***" << endl;
    for (const auto& pair : bistatic_mgr->list()) {
        cout << "src=" << pair->source()->description()
             << " rcv=" << pair->receiver()->description()
             << " dirpaths=" << pair->dirpaths()->eigenrays().size() << endl;
    }

    // clean up and exit

    cout << "clean up" << endl;
    bistatic_manager::reset();
    platform_manager::reset();
    thread_controller::reset();
}

/// @}
BOOST_AUTO_TEST_SUITE_END()
