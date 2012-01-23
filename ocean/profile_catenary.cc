/** 
 * @file profile_catenary.cc
 * Creates an analytic model for a deep duct catenary profile.
 */
 
#include <usml/ocean/profile_catenary.h>

using namespace usml::ocean ;
          
/**
 * Compute the speed of sound and it's first derivatives at
 * a series of locations.
 */
void profile_catenary::sound_speed( const wposition& location, 
    matrix<double>* speed, wvector* gradient
) {
    if (gradient) gradient->clear() ;
    
    *speed = _soundspeed1 * cosh( 
        ( location.altitude() + _depth1 ) / (-_gradient1) ) ;
    if ( gradient ) {
        gradient->rho( - sinh( ( location.altitude() + _depth1 ) 
            / (-_gradient1) ) * (_soundspeed1/_gradient1) ) ;
    }
    
    adjust_speed( location, speed, gradient ) ;
}
