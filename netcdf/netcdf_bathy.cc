/** 
 * @file netcdf_bathy.cc
 * Extracts bathymetry data from world-wide bathymetry databases.
 */
#include <usml/netcdf/netcdf_bathy.h>

using namespace usml::netcdf ;

/**
 * Load bathymetry from disk.
 */
netcdf_bathy::netcdf_bathy( 
    const char* filename, 
    double south, double north, double west, double east,
    double earth_radius )
{
    // initialize access to NetCDF file.

    NcFile file( filename ) ;
    NcVar *longitude, *latitude, *altitude ;
    decode_filetype( file, &latitude, &longitude, &altitude ) ;
    
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
    cout << " west=" << west << " east=" << east << endl ;
        
    // read latitude axis data from NetCDF file.
    // lat_first and lat_last are the integer offsets along this axis
    // _axis[0] is expressed as co-latitude in radians [0,PI]

    double a = latitude->as_double(0) ;
    int n = latitude->num_vals() - 1 ;
    double inc = ( latitude->as_double(n) - a ) / n ;
    const int lat_first = (int) floor( 1e-6 + (south-a) / inc ) ;
    const int lat_last = (int) floor( 0.5 + (north-a) / inc ) ;
    const int lat_num = lat_last - lat_first + 1 ;
    this->_axis[0] = new seq_linear( 
        to_colatitude(lat_first*inc+a),
        to_radians(-inc), 
        lat_num );

    // read longitude axis data from NetCDF file
    // lng_first and lng_last are the integer offsets along this axis
    // _axis[1] is expressed as longtitude in radians [-PI,2*PI]

    a = longitude->as_double(0) ;
    n = longitude->num_vals() - 1 ;
    inc = ( longitude->as_double(n) - a ) / n ;
    const int lng_first = (int) floor( 1e-6 + (west-a) / inc ) ;
    const int lng_last = (int) floor( 0.5 + (east-a) / inc ) ;
    const int lng_num = lng_last - lng_first + 1 ;
    this->_axis[1] = new seq_linear( 
        to_radians(lng_first*inc+a-offset), 
        to_radians(inc),
        lng_num ) ;
     cout << " a=" << a << " n=" << n << " inc=" << inc << endl ;
     cout << " lng_first=" << lng_first << " lng_last=" << lng_last << " lng_num=" << lng_num << endl ;

    // check to see if database has duplicate data at cut point

    int duplicate = 0 ;
    if ( abs(longitude->as_double(0)+360-longitude->as_double(n)) < 1e-4 ) duplicate = 1 ;

    // load depth data out of NetCDF file
    
    this->_data = new float[ lat_num * lng_num ] ;
    if ( longitude->num_vals() > lng_last ) {
        altitude->set_cur( lat_first, lng_first ) ;
        altitude->get( this->_data, lat_num, lng_num ) ;

    // support datasets that cross the unwrapping longitude
    // assumes that bathy data is repeated on both sides of cut point

    } else {
        int M = lng_last - longitude->num_vals() + 1 ;  // # pts on east side
        int N = lng_num - M ;                           // # pts on west side
        float* ptr = this->_data ;
        cout << " N=" << N << " M=" << M << endl ;
        for ( int lat = lat_first ; lat <= lat_last ; ++lat ) {

            // the west side of the block is the portion from
            // lng_first to the last latitude
            altitude->set_cur( lat, lng_first ) ;
            altitude->get( ptr, 1, N ) ;
            ptr += N ;

            // the missing points on the east side of the block
            // are read from zero until the right # of points are read
            // skip first longitude if it is a duplicate
            altitude->set_cur( lat, duplicate ) ;
            altitude->get( ptr, 1, M ) ;
            ptr += M ;
        }
    }
    
    // convert depth to rho coordinate of spherical earth system

    float* ptr = this->_data ;
    float R = (float) earth_radius ;
    while ( ptr < this->_data+(lat_num * lng_num) ) {
        *(ptr++) += R ;
    }
}
    
/**
 * Deduces the variables to be loaded based on their dimensionality.
 */
void netcdf_bathy::decode_filetype( 
    NcFile& file, NcVar **latitude, NcVar **longitude, NcVar **altitude )
{
    bool found = false ;
    for ( int n=0 ; n < file.num_vars() ; ++n ) {
        NcVar *var = file.get_var(n) ;
        cout << "var: " << var->name() << endl ;
        if ( var->num_dims() == 2 ) {
            // extract depth variable
            *altitude = var ;
            cout << "\taltitude: " << (*altitude)->name() << endl ;
            for ( int n=0 ; n < var->num_dims() ; ++n ) {
                NcDim* d = var->get_dim(n) ;
                cout << "\t\tdim(" << n << "): " << d->name() << endl ;
            }
            
            // extract latitude variable
            NcDim* dim = var->get_dim(0) ;
            *latitude = file.get_var( dim->name() ) ;  
            cout << "\tlatitude: " << (*latitude)->name() << endl ;
             
            // extract longitude variable
            dim = var->get_dim(1) ;
            *longitude = file.get_var( dim->name() ) ;   
            cout << "\tlongitude: " << (*longitude)->name() << endl ;

            // stop searching            
            found = true ;
            break ;
        }
    }
    if ( ! found ) {
        throw std::invalid_argument("unrecognized file type") ;
    }
}
