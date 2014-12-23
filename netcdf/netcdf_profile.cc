/**
 * @file netcdf_profile.cc
 * Extracts ocean profile data from world-wide databases.
 */
#include <usml/netcdf/netcdf_profile.h>

using namespace usml::netcdf ;

/**
 * Load ocean profile from disk.
 */
netcdf_profile::netcdf_profile(
    const char* profile, double date,
    double south, double north, double west, double east,
    double earth_radius )
{
    // initialize access to NetCDF file.

    double missing = NAN ;   // default value for missing information
    NcVar *time, *altitude, *latitude, *longitude, *value ;
    NcFile pfile( profile ) ;
    if (pfile.is_valid() == 0) {
        throw std::invalid_argument("file not found") ;
    }
    decode_filetype( pfile, &missing, &time, &altitude,
                     &latitude, &longitude, &value ) ;

    // find the time closest to the specified value

    int time_index = 0 ;
    double old_diff = abs( date - time->as_double(0) ) ;
    for ( int t=1 ; t < time->num_vals() ; ++t ) {
        double diff = abs( date - time->as_double(t) ) ;
        if ( old_diff > diff ) {
            old_diff = diff ;
            time_index = t ;
        }
    }

    // read altitude axis data from NetCDF variable

    const int alt_num = (int) altitude->num_vals() ;
    vector<double> vect(alt_num) ;
    for ( int d=0 ; d < alt_num ; ++d ) {
        vect[d] = earth_radius - abs( altitude->as_double(d) ) ;
    }
    this->_axis[0] = new seq_data( vect ) ;

    // manage wrap-around between eastern and western hemispheres

    double offset = 0.0 ;
    int duplicate = 0 ;
    int n = longitude->num_vals() - 1 ;
    // Is the data set bounds 0 to 359(360)
    bool zero_to_360 = ( longitude->as_double(0) <= 1.0 &&
    					 longitude->as_double(n) >= 359.0 ) ? true : false ;
    // Is the data set bounds -180 to 179(180)
    bool bounds_180 = ( longitude->as_double(n) >= 179.0 &&
			   	   	  	longitude->as_double(0) == -180.0 ) ? true : false ;
    // Is this set a global data set
    bool global = ( zero_to_360 || bounds_180 ) ;
    if( global ) {
        // check to see if database has duplicate data at cut point
        if ( abs(longitude->as_double(0)+360-longitude->as_double(n)) < 1e-4 ) duplicate = 1 ;

        // manage wrap-around between eastern and western hemispheres
        if ( longitude->as_double(0) < 0.0 ) {
            // if database has a range (-180,180)
            // make western longitudes into negative numbers
            // unless they span the 180 latitude
            if ( west > 180.0 && east > 180.0 ) offset = -360.0 ;
        } else {
            // if database has a range (0,360)
            // make all western longitudes into positive numbers
            if ( west < 0.0 ) offset = 360.0 ;
        }
        west += offset ;
        east += offset ;
    }

    // read latitude axis data from NetCDF variable
    // lat_first and lat_last are the integer offsets along this axis
    // _axis[1] is expressed as co-latitude in radians [0,PI]

    double a = latitude->as_double(0) ;
    n = latitude->num_vals() - 1 ;
    double inc = ( latitude->as_double(n) - a ) / n ;
    const int lat_first = max( 0, (int) floor( 1e-6 + (south-a) / inc ) ) ;
    const int lat_last = min( n, (int) floor( 0.5 + (north-a) / inc ) ) ;
    const int lat_num = lat_last - lat_first + 1 ;
    this->_axis[1] = new seq_linear(
        to_colatitude(lat_first*inc+a),
        to_radians(-inc),
        lat_num );

    // read longitude axis data from NetCDF variable
    // lng_first and lng_last are the integer offsets along this axis
    // _axis[2] is expressed as longtitude in radians [-PI,2*PI]

    a = longitude->as_double(0) ;
    n = (int) longitude->num_vals() - 1 ;
    inc = ( longitude->as_double(n) - a ) / n ;
    int index = (int) floor( 1e-6 + (west-a) / inc ) ;
    const int lng_first = (global) ? index : max(0, index) ;
    index = (int) floor( 0.5 + (east-a) / inc ) ;
    const int lng_last = (global) ? index : min(n, index) ;
    const int lng_num = lng_last - lng_first + 1 ;
    this->_axis[2] = new seq_linear(
        to_radians(lng_first*inc+a-offset),
        to_radians(inc),
        lng_num ) ;

    // load profile data out of NetCDF variable

    this->_data = new double[ alt_num * lat_num * lng_num ] ;
    if ( longitude->num_vals() > lng_last ) {
        value->set_cur( time_index, 0, lat_first, lng_first ) ;
        value->get( this->_data, 1, alt_num, lat_num, lng_num ) ;

    // support datasets that cross the unwrapping longitude
    // assumes that bathy data is NOT repeated on both sides of cut point

    } else {
        int M = lng_last - longitude->num_vals() + 1 ;  // # pts on east side
        int N = lng_num - M ;                           // # pts on west side
        double* ptr = this->_data ;
        for ( int alt = 0 ; alt < alt_num ; ++alt ) {
            for ( int lat = lat_first ; lat <= lat_last ; ++lat ) {

                // the west side of the block is the portion from
                // lng_first to the last latitude
                value->set_cur( time_index, alt, lat, lng_first ) ;
                value->get( ptr, 1, 1, 1, N ) ;
                ptr += N ;

                // the missing points on the east side of the block
                // are read from zero until the right # of points are read
                // skip first longitude if it is a duplicate
                value->set_cur( time_index, alt, lat, duplicate ) ;
                value->get( ptr, 1, 1, 1, M ) ;
                ptr += M ;
            }
        }
    }

    // change missing values in the file to NaN in memory
    // don't execute if netCDF file didn't specify a "missing" value

    if ( ! isnan(missing) ) {
        const int N = alt_num * lat_num * lng_num ;
        double* ptr = this->_data ;
        while ( ptr < this->_data + N ) {
            if ( *ptr == missing ) *ptr = NAN ;
            ++ptr ;
        }
    }
}

