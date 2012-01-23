/** 
 * @file reflection_model.cc
 * Reflection model components of wave_queue object type.
 */
#include <usml/wave_q3d/reflection_model.h>
#include <usml/wave_q3d/ode_integ.h>

using namespace usml::wave_q3d ;

/**
 * Reflect a single acoustic ray from the ocean bottom.  
 */
bool reflection_model::bottom_reflection( unsigned de, unsigned az ) {
    double N ;
    
    // extract position, direction, and sound speed from this ray
    // at a point just before it goes below the bottom
    
    wposition1 position( _wave._curr->position, de, az ) ;
    wvector1 ndirection( _wave._curr->ndirection, de, az ) ;
    double c = _wave._curr->sound_speed( de, az ) ;
   
    // extract height above boundary and 
    // bathymetry slope at this point from the ocean bottom model
    
    double height ;
    wvector1 normal ;
    boundary_model& boundary = _wave._ocean.bottom() ;
    boundary.height( position, &height, &normal ) ;

    // make normal horizontal for very shallow water
    // to avoid propagating onto land
    
    if ( (height-wposition::earth_radius) > TOO_SHALLOW ) {
    	N = normal.theta()*normal.theta() + normal.phi()*normal.phi() ;
        normal.rho( 0.0 ) ;
        normal.theta( normal.theta() / N ) ;
        normal.phi(   normal.phi() / N  ) ;
    }
    
    // convert normalized direction to dr/dt in rectangular coordinates
    // relative to the point of collision
       
    double c2 = c*c ;
    ndirection.rho(   c2 * ndirection.rho() ) ;
    ndirection.theta( c2 * ndirection.theta() ) ;
    ndirection.phi(   c2 * ndirection.phi() ) ;

    // compute fraction of time step needed to strike the point of collision
    
    double dot = normal.rho() * ndirection.rho()
               + normal.theta() * ndirection.theta()
               + normal.phi() * ndirection.phi() ;
    const double dtime = ( dot == 0.0 ) ? 0.0 
        : ( height - position.rho() ) * normal.rho() / dot ;
                 
    // compute the precise values for position, direction, 
    // sound speed, and grazing angle at the point of collision
    
    collision_location( de, az, dtime, &position, &ndirection, &c ) ;
    c2 = c*c ;
    ndirection.rho(   c2 * ndirection.rho() ) ;
    ndirection.theta( c2 * ndirection.theta() ) ;
    ndirection.phi(   c2 * ndirection.phi() ) ;
    double ray_de, ray_az ;
    ndirection.direction( &ray_de, &ray_az ) ;
    
    dot = normal.rho() * ndirection.rho()
        + normal.theta() * ndirection.theta()
        + normal.phi() * ndirection.phi() ;
    N = sqrt( ndirection.rho() * ndirection.rho()
            + ndirection.theta() * ndirection.theta()
            + ndirection.phi() * ndirection.phi() ) ;
    double angle = asin( dot / -N ) ;
    if ( angle <= 0.0 ) return false ;	// near miss of the bottom

    // invoke bottom reverberation callback
    // THIS IS A STUB FOR FUTURE BEHAVIORS.

    if ( _bottom_reverb ) {
        _bottom_reverb->collision(  de, az, _wave._time+dtime,
            position,  ndirection, c, *(_wave._frequencies),
            _wave._curr->attenuation(de,az), _wave._curr->phase(de,az) ) ;
            // Still need to calculate eigenray ampltiude and phase for
            // reverberation callback. Just passing bogus values currently.
    }

    // compute reflection loss
    // adds reflection attenuation and phase to existing value
    
    vector<double> amplitude( _wave._frequencies->size() ) ;
    vector<double> phase( _wave._frequencies->size() ) ;
    boundary.reflect_loss( 
        position, *(_wave._frequencies), angle, &amplitude, &phase ) ;
    for ( unsigned f=0 ; f < _wave._frequencies->size() ; ++f ) {
        _wave._next->attenuation(de,az)(f) += amplitude(f) ;
        _wave._next->phase(de,az)(f) += phase(f) ; 
    }

    // change direction of the ray ( R = I - 2 dot(n,I) n )
    // and reinit past, prev, curr, next entries

    dot *= 2.0 ;
    ndirection.rho(   ndirection.rho()   - dot * normal.rho() ) ;
    ndirection.theta( ndirection.theta() - dot * normal.theta() ) ;
    ndirection.phi(   ndirection.phi()   - dot * normal.phi() ) ;

    N = sqrt( ndirection.rho() * ndirection.rho()
            + ndirection.theta() * ndirection.theta()
            + ndirection.phi() * ndirection.phi() )
            * c ;
    
    ndirection.rho(   ndirection.rho() / N ) ;
    ndirection.theta( ndirection.theta() / N ) ;
    ndirection.phi(   ndirection.phi() / N ) ;

    reflection_reinit( de, az, dtime, position, ndirection, c ) ;
    ndirection.direction( &ray_de, &ray_az ) ;
    return true ;
}

