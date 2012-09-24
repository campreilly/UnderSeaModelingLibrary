/**
 * @file netcdf_profile.h
 * Extracts ocean profile data from world-wide databases.
 */
#ifndef USML_NETCDF_PROFILE_H
#define USML_NETCDF_PROFILE_H

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
 * Extracts ocean profile data from world-wide databases.
 * These profiles can be a physical characteristic like temperature and
 * salinity, or an acoustic property like sound speed.
 * The three axes are stored in the order: altitude, latitude,
 * longitude. In other words, longitude changes the fastest and altitude
 * the slowest in the index to the underlying 3-D dataset.
 *
 * Stores the latitude, longitude, and depth in spherical earth
 * coordinates for faster interpolation within the WaveQ3D model.
 * This choice of coordinates means that the latitude, longitude axes
 * are actually stored in the form of a colatitude and azimuth in radians.
 * Because these databases store their data such that latitudes area increasing,
 * the co-latitude axis starts from it's largest value, and then uses
 * a negative increment to sequence down to its smallest value.  In other words,
 * the data appear to be "upside-down" in spherical earth coordinates.
 *
 * This implementation replaces all missing values from the input file
 * with NaN.  The fill_missing() routine allows developers to replace
 * these NaN values with the average data at each depth.
 *
 * Deduces the variables to be loaded based on their dimensionality.
 * The first variable to have 4 dimensions is assumed to be sound speed
 * (or its temperature/salinity equivalents). Assumes that the dataset
 * supports the COARDS conventions for the standardization of NetCDF files.
 *
 * - The first dimension of the profile is assumed to be month.
 * - The second dimension of the profile is assumed to be depth.
 *   The negative of the depth value must be used to convert into the
 *   altitude form used in the rest of our coordinate system.
 * - The third dimension of the profile is assumed to be latitude.
 * - The forth dimension of the profile is assumed to be longitude.
 *   This implementation automatically unwraps differences between
 *   the [0,360) and the [-180,180) longitude range.
 *
 * This may seem like a pretty loose specification. But, this looseness is very
 * helpful in automating the reading NetCDF files from a variety of sources.
 */
class USML_DECLSPEC netcdf_profile : public data_grid<double,3> {

  public:

    /**
     * Load ocean profile from disk. Missing values are replaced by NAN.
     *
     * Western hemisphere longitude can be expressed either as negative
     * values or values above 180 degrees. Output longitudes use the
     * same western hemisphere convention as input values. Exceptions to this
     * logic happen in areas that span longitudes 0 or 180.
     * Areas that span longitude 0 should use negative values for west and
     * positive values for east. Areas that span longitude 180 should use
     * positive values for both east and west.
     *
     * @param  profile      Name of the NetCDF temperature file to load.
     * @param  date         Extract data for the time closest to
     *                      the specified date. Same units and scale as
     *                      used in the NetCDF file.
     * @param  south        Lower limit for the latitude axis (degrees).
     * @param  north        Upper limit for the latitude axis (degrees).
     * @param  west         Lower limit for the longitude axis (degrees).
     * @param  east         Upper limit for the longitude axis (degrees).
     * @param  earth_radius Depth correction term (meters).
     *                      Set to zero if you want to make depths
     *                      relative to earth's surface.
     */
    netcdf_profile(
        const char* profile, double date,
        double south, double north, double west, double east,
        double earth_radius=wposition::earth_radius ) ;

    /**
     * Fill missing values with average data at each depth.
     * This is designed to smooth out sharp changes in the
     * near-bottom profile that do not correspond to
     * physical phenomena.
     */
    void fill_missing() ;

  private:

    /**
     * Deduces the variables to be loaded based on their dimensionality.
     * The first variable to have 4 dimensions is assumed to be the
     * profile value.  The 4 dimensions of the profile are assumed to be
     * (in order) time, altitude, latitude, and longitude in compliance
     * with the COOARDS standards.
     *
     * @param  file         Reference to an open NetCDF file.
     * @param  missing      Value used to indicate missing data.
     * @param  time         NetCDF variable for time of year (output).
     * @param  altitude     NetCDF variable for altitude (output).
     * @param  latitude     NetCDF variable for latitude (output).
     * @param  longitude    NetCDF variable for longitude (output).
     * @param  value        NetCDF variable for datafile value (output).
     */
    void decode_filetype(
        NcFile& file, double *missing, NcVar **time, NcVar **altitude,
        NcVar **latitude, NcVar **longitude, NcVar **value ) ;
} ;

/// @}
}  // end of namespace netcdf
}  // end of namespace usml

#endif
