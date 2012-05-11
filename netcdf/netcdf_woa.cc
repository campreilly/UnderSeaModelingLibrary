/** 
 * @file netcdf_woa.cc
 * Extracts ocean profile data from world-wide databases.
 */
#include <usml/netcdf/netcdf_woa.h>

using namespace usml::netcdf ;

/**
 * Load deep and shallow parts of WOA ocean profile from disk. 
 */
netcdf_woa::netcdf_woa( 
    const char* deep, const char* shallow, int month,
    double south, double north, double west, double east,
    double earth_radius ) 
    : netcdf_profile(deep,(int)round(30.5*(month-1)),south,north,west,east,earth_radius)
{
    // replace beginning of deep data with shallow values
    
    if ( shallow ) {
        netcdf_woa replace( shallow, NULL, month,
            south, north, west, east, earth_radius ) ;
            // work around protected nature of _data and _axis by using
            // netcdf_woa for "replace" instead of netcdf_profile.
        memcpy( this->_data, replace._data, 
                sizeof(double) *
                replace.axis(0)->size() * 
                replace.axis(1)->size() * 
                replace.axis(2)->size() ) ;
        fill_missing() ;
        interp_type(0,GRID_INTERP_PCHIP);// increase smoothness of depth interp
    }
}
