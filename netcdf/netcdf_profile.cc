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

    // read latitude axis data from NetCDF variable
    // lat_first and lat_last are the integer offsets along this axis
    // _axis[1] is expressed as co-latitude in radians [0,PI]

    double a = latitude->as_double(0) ;
    int n = latitude->num_vals() - 1 ;
    double inc = ( latitude->as_double(n) - a ) / n ;
    const int lat_first = (int) floor( 1e-6 + (south-a) / inc ) ;
    const int lat_last = (int) floor( 0.5 + (north-a) / inc ) ;
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
    int lng_first = (int) floor( 1e-6 + (west-a) / inc ) ;
    if (lng_first < 0) {  // Prevent request for data outside
        lng_first = 0;    // of the data provided, on the left side.
    }
    const int lng_last = (int) floor( 0.5 + (east-a) / inc ) ;
    const int lng_num = lng_last - lng_first + 1 ;
    this->_axis[2] = new seq_linear(
        to_radians(lng_first*inc+a-offset),
        to_radians(inc),
        lng_num ) ;

    // check to see if database has duplicate data at cut point

    int duplicate = 0 ;
    if ( abs(longitude->as_double(0)+360-longitude->as_double(n)) < 1e-4 ) duplicate = 1 ;

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

	typedef unsigned int		 			size_type ;
	typedef double				 			value_type ;
	typedef data_grid<double,3>  			grid_type ;
	typedef matrix<value_type>				matrix_type ;
	typedef ublas::vector<matrix_type>		vector_type ;
	typedef zero_matrix<value_type>			matrix_z ;
	typedef scalar_matrix<value_type>		matrix_s ;

	seq_vector* ax[3] ;
	ax[0] = this->_axis[0] ;
	ax[1] = this->_axis[1] ;
	ax[2] = this->_axis[2] ;
	grid_type replace( ax ) ;
	seq_vector* depth = this->_axis[0] ;
	size_type ndepth = depth->size() ;
	size_type nlat = this->_axis[1]->size() ;
	size_type nlon = this->_axis[2]->size() ;

	vector_type profile_grad( ndepth ) ;
	vector_type replace_grad( ndepth ) ;
	for(size_type d=0; d<ndepth; ++d) {
		if( d==0 ) replace_grad(d) = matrix_s( nlat, nlon, NAN ) ;
		replace_grad(d) = matrix_z( nlat, nlon ) ;
		profile_grad(d) = matrix_s( nlat, nlon, NAN ) ;
		if( d > 0 ) {
			for(size_type j=0; j<nlat; ++j) {
				for(size_type k=0; k<nlon; ++k) {
					size_type indx[] = { d, j, k } ;
					value_type curr = this->data( indx ) ;
					if( ! std::isnan(curr) ) {
						size_type indx2[] = { d-1, j, k } ;
						value_type prev = this->data( indx2 ) ;
						profile_grad(d)(j,k) = ( curr - prev ) / depth->increment(d-1) ;
					}
				}
			}
		}
	}

	int Q = 4 ;
	for(size_type d=0; d<ndepth; ++d) {
		for(size_type j=0; j<nlat; ++j) {
			for(size_type k=0; k<nlon; ++k) {
				size_type index[] = { d, j, k } ;
				value_type r = this->data( index );
				if( ! std::isnan(r) ) {
					replace.data( index, r ) ;
					replace_grad(d)(j,k) = profile_grad(d)(j,k) ;
				} else {
					// compute weight
					value_type weight = 0.0 ;
					for(size_type n=0; n<nlat; ++n) {
						for(size_type m=0; m<nlon; ++m) {
							size_type index2[] = { d, n, m } ;
//							index2[0] = d ;
//							index2[1] = n ;
//							index2[2] = m ;
							value_type tmp = this->data(index2) ;
							if( ! std::isnan(tmp) ) {
								value_type t = (j-n)*(j-n) + (k-m)*(k-m) ;
								t = pow( t, Q ) ;
								value_type dist2 = 1.0 / t ;
								weight += dist2 ;
								if( d==0 ) {
									r = replace.data(index) + dist2 * tmp ;
									replace.data( index, r ) ;
								} else {
									replace_grad(d)(j,k) += dist2 * profile_grad(d)(n,m) ;
								}	// end if( d==0 )
							}	// end if( !isnan(tmp) )
						}	// end for m<nlon
					}	// end for n<nlat
					// apply weighted sum
					if( weight > 0.0 ) {
						if( d==0 ) {
							r = replace.data(index) / weight ;
							replace.data( index, r ) ;
						} else {
							replace_grad(d)(j,k) /= weight ;
							size_type index2[3] ;
							index2[0] = d-1 ;
							index2[1] = j ;
							index2[2] = k ;
							r = replace.data(index2) + replace_grad(d)(j,k) * depth->increment(d-1) ;
							replace.data( index, r ) ;
						}
					}	// end if( weight > 0.0 )
				}	// end of if( ! isnan )
			}	// end for k<nlon
		}	// end for j<nlat
	}	// end for d<ndepth
	this->copy(replace) ;
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
