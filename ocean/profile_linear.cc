/** 
 * @file profile_linear.cc
 * Creates an analytic model for the Bi-Linear, Linear or Constant profile.
 */
 
#include <usml/ocean/profile_linear.h>

using namespace usml::ocean ;
          
/**
 * Compute the speed of sound and it's first derivatives at
 * a series of locations.
 */
void profile_linear::sound_speed( const wposition& location, 
    matrix<double>* speed, wvector* gradient
) {
    if (gradient) gradient->clear() ;
    
    for ( size_t r=0 ; r < location.size1() ; ++r ) {
        for ( size_t c=0 ; c < location.size2() ; ++c ) {
            double z = -location.altitude(r,c) ;
            if ( z < _depth1 ) {
                (*speed)(r,c) = _soundspeed0 
                              + _gradient0 * z ;
                if (gradient) gradient->rho(r,c,-_gradient0) ;
            } else {
                (*speed)(r,c) = _soundspeed0 
                              + _gradient0 * _depth1 
                              + _gradient1 * (z-_depth1);
                if (gradient) gradient->rho(r,c,-_gradient1) ;
            }
        }
    }
    
    adjust_speed( location, speed, gradient ) ;
}
