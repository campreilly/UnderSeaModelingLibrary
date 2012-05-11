/** 
 * @file netcdf_bathy.h 
 * Extracts bathymetry data from world-wide bathymetry databases.
 */
#ifndef USML_NETCDF_BATHY_H
#define USML_NETCDF_BATHY_H

#include <netcdfcpp.h>
#include <usml/ublas/ublas.h>
#include <usml/types/types.h>

namespace usml {
namespace netcdf {

using namespace usml::ublas ;
using namespace usml::types ;

/// @ingroup netcdf_files
/// @{

/**
 * Extracts bathymetry data from world-wide bathymetry databases.
 * Stores the latitude, longitude, and depth in spherical earth
 * coordinates for faster interpolation within the WaveQ3D model.
 *
 * Deduces the variables to be loaded based on their dimensionality.
 * The first variable to have 2 dimensions is assumed to be depth.
 * Negative depth values in netCDF file are taken to be underwater.
 * Assumes that the dataset supports the COARDS conventions for 
 * the standardization of NetCDF files.
 *
 * - The first dimension of the depth is assumed to be latitude.
 * - The second dimension of the depth is assumed to be longitude.
 *   This implementation automatically unwraps differences between
 *   the [0,360) and the [-180,180) longitude range.
 *
 * This may seem like a pretty loose specification, but this looseness is very
 * helpful in automating the reading NetCDF files from a variety of sources.
 *
 * Successfully tested using ETOPO1, ETOPO2, and ETOPO5 data:
 * - ETOPO1 grid/node-registered ice surface data. Grid of Earth's surface
 *   depicting the top of the Antarctic and Greenland ice sheets.
 *   Cells are centered on lines of latitude and longitude.
 * - ETOPO2 gridline registered, with cell boundaries defined by lines
 *   of odd minutes of latitude and longitude.
 * - ETOPO5 5-minute gridded elevations/bathymetry for the world.
 */
class USML_DECLSPEC netcdf_bathy : public data_grid<double,2> {

  public:  

    /**
     * Load bathymetry from disk. Western hemisphere longitude can be 
     * expressed either as negative values or values above 180 degrees.
     * Output longitudes use the same western hemisphere convention as
     * input values.  Exceptions to this logic happen in areas that span 
     * longitudes 0 and 180.  Areas that span longitude 0 should use negative
     * values for west and positive values for east. Areas that span 
     * longitude 180 should use positive values for both east and west.
     *
     * @param  filename     Name of the NetCDF file to load.
     * @param  south        Lower limit for the latitude axis (degrees).
     * @param  north        Upper limit for the latitude axis (degrees).
     * @param  west         Lower limit for the longitude axis (degrees).
     * @param  east         Upper limit for the longitude axis (degrees).
     * @param  earth_radius Local earth radius of curvature (meters).
     *                      Set to zero if you want to make depths
     *                      relative to earth's surface.
     */
    netcdf_bathy( 
        const char* filename,
        double south, double north, double west, double east,
        double earth_radius=wposition::earth_radius ) ;

  private:
  
    /**
     * Deduces the variables to be loaded based on their dimensionality.
     * The first variable to have 2 dimensions is assumed to be depth.
     * The 2 dimensions of the depth are assumed to be (in order)
     * latitude and longitude in compliance with the COOARDS standards.
     *
     * @param  file         Reference to an open NetCDF file.
     * @param  latitude     NetCDF variable for latitude (output).
     * @param  longitude    NetCDF variable for longitude (output).
     * @param  altitude     NetCDF variable for altitude (output).
     */
     void decode_filetype( 
        NcFile& file, NcVar **latitude, NcVar **longitude, NcVar **altitude ) ;
} ;

/// @}
}  // end of namespace netcdf
}  // end of namespace usml

#endif
