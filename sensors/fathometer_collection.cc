/**
 * @file fathometer_collection.cc
 * Container for one fathometer_collection instance.
 */

#include <usml/sensors/fathometer_collection.h>
#include <boost/foreach.hpp>

#include <netcdfcpp.h>

using namespace boost ;
using namespace usml::sensors ;


/**
 * Updates the fathometer data with the parameters provided.
 */
void fathometer_collection::dead_reckon(double delta_time, double slant_range, double prev_range) {

    // Set new slant_range
    _slant_range = slant_range;

    write_lock_guard guard(_eigenrays_mutex);

    BOOST_FOREACH ( eigenray ray, _eigenrays) {

        ray.time = ray.time + delta_time;
        for (int i = 0; i < ray.frequencies->size(); ++i) {
            ray.intensity[i] = ray.intensity[i] +
                (20*log10(prev_range)) - (20*log10(_slant_range));
        }
    }
}

/**
 * Write fathometer_collection data to to netCDF file.
 */
void fathometer_collection::write_netcdf( const char* filename, const char* long_name )
{
    NcFile* nc_file = new NcFile(filename, NcFile::Replace);
    if (long_name) {
        nc_file->add_att("long_name", long_name);
    }
    nc_file->add_att("Conventions", "COARDS");

    if ( _eigenrays.size() == 0 ) {
        nc_file->add_att("Eigenrays", "None Found");
        // close file
        delete nc_file;
        return;
    }
    // Get the list to get the frequency size
    long num_frequencies = ( long ) _eigenrays.front().frequencies->size();

    // dimensions

    NcDim *freq_dim = nc_file->add_dim("frequency", num_frequencies);
    NcVar *freq_var = nc_file->add_var("frequency", ncDouble, freq_dim);
    NcDim *eigenray_dim = nc_file->add_dim("eigenrays", ( long ) _eigenrays.size());
   
    // fathometer_collection attributes

    NcVar *source_id = nc_file->add_var("source_id", ncShort);
    NcVar *receiver_id = nc_file->add_var("receiver_id", ncShort);
    NcVar *initial_time = nc_file->add_var("initial_time", ncDouble);
    NcVar *slant_range = nc_file->add_var("slant_range", ncDouble);

    // coordinates

    NcVar *src_lat_var = nc_file->add_var("source_latitude", ncDouble);
    NcVar *src_lng_var = nc_file->add_var("source_longitude", ncDouble);
    NcVar *src_alt_var = nc_file->add_var("source_altitude", ncDouble);

    NcVar *rcv_lat_var = nc_file->add_var("receiver_latitude", ncDouble);
    NcVar *rcv_lng_var = nc_file->add_var("receiver_longitude", ncDouble);
    NcVar *rcv_alt_var = nc_file->add_var("receiver_altitude", ncDouble);

    NcVar *intensity_var = nc_file->add_var("intensity", ncDouble, eigenray_dim, freq_dim);
    NcVar *phase_var = nc_file->add_var("phase", ncDouble, eigenray_dim, freq_dim);
    NcVar *time_var = nc_file->add_var("travel_time", ncDouble, eigenray_dim);
    NcVar *source_de_var = nc_file->add_var("source_de", ncDouble, eigenray_dim);
    NcVar *source_az_var = nc_file->add_var("source_az", ncDouble, eigenray_dim);
    NcVar *target_de_var = nc_file->add_var("target_de", ncDouble, eigenray_dim);
    NcVar *target_az_var = nc_file->add_var("target_az", ncDouble, eigenray_dim);
    NcVar *surface_var = nc_file->add_var("surface", ncShort, eigenray_dim);
    NcVar *bottom_var = nc_file->add_var("bottom", ncShort, eigenray_dim);
    NcVar *caustic_var = nc_file->add_var("caustic", ncShort, eigenray_dim);

    // units

    freq_var->add_att("units", "Hertz");
    src_lat_var->add_att("units", "degrees_north");
    src_lng_var->add_att("units", "degrees_east");
    src_alt_var->add_att("units", "meters");
    src_alt_var->add_att("positive", "up");

    rcv_lat_var->add_att("units", "degrees_north");
    rcv_lng_var->add_att("units", "degrees_east");
    rcv_alt_var->add_att("units", "meters");
    rcv_alt_var->add_att("positive", "up");

    intensity_var->add_att("units", "dB");
    phase_var->add_att("units", "radians");
    time_var->add_att("units", "seconds");

    source_de_var->add_att("units", "degrees");
    source_de_var->add_att("positive", "up");
    source_az_var->add_att("units", "degrees_true");
    source_az_var->add_att("positive", "clockwise");

    target_de_var->add_att("units", "degrees");
    target_de_var->add_att("positive", "up");
    target_az_var->add_att("units", "degrees_true");
    target_az_var->add_att("positive", "clockwise");

    surface_var->add_att("units", "count");
    bottom_var->add_att("units", "count");
    caustic_var->add_att("units", "count");

    int item;
    double v;
    int record = 0; // current record number

    
    // write base attributes

    freq_var->put(_eigenrays.front().frequencies->data().begin(), num_frequencies);
    item = _source_id; source_id->put(&item, 1);
    item = _receiver_id; receiver_id->put(&item, 1); 

    v = _initial_time;  initial_time->put(&v, 1);
    v = _slant_range;   slant_range->put(&v, 1);

    // write source parameters

    v = _source_position.latitude();    src_lat_var->put(&v, 1);
    v = _source_position.longitude();   src_lng_var->put(&v, 1);
    v = _source_position.altitude();    src_alt_var->put(&v, 1);

    // write receiver parameters

    v = _receiver_position.latitude();    rcv_lat_var->put(&v, 1);
    v = _receiver_position.longitude();   rcv_lng_var->put(&v, 1);
    v = _receiver_position.altitude();    rcv_alt_var->put(&v, 1);

    BOOST_FOREACH(eigenray ray, _eigenrays)
    {
        // set record number for each eigenray data element

        intensity_var->set_cur(record);
        phase_var->set_cur(record);
        time_var->set_cur(record);
        source_de_var->set_cur(record);
        source_az_var->set_cur(record);
        target_de_var->set_cur(record);
        target_az_var->set_cur(record);
        surface_var->set_cur(record);
        bottom_var->set_cur(record);
        caustic_var->set_cur(record);
        ++record;

        intensity_var->put(ray.intensity.data().begin(), 1, num_frequencies);
        phase_var->put(ray.phase.data().begin(), 1, num_frequencies);
        time_var->put(&( ray.time ), 1);
        source_de_var->put(&( ray.source_de ), 1);
        source_az_var->put(&( ray.source_az ), 1);
        target_de_var->put(&( ray.target_de ), 1);
        target_az_var->put(&( ray.target_az ), 1);
        surface_var->put(&( ray.surface ), 1);
        bottom_var->put(&( ray.bottom ), 1);
        caustic_var->put(&( ray.caustic ), 1);

    } // loop over # of eigenrays

    // close file
    delete nc_file; // destructor frees all netCDF temp variables
}