/**
 * Reflect a single acoustic ray from the ocean surface.  
 */
bool reflection_model::surface_reflection( unsigned de, unsigned az ) {
    boundary_model& boundary = _wave._ocean.surface() ;

    // compute fraction of time step needed to strike the point of collision
    
    double c = _wave._curr->sound_speed(de,az) ;
    const double d = c*c * _wave._curr->ndirection.rho(de,az) ;
    double dtime = (d==0.0) ? 0.0 
        : - _wave._curr->position.altitude(de,az) / d ;

    // compute the precise values for position, direction, 
    // sound speed, and grazing angle at the point of collision
    
    wposition1 position ;
    wvector1 ndirection ;
    collision_location( de, az, dtime, &position, &ndirection, &c ) ;
    double angle = atan2( _wave._curr->ndirection.rho(de,az), sqrt(
        _wave._curr->ndirection.theta(de,az) * 
        _wave._curr->ndirection.theta(de,az) +
        _wave._curr->ndirection.phi(de,az) * 
        _wave._curr->ndirection.phi(de,az)
    ) ) ;
    if ( angle <= 0.0 ) return false ;	// near miss of the surface

    // invoke bottom reverberation callback
    // THIS IS A STUB FOR FUTURE BEHAVIORS.

    if ( _surface_reverb ) {
        _surface_reverb->collision(  de, az, _wave._time+dtime,
            position,  ndirection, c, *(_wave._frequencies),
            _wave._curr->attenuation(de,az), _wave._curr->phase(de,az) ) ;
            // Still need to calculate eigenray ampltiude and phase for
            // reverberation callback. Just passing bogus values currently.
    }
        
    // compute reflection loss
    // adds reflection attenuation and phase to existing value
    
    vector<double> amplitude( _wave._frequencies->size() ) ;
    boundary.reflect_loss( 
        position, *(_wave._frequencies), angle, &amplitude ) ;
    for ( unsigned f=0 ; f < _wave._frequencies->size() ; ++f ) {
        _wave._next->attenuation(de,az)(f) += amplitude(f) ;
        _wave._next->phase(de,az)(f) -= M_PI ;
    }
    
    // change direction of the ray ( Rz = -Iz )
    // and reinit past, prev, curr, next entries

    ndirection.rho( -ndirection.rho() ) ;
    reflection_reinit(de, az, dtime, position, ndirection, c ) ;
    return true ;
}

/**
 * Compute the precise location and direction at the point of collision.
 */
