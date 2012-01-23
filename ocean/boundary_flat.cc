/** 
 * @file boundary_flat.cc
 * Models any flat boundary in the ocean including the ocean surface.
 */
 
#include <usml/ocean/boundary_flat.h>

using namespace usml::ocean ;
          
/**
 * Initialize component models within ocean profile.
 */
boundary_flat::boundary_flat(double depth, reflect_loss_model* reflect_loss) 
    : boundary_model(reflect_loss), _height(wposition::earth_radius-depth)
{
    if ( depth < 1e-6 ) {
        _normal_rho = -1.0 ;
        if ( _reflect_loss_model == NULL ) {
            _reflect_loss_model = new reflect_loss_constant( 0.0, M_PI ) ;
        }
    } else {
        _normal_rho = 1.0 ;
        if ( _reflect_loss_model == NULL ) {
            _reflect_loss_model = new reflect_loss_constant( 0.0, 0.0 ) ;
        }
    }
}

/**
 * Compute the height of the boundary and it's surface normal at
 * a series of locations.
 */
void boundary_flat::height( const wposition& location, 
    matrix<double>* rho, wvector* normal )
{
    *rho = scalar_matrix<double>( 
        location.size1(), location.size2(), _height );
    if ( normal ) {
        normal->rho( scalar_matrix<double>( 
            location.size1(), location.size2(), _normal_rho ) ) ;
        normal->theta( scalar_matrix<double>( 
            location.size1(), location.size2(), 0.0 ) ) ;
        normal->phi( normal->theta() ) ;
    }
}

/**
 * Compute the height of the boundary and it's surface normal at
 * a single location.  Often used during reflection processing.
 */
void boundary_flat::height( const wposition1& location, 
    double* rho, wvector1* normal ) 
{
    *rho = _height ;
    if ( normal ) {
        normal->rho( _normal_rho ) ;
        normal->theta( 0.0 ) ;
        normal->phi( 0.0 ) ;
    }
}
