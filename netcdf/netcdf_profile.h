/**
 * @file netcdf_profile.h
 * Extracts ocean profile data from world-wide databases.
 */
#pragma once

#include <stddef.h>
#include <usml/types/gen_grid.h>
#include <usml/usml_config.h>

#include <netcdf>

namespace usml {
namespace netcdf {

using namespace usml::ublas;
using namespace usml::types;

/// @ingroup netcdf_files
/// @{

/**
 * Extracts ocean profile data from world-wide databases.
 * These profiles can be a physical characteristics like temperature
 * and salinity, or an acoustic property like sound speed.
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
class USML_DECLSPEC netcdf_profile : public gen_grid<3> {
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
     * @param  filename     Name of the NetCDF temperature file to load.
     * @param  date         Extract data for the time closest to
     *                      the specified date. Same units and scale as
     *                      used in the NetCDF file.
     * @param  south        Lower limit for the latitude axis (degrees).
     * @param  north        Upper limit for the latitude axis (degrees).
     * @param  west         Lower limit for the longitude axis (degrees).
     * @param  east         Upper limit for the longitude axis (degrees).
     * @param  varname      Variable name to search for, or nullptr if name
     *                      can be anything. Uses boost::algorithm::icontains()
     *                      to find provided string in variable name.
     * @throws std:invalid_argument on invalid name or path of data file.
     */
    netcdf_profile(const char* filename, double date, double south,
                   double north, double west, double east,
                   const char* varname = nullptr);

    /**
     * Fill missing values with average data at each depth.
     * This is designed to smooth out sharp changes in the
     * near-bottom profile that do not correspond to
     * physical phenomena.
     *
     * The algorithm builds up NAN replacements as the sum of the points around
     * them in latitude and longitude, weighted by the fourth power of the
     * distance. For the first depth, it computes the weighted average of the
     * actual data values. For the other depths, it computes the weighted
     * average of the depth gradients. Each replacement is set to the point
     * above it plus the difference computed from the computed gradient.
     *
     * Beyond the point where any latitude or longitude has valid data, the
     * weighted average of the depth gradients can not be computed. At these
     * depths, the algorithm assumes that the gradient at each latitude and
     * longitude smoothly tapers to zero. The gradient at each depth is taken to
     * be half of the gradient above it. This generally a better approximation
     * for ocean temperature and salinity than it is for sound speed.
     */
    void fill_missing();

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
     * @param  scale        Value used to scale packed values.
     * @param  offset       Value used to offset packed values.
     * @param  time         NetCDF variable for time of year (output).
     * @param  altitude     NetCDF variable for altitude (output).
     * @param  latitude     NetCDF variable for latitude (output).
     * @param  longitude    NetCDF variable for longitude (output).
     * @param  value        NetCDF variable for datafile value (output).
     * @param  varname      Variable name to search for, or nullptr if name
     *                      can be anything.  Uses strstr() to find provided
     *                      string in variable name.
     */
    static void decode_filetype(netCDF::NcFile& file, double& missing,
                                double& scale, double& offset,
                                netCDF::NcVar& time, netCDF::NcVar& altitude,
                                netCDF::NcVar& latitude,
                                netCDF::NcVar& longitude,
                                netCDF::NcVar& profile, const char* varname);
};

/// @}
}  // end of namespace netcdf
}  // end of namespace usml
