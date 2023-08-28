/**
 * @example wavegen/test/platform_update_test.cc
 */

#include <usml/netcdf/netcdf_files.h>
#include <usml/ocean/ocean.h>
#include <usml/platforms/platforms.h>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(platform_update_test)

using namespace boost::unit_test;
using namespace usml::eigenrays;
using namespace usml::netcdf;
using namespace usml::ocean;
using namespace usml::platforms;

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
 * Listen for eigenray updates on sensor.
 */
class sensor_listener : public wavefront_listener {
   public:
    /**
     * Notify listener of new wavefront data (eigenrays and eigenverbs) have
     * been computed for a sensor.
     *
     * @param source 		Sensor model that generated this wavefront data.
     * @param eigenrays 	Shared pointer to an eigenrays computed.
     * @param eigenverbs 	Shared pointer to an eigenverbs computed.
     */
    void update_wavefront_data(
        const sensor_model* sensor, eigenray_collection::csptr eigenrays,
        eigenverb_collection::csptr  /*eigenverbs*/) override {

        // write eigenrays to netCDF file

        std::string fullname = USML_TEST_DIR "/platforms/test/" +
                               sensor->description() + "_eigenrays.nc";
        const char* filename = fullname.c_str();
        cout << "writing eigenrays to " << filename << endl;
        eigenrays->write_netcdf(filename);

        // check to see if eigenrays created correctly

        BOOST_CHECK_EQUAL(eigenrays->size1(), 5);
        BOOST_CHECK_EQUAL(eigenrays->size2(), 1);
    }
};

/**
 * @ingroup wavegen_test
 * @{
 */

/**
 * This test computes eigenrays from a receiver to all sources.
 * It is used as a fundamental test to see if entries can be created
 * and if notifications work properly.
 */
BOOST_AUTO_TEST_CASE(propagate_wavefront) {
    cout << "=== platform_update_test: propagate_wavefront ===" << endl;
    build_ocean();
    platform_manager* platform_mgr = platform_manager::instance();
    sensor_listener listener;

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

    for (platform_model::key_type site = 1; site <= 5; ++site) {
        std::ostringstream name;
        name << "site" << site;
        const int index = site - 1;
        cout << "add sensor " << name.str() << " (" << pos[index][0] << ","
             << pos[index][1] << "," << pos[index][2] << ")" << endl;
        wposition1 position(pos[index][0], pos[index][1], pos[index][2]);
        auto* sensor = new sensor_model(site, name.str(), 0.0, position);
        sensor->time_maximum(8.0);
        sensor->compute_reverb(false);
        sensor->add_wavefront_listener(&listener);
        platform_mgr->add(platform_model::sptr(sensor));
    }

    // update acoustics for sensor #2

    cout << "update acoustics for sensor #2" << endl;
    platform_model::sptr platform = platform_manager::instance()->find(2);
    platform->update(0.0, platform_model::FORCE_UPDATE);
    while (thread_task::num_active() > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    cout << "clean up" << endl;
    platform_manager::reset();
    thread_controller::reset();
}

/// @}
BOOST_AUTO_TEST_SUITE_END()
