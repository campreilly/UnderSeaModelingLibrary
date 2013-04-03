/**
 * @file netcdf_woa.h
 * Extracts ocean profile data from World Ocean Atlas.
 */
#ifndef USML_NETCDF_WOA_H
#define USML_NETCDF_WOA_H

#include <usml/netcdf/netcdf_profile.h>

namespace usml {
namespace netcdf {

/// @ingroup netcdf_files
/// @{

/**
 * Extracts ocean profile data from World Ocean Atlas (WOA).
 * The World Ocean Atlas defines worldwide physical characteristics, like
 * temperature and salinity for monthly, seasonal, and annual averages.
 * All of these products uses a standard set of depths:
 *
 * - Depth: 0, 10, 20, 30, 50, 75, 100, 125, 150, 200, 250, 300,
 *   400, 500, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500,
 *   1750, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500 meters.
 *
 * This class is a specialization of the netcdf_profile class that allows
 * monthly average to be constructed over the entire range from 0 to 5500 by
 * splicing shallow monthly averages into either the deep seaonal average or
 * the deep yearly average. Seaonal average are recommended over yearly
 * averages for this splicing process.
 *
 * WOA dates are given in terms of days since the start of the year for the
 * middle of each month.  The date closest to the desired date is
 * considered the best match.  In WOA09, the exact dates used for each
 * of the products are:
 *
 * - Monthly: 15, 46, 75, 106, 136, 167, 197, 228, 259, 289, 320, 350 days.
 * - Seaonsal: 46, 136, 228, 320 days.
 * - Yearly: 182 days.
 *
 * WOA latitude and longitudes are given relative to the center of the cell
 * in which they apply.  Latitudes are gridded at a 1 degree spacing from
 * -89.5 to 89.5 degrees.  Longitudes are gridded at a 1 degree spacing from
 * 0.5 to 359.5 degrees.
 *
 * Successfully tested using objectively analyzed climatologies for the
 * World Ocean Atlas 2009 and 2005 data sets. The WOA09 filenames for
 * the 1 degree version of each of these products take the form:
 *
 * - xxx_monthly_1deg.nc = Monthly averages for depths in the [0,1500] meter range.
 * - xxx_seasonal_1deg.nc = Seasonal averages for depths in the [0,5500] meter range.
 * - xxx_annual_1deg.nc = Yearly averages for depths in the [0,5500] meter range.
 *
 * where xxx = "salinity" or "temperature", and the analyzed climatologies
 * are the first 4-D grid (s_an or t_an). The WOA05 filenames for each of
 * the analyzed climatologies take the form:
 *
 * - x0112an1.nc = Monthly averages for depths in the [0,1500] meter range.
 * - x1316an1.nc = Seasonal averages for depths in the [0,5500] meter range.
 * - x00an1.nc = Yearly averages for depths in the [0,5500] meter range.
 *
 * where x = "s" for salinity or "t" for temperature.
 *
 * @xref National Oceanographic Data Center (NODC), "World Ocean
 * Atlas 2009 (WOA09) Product Documentation," National Oceanic and
 * Atmospheric Administration (NOAA), Silver Springs, MD, March 2010.
 * See http://www.nodc.noaa.gov/OC5/WOA09/pr_woa09.html for more information.
 *
 * @xref National Oceanographic Data Center (NODC), "World Ocean
 * Atlas 2005 (WOA05) Product Documentation," National Oceanic and
 * Atmospheric Administration (NOAA), Silver Springs, MD, September 2006.
 * See http://www.nodc.noaa.gov/OC5/WOA05/pubwoa05.html for more information.
 *
 * @xref Mark A. Collier, Paul J. Durack, "CSIRO netCDF version of the
 * NODC World Ocean Atlas 2005," Commonwealth Scientific and Industrial
 * Research Organization (CSIRO) Marine and Atmospheric Research Paper 015,
 * Australia, December 2006. Contact mark.collier@csiro.au for more information.
 */
class USML_DECLSPEC netcdf_woa : public netcdf_profile {

  public:

    /**
     * Load deep and shallow parts of WOA ocean profile from disk.
     * Uses netcdf_profile.fill_missing() to automatically replace
     * missing (NaN) values with average data at each depth.  The
     * interpolation features of the data_grid superclass are setup to use
     * linear interpolation for latitude and longitude, but PCHIP
     * interpolation for the depth axis.
     *
     * Western hemisphere longitude can be expressed either as negative
     * values or values above 180 degrees. Output longitudes use the
     * same western hemisphere convention as input values. Exceptions to this
     * logic happen in areas that span longitudes 0 or 180.
     * Areas that span longitude 0 should use negative values for west and
     * positive values for east. Areas that span longitude 180 should use
     * positive values for both east and west.
     *
     * @param  deep         Name of the WOA file to use below 1500m.
     * @param  shallow      Name of the WOA file to use above (or at) 1500m.
     * @param  month        Month of the year (1=January, 12=December).
     * @param  south        Lower limit for the latitude axis (degrees).
     * @param  north        Upper limit for the latitude axis (degrees).
     * @param  west         Lower limit for the longitude axis (degrees).
     * @param  east         Upper limit for the longitude axis (degrees).
     * @param  earth_radius Depth correction term (meters).
     *                      Set to zero if you want to avoid transforming
     *                      profile into spherical earth coordinates.
     */
    netcdf_woa(
        const char* deep, const char* shallow, int month,
        double south, double north, double west, double east,
        double earth_radius=wposition::earth_radius ) ;

} ;

/// @}
}  // end of namespace netcdf
}  // end of namespace usml

#endif
