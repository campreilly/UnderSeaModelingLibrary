/**
 * @file wave_queue_netcdf.cc
 * Recording to netCDF wavefront log
 */
#include <usml/waveq3d/wave_queue.h>

using namespace usml::waveq3d;

/**
 * Initialize recording to netCDF wavefront log.
 */
void wave_queue::init_netcdf(const char *filename, const char *long_name) {
    _nc_file = std::unique_ptr<netCDF::NcFile>(
        new netCDF::NcFile(filename, netCDF::NcFile::replace));
    _nc_rec = 0;
    if (long_name != nullptr) {
        _nc_file->putAtt("long_name", long_name);
    }
    _nc_file->putAtt("Conventions", "COARDS");

    // dimensions

    netCDF::NcDim freq_dim =
        _nc_file->addDim("frequencies", _frequencies->size());
    netCDF::NcDim de_dim = _nc_file->addDim("source_de", _source_de->size());
    netCDF::NcDim az_dim = _nc_file->addDim("source_az", _source_az->size());
    netCDF::NcDim time_dim = _nc_file->addDim("travel_time");  // unlimited
    std::vector<netCDF::NcDim> tda_dim = {time_dim, de_dim, az_dim};

    // coordinates

    netCDF::NcVar freq_var =
        _nc_file->addVar("frequencies", netCDF::NcDouble(), freq_dim);
    netCDF::NcVar de_var =
        _nc_file->addVar("source_de", netCDF::NcDouble(), de_dim);
    netCDF::NcVar az_var =
        _nc_file->addVar("source_az", netCDF::NcDouble(), az_dim);
    _nc_time = _nc_file->addVar("travel_time", netCDF::NcDouble(), time_dim);
    _nc_latitude = _nc_file->addVar("latitude", netCDF::NcDouble(), tda_dim);
    _nc_longitude = _nc_file->addVar("longitude", netCDF::NcDouble(), tda_dim);
    _nc_altitude = _nc_file->addVar("altitude", netCDF::NcDouble(), tda_dim);
    _nc_surface = _nc_file->addVar("surface", netCDF::NcShort(), tda_dim);
    _nc_bottom = _nc_file->addVar("bottom", netCDF::NcShort(), tda_dim);
    _nc_caustic = _nc_file->addVar("caustic", netCDF::NcShort(), tda_dim);
    _nc_upper = _nc_file->addVar("upper", netCDF::NcShort(), tda_dim);
    _nc_lower = _nc_file->addVar("lower", netCDF::NcShort(), tda_dim);
    _nc_on_edge = _nc_file->addVar("on_edge", netCDF::NcByte(), tda_dim);

    // units

    freq_var.putAtt("units", "hertz");
    de_var.putAtt("units", "degrees");
    de_var.putAtt("positive", "up");
    az_var.putAtt("units", "degrees_true");
    az_var.putAtt("positive", "clockwise");
    _nc_time.putAtt("units", "seconds");
    _nc_latitude.putAtt("units", "degrees_north");
    _nc_longitude.putAtt("units", "degrees_east");
    _nc_altitude.putAtt("units", "meters");
    _nc_altitude.putAtt("positive", "up");
    _nc_surface.putAtt("units", "count");
    _nc_bottom.putAtt("units", "count");
    _nc_caustic.putAtt("units", "count");
    _nc_upper.putAtt("units", "count");
    _nc_lower.putAtt("units", "count");
    _nc_on_edge.putAtt("units", "bool");

    // coordinate data

    freq_var.putVar(_frequencies->data().begin());
    de_var.putVar(_source_de->data().begin());
    az_var.putVar(_source_az->data().begin());
}

/**
 * Write current record to netCDF wavefront log.
 */
void wave_queue::save_netcdf() {
    // NcError( verbose_nonfatal ) ;
    const std::vector<size_t> start1 = {_nc_rec};
    const std::vector<size_t> count1 = {1};
    const std::vector<size_t> startp = {_nc_rec, 0, 0};
    const std::vector<size_t> countp = {1, _source_de->size(),
                                        _source_az->size()};
    _nc_time.putVar(startp, countp, &_time);
    _nc_latitude.putVar(startp, countp,
                        _curr->position.latitude().data().begin());
    _nc_longitude.putVar(startp, countp,
                         _curr->position.longitude().data().begin());
    _nc_altitude.putVar(startp, countp,
                        _curr->position.altitude().data().begin());
    _nc_surface.putVar(startp, countp, _curr->surface.data().begin());
    _nc_bottom.putVar(startp, countp, _curr->bottom.data().begin());
    _nc_caustic.putVar(startp, countp, _curr->caustic.data().begin());
    _nc_upper.putVar(startp, countp, _curr->upper.data().begin());
    _nc_lower.putVar(startp, countp, _curr->lower.data().begin());
    _nc_on_edge.putVar(startp, countp,_curr->on_edge.data().begin());
    ++_nc_rec;
}

/**
 * Close netCDF wavefront log.
 */
void wave_queue::close_netcdf() {
    _nc_file.reset();  // destructor frees all netCDF temp variables
}