/**
 * Fill missing values with average data at each depth.
 */
void netcdf_profile::fill_missing() {

    const int alt_num = this->_axis[0]->size() ;
    const int lat_num = this->_axis[1]->size() ;
    const int lng_num = this->_axis[2]->size() ;
    unsigned index[3] ;

    // compute average value at each depth

    data_grid<double,1> average( this->_axis ) ;
    data_grid<double,1> number( this->_axis ) ;

    for ( int alt = 0 ; alt < alt_num ; ++alt ) {
        index[0] = alt ;

        // sum non-NAN data from all lat/longs

        for ( int lat = 0 ; lat < lat_num ; ++lat ) {
            index[1] = lat ;
            for ( int lng = 0 ; lng < lng_num ; ++lng ) {
                index[2] = lng ;
                double value = data(index) ;
                if ( ! isnan(value) ) {
                    average.data( index, average.data(index)+value ) ;
                    number.data( index, number.data(index)+1.0 ) ;
                }
            }
        }

        // divide data sum by number of observations
        // use value from previous depth if all lat/longs are NAN

        if ( number.data(index) == 0.0 ) {
            if ( index[0] <= 0 ) {
                average.data( index, NAN ) ;
            } else {
                unsigned prev_index[1] ;
                prev_index[0] = index[0] - 1 ;
                average.data( index, average.data(prev_index) ) ;
            }
        } else {
            average.data( index, average.data(index) / number.data(index) ) ;
        }
    }

    // fill in missing values with average values

    for ( int alt = 0 ; alt < alt_num ; ++alt ) {
        index[0] = alt ;
        for ( int lat = 0 ; lat < lat_num ; ++lat ) {
            index[1] = lat ;
            for ( int lng = 0 ; lng < lng_num ; ++lng ) {
                index[2] = lng ;
                double value = data(index) ;
                if ( isnan(value) ) {
                    data( index, average.data(index) ) ;
                }
            }
        }
    }
}

/**
 * Deduces the variables to be loaded based on their dimensionality.
 */
void netcdf_profile::decode_filetype(
    NcFile& file, double *missing, NcVar **time,
    NcVar **altitude, NcVar **latitude, NcVar **longitude,
    NcVar **value )
{
    bool found = false ;
    for ( int n=0 ; n < file.num_vars() ; ++n ) {
        NcVar *var = file.get_var(n) ;
        if ( var->num_dims() == 4 ) {
            // extract profile variable
            *value = var ;

            // extract time variable
            NcDim* dim = var->get_dim(0) ;
            *time = file.get_var( dim->name() ) ;

            // extract altitude variable
            dim = var->get_dim(1) ;
            *altitude = file.get_var( dim->name() ) ;

            // extract latitude variable
            dim = var->get_dim(2) ;
            *latitude = file.get_var( dim->name() ) ;

            // extract longitude variable
            dim = var->get_dim(3) ;
            *longitude = file.get_var( dim->name() ) ;

            { // Handle ncdfiles with no diffault _FillValue
            NcError* ncdfError = new NcError(NcError::silent_nonfatal) ;
            // extract missing attribute
                NcAtt* att = var->get_att("_FillValue") ;
                if (att) {
                    NcValues* values = att->values() ;
                    *missing = values->as_double(0) ;
                    delete att ; delete values ;
                }
                delete ncdfError;
            }


            // stop searching
            found = true ;
            break ;
        }
    }
    if ( ! found ) {
        throw std::invalid_argument("unrecognized file type") ;
    }
}