void reflection_model::collision_location(
    unsigned de, unsigned az, double dtime,
    wposition1* position, wvector1* ndirection, double* speed ) const
{
    double drho, dtheta, dphi, d2rho, d2theta, d2phi ;
    const double time1 = 2.0 * _wave._time_step ;
    const double time2 = _wave._time_step * _wave._time_step ;
    const double dtime2 = dtime * dtime ;

    // second order Taylor series for sound speed

    drho = ( _wave._next->sound_speed(de,az)
        - _wave._prev->sound_speed(de,az) )
        / time1 ;

    d2rho = ( _wave._next->sound_speed(de,az)
        + _wave._prev->sound_speed(de,az)
        - 2.0 * _wave._curr->sound_speed(de,az) )
        / time2 ;

    *speed = _wave._curr->sound_speed(de,az)
        + drho * dtime + 0.5 * d2rho * dtime2 ;

    // second order Taylor series for position

    drho = ( _wave._next->position.rho(de,az)
        - _wave._prev->position.rho(de,az) )
        / time1 ;
    dtheta = ( _wave._next->position.theta(de,az)
        - _wave._prev->position.theta(de,az) )
        / time1 ;
    dphi = ( _wave._next->position.phi(de,az)
        - _wave._prev->position.phi(de,az) )
        / time1 ;

    d2rho = ( _wave._next->position.rho(de,az)
        + _wave._prev->position.rho(de,az)
        - 2.0 * _wave._curr->position.rho(de,az) )
        / time2 ;
    d2theta = ( _wave._next->position.theta(de,az)
        + _wave._prev->position.theta(de,az)
        - 2.0 * _wave._curr->position.theta(de,az) )
        / time2 ;
    d2phi = ( _wave._next->position.phi(de,az)
        + _wave._prev->position.phi(de,az)
        - 2.0 * _wave._curr->position.phi(de,az) )
        / time2 ;

    position->rho( _wave._curr->position.rho(de,az)
        + drho * dtime + 0.5 * d2rho * dtime2 ) ;
    position->theta( _wave._curr->position.theta(de,az)
        + dtheta * dtime + 0.5 * d2theta * dtime2 ) ;
    position->phi( _wave._curr->position.phi(de,az)
        + dphi * dtime + 0.5 * d2phi * dtime2 ) ;

    // second order Taylor series for ndirection

    drho = ( _wave._next->ndirection.rho(de,az)
        - _wave._prev->ndirection.rho(de,az) )
        / time1 ;
    dtheta = ( _wave._next->ndirection.theta(de,az)
        - _wave._prev->ndirection.theta(de,az) )
        / time1 ;
    dphi = ( _wave._next->ndirection.phi(de,az)
        - _wave._prev->ndirection.phi(de,az) )
        / time1 ;

    d2rho = ( _wave._next->ndirection.rho(de,az)
        + _wave._prev->ndirection.rho(de,az)
        - 2.0 * _wave._curr->ndirection.rho(de,az) )
        / time2 ;
    d2theta = ( _wave._next->ndirection.theta(de,az)
        + _wave._prev->ndirection.theta(de,az)
        - 2.0 * _wave._curr->ndirection.theta(de,az) )
        / time2 ;
    d2phi = ( _wave._next->ndirection.phi(de,az)
        + _wave._prev->ndirection.phi(de,az)
        - 2.0 * _wave._curr->ndirection.phi(de,az) )
        / time2 ;

    ndirection->rho( _wave._curr->ndirection.rho(de,az)
        + drho * dtime + 0.5 * d2rho * dtime2 ) ;
    ndirection->theta( _wave._curr->ndirection.theta(de,az)
        + dtheta * dtime + 0.5 * d2theta * dtime2 ) ;
    ndirection->phi( _wave._curr->ndirection.phi(de,az)
        + dphi * dtime + 0.5 * d2phi * dtime2 ) ;
}

/**
 * Re-initialize an individual ray after reflection.
 */
void reflection_model::reflection_reinit( 
    unsigned de, unsigned az, double dtime, 
    const wposition1& position, const wvector1& ndirection, double speed )
{
    // create temporary 1x1 wavefront elements
    
    wave_front past( _wave._ocean, _wave._frequencies, 1, 1 ) ;
    wave_front prev( _wave._ocean, _wave._frequencies, 1, 1 ) ;
    wave_front curr( _wave._ocean, _wave._frequencies, 1, 1 ) ;
    wave_front next( _wave._ocean, _wave._frequencies, 1, 1 ) ;
      
    // initialize current entry with reflected position and direction
    // adapted from wave_front::init_wave()
    
    curr.position.rho(   0, 0, position.rho() ) ;
    curr.position.theta( 0, 0, position.theta() ) ;
    curr.position.phi(   0, 0, position.phi() ) ;
    
    curr.ndirection.rho(   0, 0, ndirection.rho()  ) ;
    curr.ndirection.theta( 0, 0, ndirection.theta() ) ;
    curr.ndirection.phi(   0, 0, ndirection.phi() ) ;

    curr.update() ;
    
    // Runge-Kutta to initialize current entry "dtime" seconds in the past
    // adapted from wave_queue::init_wavefronts() 
    
    ode_integ::rk1_pos(  - dtime, &curr, &next ) ;
    ode_integ::rk1_ndir( - dtime, &curr, &next ) ;
    next.update() ;
    
    ode_integ::rk2_pos(  - dtime, &curr, &next, &past ) ;
    ode_integ::rk2_ndir( - dtime, &curr, &next, &past ) ;
    past.update() ;
    
    ode_integ::rk3_pos(  - dtime, &curr, &next, &past, &curr, false ) ;
    ode_integ::rk3_ndir( - dtime, &curr, &next, &past, &curr, false ) ;
    curr.update() ;
    reflection_copy( _wave._curr, de, az, curr ) ;
    
    // Runge-Kutta to estimate prev wavefront from curr entry
    // adapted from wave_queue::init_wavefronts() 
    
    double time_step = _wave._time_step ;
    ode_integ::rk1_pos(  - time_step, &curr, &next ) ;
    ode_integ::rk1_ndir( - time_step, &curr, &next ) ;
    next.update() ;
    
    ode_integ::rk2_pos(  - time_step, &curr, &next, &past ) ;
    ode_integ::rk2_ndir( - time_step, &curr, &next, &past ) ;
    past.update() ;
    
    ode_integ::rk3_pos(  - time_step, &curr, &next, &past, &prev ) ;
    ode_integ::rk3_ndir( - time_step, &curr, &next, &past, &prev ) ;
    prev.update() ;
    reflection_copy( _wave._prev, de, az, prev ) ;

    // Runge-Kutta to estimate past wavefront from prev entry
    // adapted from wave_queue::init_wavefronts() 
    
    ode_integ::rk1_pos(  - time_step, &prev, &next ) ;
    ode_integ::rk1_ndir( - time_step, &prev, &next ) ;
    next.update() ;
    
    ode_integ::rk2_pos(  - time_step, &prev, &next, &past ) ;
    ode_integ::rk2_ndir( - time_step, &prev, &next, &past ) ;
    past.update() ;
    
    ode_integ::rk3_pos(  - time_step, &prev, &next, &past, &past, false ) ;
    ode_integ::rk3_ndir( - time_step, &prev, &next, &past, &past, false ) ;
    past.update() ;
    reflection_copy( _wave._past, de, az, past ) ;

    // Adams-Bashforth to estimate next wavefront 
    // from past, prev, and curr entries
    // adapted from wave_queue::init_wavefronts() 
    
    ode_integ::ab3_pos(  time_step, &past, &prev, &curr, &next ) ;
    ode_integ::ab3_ndir( time_step, &past, &prev, &curr, &next ) ;
    next.update() ;
    
    reflection_copy( _wave._next, de, az, next ) ;
}

/**
 * Copy new wave element data into the destination wavefront.
 */
void reflection_model::reflection_copy( 
    wave_front* element, unsigned de, unsigned az, 
    wave_front& results )
{
    element->position.rho(   de, az, results.position.rho(0,0) ) ;
    element->position.theta( de, az, results.position.theta(0,0) ) ;
    element->position.phi(   de, az, results.position.phi(0,0) ) ;
    
    element->pos_gradient.rho(  de, az, results.pos_gradient.rho(0,0) ) ;
    element->pos_gradient.theta(de, az, results.pos_gradient.theta(0,0) ) ;
    element->pos_gradient.phi(  de, az, results.pos_gradient.phi(0,0) ) ;
    
    element->ndirection.rho(   de, az, results.ndirection.rho(0,0) ) ;
    element->ndirection.theta( de, az, results.ndirection.theta(0,0) ) ;
    element->ndirection.phi(   de, az, results.ndirection.phi(0,0) ) ;

    element->ndir_gradient.rho(  de, az, results.ndir_gradient.rho(0,0));
    element->ndir_gradient.theta(de, az, results.ndir_gradient.theta(0,0));
    element->ndir_gradient.phi(  de, az, results.ndir_gradient.phi(0,0));

    element->sound_gradient.rho(  de, az, results.sound_gradient.rho(0,0));
    element->sound_gradient.theta(de, az, results.sound_gradient.theta(0,0));
    element->sound_gradient.phi(  de, az, results.sound_gradient.phi(0,0));

    element->sound_speed( de, az ) = results.sound_speed(0,0) ;
    element->distance( de, az ) = results.distance(0,0) ;
}
