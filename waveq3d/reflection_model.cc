/**
 * @file reflection_model.cc
 * Reflection model components of wave_queue object type.
 */
#include <usml/waveq3d/reflection_model.h>
#include <usml/waveq3d/ode_integ.h>

//#define REFLECT_BOT_DEBUG
//#define REFLECT_SRF_DEBUG
//#define COLLISION_DEBUG

using namespace usml::waveq3d ;

const double reflection_model::MIN_REFLECT = 6.0 ;

/**
 * Reflect a single acoustic ray from the ocean bottom.
 */
bool reflection_model::bottom_reflection( unsigned de, unsigned az, double depth ) {
    #ifdef REFLECT_BOT_DEBUG
        cout << "***Entering reflection_model::bottom_reflection***" << endl;
    #endif
    double N ;
    bool shallow = false ;

    // extract position, direction, and sound speed from this ray
    // at a point just before it goes below the bottom

    wposition1 position( _wave._curr->position, de, az ) ;
    wvector1 ndirection( _wave._curr->ndirection, de, az ) ;
    double c = _wave._curr->sound_speed( de, az ) ;
    double c2 = c*c ;

    // extract radial height and slope at current location
    // height_water = initial ray height above the bottom (must be positive)

    double bottom_rho ;
    wvector1 bottom_normal ;
    boundary_model& boundary = _wave._ocean.bottom() ;
    boundary.height( position, &bottom_rho, &bottom_normal ) ;
    double height_water = position.rho() - bottom_rho ;
    if( (wposition::earth_radius-bottom_rho) < TOO_SHALLOW ) shallow = true ;

    // make bottom vertical for very shallow water
    // to avoid propagating onto land

    if ( shallow ) {
        #ifdef REFLECT_BOT_DEBUG
            cout << "WARNING: Ray is in too shallow of water, turning ray around!" << endl ;
        #endif
    	N = sqrt( bottom_normal.theta()*bottom_normal.theta()
    	  + bottom_normal.phi()*bottom_normal.phi() ) ;
        bottom_normal.rho( 0.0 ) ;
        bottom_normal.theta( bottom_normal.theta() / N ) ;
        bottom_normal.phi(   bottom_normal.phi() / N  ) ;
    }

    // compute dot_full = dot product of the full dr/dt with bottom_normal (negative #)
    // converts ndirection to dr/dt in rectangular coordinates relative to reflection point

    ndirection.rho(   c2 * ndirection.rho() ) ;
    ndirection.theta( c2 * ndirection.theta() ) ;
    ndirection.phi(   c2 * ndirection.phi() ) ;
    double dot_full = bottom_normal.rho() * ndirection.rho()
                    + bottom_normal.theta() * ndirection.theta()
                    + bottom_normal.phi() * ndirection.phi() ;

    // compute the smallest "dot_full" that could have led to this penetration depth
    // assume minimum depth change, along normal, of 1.0 meters

    double max_dot = - max( MIN_REFLECT, (height_water+depth)*bottom_normal.rho() ) ;
    if ( dot_full >= max_dot ) dot_full = max_dot ;

    // compute time_water = fraction of time step needed to strike the bottom
    // time step = ratio of in water dot product to full dot product
    // dot_water = component of "height_water" parallel to bottom normal (negative #)

    const double dot_water = -height_water * bottom_normal.rho() ;
    double time_water = max( 0.0, dot_water / dot_full ) ;

    #ifdef REFLECT_BOT_DEBUG
        cout << "\t===Information before calling bottom::COLLISION_LOCATION===" << endl;
        cout << "\t\tposition(rho,theta,phi): (" << position.rho() - wposition::earth_radius << ", "
                                            << position.theta() << ", "
                                            << position.phi() << ")"
                                            << endl;
        cout << "\t\tndirection(rho,theta,phi): (" << ndirection.rho() << ", "
                                            << ndirection.theta() << ", "
                                            << ndirection.phi() << ")"
                                            << endl;
        cout << "\t\tsound_speed: " << c << "\tc_squared: " << c2 << endl;
        cout << "\t\tbottom_rho: " << bottom_rho - wposition::earth_radius << endl;
        cout << "\t\tbottom_normal(rho,theta,phi): (" << bottom_normal.rho() << ", "
                                                    << bottom_normal.theta() << ", "
                                                    << bottom_normal.phi() << ")"
                                                    << endl;
        cout << "\t\theight_water: " << height_water << endl;
        cout << "\t\tdot_full: " << dot_full << "\tmax_dot: " << max_dot << endl;
        cout << "\t\tdot_water: " << dot_water << "\ttime_water: " << time_water << endl;
    #endif

    // compute the more precise values for position, direction,
    // sound speed, bottom height, bottom slope, and grazing angle at the point of collision.
    // reduces grazing angle errors in highly refractive environments.

    collision_location( de, az, time_water, &position, &ndirection, &c ) ;
    boundary.height( position, &bottom_rho, &bottom_normal ) ;
    if ( shallow ) {
        #ifdef REFLECT_BOT_DEBUG
            cout << "WARNING: Ray is in too shallow of water, turning ray around!" << endl ;
        #endif
    	N = sqrt( bottom_normal.theta()*bottom_normal.theta()
    	  + bottom_normal.phi()*bottom_normal.phi() ) ;
        bottom_normal.rho( 0.0 ) ;
        bottom_normal.theta( bottom_normal.theta() / N ) ;
        bottom_normal.phi(   bottom_normal.phi() / N  ) ;
    }
    c2 = c*c ;
    height_water = position.rho() - bottom_rho ;

    ndirection.rho(   c2 * ndirection.rho() ) ;
    ndirection.theta( c2 * ndirection.theta() ) ;
    ndirection.phi(   c2 * ndirection.phi() ) ;
    dot_full = bottom_normal.rho() * ndirection.rho()
        + bottom_normal.theta() * ndirection.theta()
        + bottom_normal.phi() * ndirection.phi() ;  // negative #
    max_dot = - max( MIN_REFLECT, (height_water+depth)*bottom_normal.rho() ) ;
    if ( dot_full >= max_dot ) dot_full = max_dot ;
    double grazing = 0.0 ;                                       //added proper logic to account for instances when abs(dot_full/c) >= 1
    if ( dot_full / c >= 1.0 ) { grazing = -M_PI_2 ; }
    else if ( dot_full / c <= -1.0 ) { grazing = M_PI_2 ; }
    else { grazing = asin( -dot_full / c ) ; }

    // invoke bottom reverberation callback
    // @todo THIS IS A STUB FOR FUTURE BEHAVIORS.
    // @todo Add grazing angle to this call.

    #ifdef REFLECT_BOT_DEBUG
        cout << "\t===Infromation after calling bottom::COLLISION_LOCATION===" << endl;
        cout << "\t\tposition(rho,theta,phi): (" << position.rho() - wposition::earth_radius << ", "
                                            << position.theta() << ", "
                                            << position.phi() << ")"
                                            << endl;
        cout << "\t\tndirection(rho,theta,phi): (" << ndirection.rho() << ", "
                                            << ndirection.theta() << ", "
                                            << ndirection.phi() << ")"
                                            << endl;
        cout << "\t\tsound_speed: " << c << "\tc_squared: " << c2 << endl;
        cout << "\t\tbottom_rho: " << bottom_rho - wposition::earth_radius << endl;
        cout << "\t\tbottom_normal(rho,theta,phi): (" << bottom_normal.rho() << ", "
                                                    << bottom_normal.theta() << ", "
                                                    << bottom_normal.phi() << ")"
                                                    << endl;
        cout << "\t\theight_water: " << height_water << endl;
        cout << "\t\tdot_full: " << dot_full << "\tmax_dot: " << max_dot << endl;
        cout << "\t\tgrazing: " << grazing*180.0/M_PI ;
    #endif

    if ( _bottom_reverb ) {
        _bottom_reverb->collision(  de, az, _wave._time+time_water,
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
        position, *(_wave._frequencies), grazing, &amplitude, &phase ) ;
    for ( unsigned f=0 ; f < _wave._frequencies->size() ; ++f ) {
        #ifdef REFLECT_BOT_DEBUG
            cout << "\tBottom loss: " << amplitude(f) << endl ;
        #endif
        _wave._next->attenuation(de,az)(f) += amplitude(f) ;
        _wave._next->phase(de,az)(f) += phase(f) ;
    }

    // change direction of the ray ( R = I - 2 dot(n,I) n )
    // and reinit past, prev, curr, next entries

    dot_full *= 2.0 ;
    ndirection.rho(   ndirection.rho()   - dot_full * bottom_normal.rho() ) ;
    ndirection.theta( ndirection.theta() - dot_full * bottom_normal.theta() ) ;
    ndirection.phi(   ndirection.phi()   - dot_full * bottom_normal.phi() ) ;

    N = sqrt( ndirection.rho() * ndirection.rho()
            + ndirection.theta() * ndirection.theta()
            + ndirection.phi() * ndirection.phi() ) * c ;

    ndirection.rho(   ndirection.rho() / N ) ;
    ndirection.theta( ndirection.theta() / N ) ;
    ndirection.phi(   ndirection.phi() / N ) ;

    #ifdef REFLECT_BOT_DEBUG
        cout << "\t===Infromation before bottom::REFLECTION_REINIT===" << endl;
        cout << "\t\tposition(rho,theta,phi): (" << position.rho() - wposition::earth_radius << ", "
                                            << position.theta() << ", "
                                            << position.phi() << ")"
                                            << endl;
        cout << "\t\tndirection(rho,theta,phi): (" << ndirection.rho() << ", "
                                            << ndirection.theta() << ", "
                                            << ndirection.phi() << ")"
                                            << endl;
        cout << "\t\tbottom_normal(rho,theta,phi): (" << bottom_normal.rho() << ", "
                                                    << bottom_normal.theta() << ", "
                                                    << bottom_normal.phi() << ")"
                                                    << endl;
        cout << "\t\tsound_speed: " << c << "\tdot_full: " << dot_full << "\tN: " << N << endl;
    #endif

    reflection_reinit( de, az, time_water, position, ndirection, c ) ;
    return true ;
}

/**
 * Reflect a single acoustic ray from the ocean surface.
 */
bool reflection_model::surface_reflection( unsigned de, unsigned az ) {
    #ifdef REFLECT_SRF_DEBUG
        cout << "***Entering reflection_model::surface_reflection***" << endl;
    #endif
    boundary_model& boundary = _wave._ocean.surface() ;

    // compute fraction of time step needed to strike the point of collision

    double c = _wave._curr->sound_speed(de,az) ;
    const double d = c*c * _wave._curr->ndirection.rho(de,az) ;
    double time_water = (d==0.0) ? 0.0
        : - _wave._curr->position.altitude(de,az) / d ;

    // compute the precise values for position, direction,
    // sound speed, and grazing angle at the point of collision

    #ifdef REFLECT_SRF_DEBUG
        cout << "\t===Information before calling surface::COLLISION_LOCATION===" << endl;
        cout << "\t\tsound_speed: " << c << "\td: " << d << endl;
        cout << "\t\ttime_water: " << time_water << endl;
    #endif

    wposition1 position ;
    wvector1 ndirection ;
    collision_location( de, az, time_water, &position, &ndirection, &c ) ;
    double grazing = atan2( _wave._curr->ndirection.rho(de,az), sqrt(
        _wave._curr->ndirection.theta(de,az) *
        _wave._curr->ndirection.theta(de,az) +
        _wave._curr->ndirection.phi(de,az) *
        _wave._curr->ndirection.phi(de,az)
    ) ) ;
    #ifdef REFLECT_SRF_DEBUG
        cout << "\t===Infromation after calling surface::COLLISION_LOCATION===" << endl;
        cout << "\t\tposition(rho,theta,phi): (" << position.rho() - wposition::earth_radius << ", "
                                            << position.theta() << ", "
                                            << position.phi() << ")"
                                            << endl;
        cout << "\t\tndirection(rho,theta,phi): (" << ndirection.rho() << ", "
                                            << ndirection.theta() << ", "
                                            << ndirection.phi() << ")"
                                            << endl;
        cout << "\t\tsound_speed: " << c << "\tgrazing: " << grazing << endl;
    #endif
    if ( grazing <= 0.0 ) return false ;	// near miss of the surface

    // invoke surface reverberation callback
    // @todo THIS IS A STUB FOR FUTURE BEHAVIORS.


    if ( _surface_reverb ) {
        _surface_reverb->collision(  de, az, _wave._time+time_water,
            position,  ndirection, c, *(_wave._frequencies),
            _wave._curr->attenuation(de,az), _wave._curr->phase(de,az) ) ;
            // Still need to calculate eigenray ampltiude and phase for
            // reverberation callback. Just passing bogus values currently.
    }

    // compute reflection loss
    // adds reflection attenuation and phase to existing value

    vector<double> amplitude( _wave._frequencies->size() ) ;
    boundary.reflect_loss(
        position, *(_wave._frequencies), grazing, &amplitude ) ;
    for ( unsigned f=0 ; f < _wave._frequencies->size() ; ++f ) {
        _wave._next->attenuation(de,az)(f) += amplitude(f) ;
        _wave._next->phase(de,az)(f) -= M_PI ;
    }

    // change direction of the ray ( Rz = -Iz )
    // and reinit past, prev, curr, next entries

    ndirection.rho( -ndirection.rho() ) ;
    #ifdef REFLECT_SRF_DEBUG
        cout << "\t===Infromation before surface::REFLECTION_REINIT===" << endl;
        cout << "\t\tposition(rho,theta,phi): (" << position.rho() - wposition::earth_radius << ", "
                                            << position.theta() << ", "
                                            << position.phi() << ")"
                                            << endl;
        cout << "\t\tndirection(rho,theta,phi): (" << ndirection.rho() << ", "
                                            << ndirection.theta() << ", "
                                            << ndirection.phi() << ")"
                                            << endl;
        cout << "\t\tsound_speed: " << c << "\ttime_water: " << time_water << endl;
    #endif
    reflection_reinit(de, az, time_water, position, ndirection, c ) ;
    return true ;
}

/**
 * Compute the precise location and direction at the point of collision.
 */
void reflection_model::collision_location(
    unsigned de, unsigned az, double time_water,
    wposition1* position, wvector1* ndirection, double* speed ) const
{
    #ifdef COLLISION_DEBUG
        cout << "***Entering reflection_model::collision_location***" << endl;
    #endif
    double drho, dtheta, dphi, d2rho, d2theta, d2phi ;
    const double time1 = 2.0 * _wave._time_step ;
    const double time2 = _wave._time_step * _wave._time_step ;
    const double dtime2 = time_water * time_water ;

    #ifdef COLLISION_DEBUG
        cout << "\t---Time values---" << endl;
        cout << "\t\ttime1: " << time1
             << "\ttime2: " << time2
             << "\tdtime2: " << dtime2
             << endl;
        cout << "\t---sound speeds---" << endl;
        cout << "\t\t_next sound speed: "
             << _wave._next->sound_speed(de,az) << endl;
        cout << "\t\t_curr sound speed: "
             << _wave._curr->sound_speed(de,az) << endl;
        cout << "\t\t_prev sound speed: "
             << _wave._prev->sound_speed(de,az) << endl;
        cout << "\t\t_past sound speed: "
             << _wave._past->sound_speed(de,az) << endl;
    #endif

    // second order Taylor series for sound speed

    drho = ( _wave._next->sound_speed(de,az)
        - _wave._prev->sound_speed(de,az) )
        / time1 ;

    d2rho = ( _wave._next->sound_speed(de,az)
        + _wave._prev->sound_speed(de,az)
        - 2.0 * _wave._curr->sound_speed(de,az) )
        / time2 ;

    *speed = _wave._curr->sound_speed(de,az)
        + drho * time_water + 0.5 * d2rho * dtime2 ;

    #ifdef COLLISION_DEBUG
        cout << "\t---Taylor Series coeff for sound speed---" << endl;
        cout << "\t\tdrho: " << drho << "\td2rho: " << d2rho << endl;
        cout << "\t\t*speed: " << *speed << endl;
    #endif

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

    #ifdef COLLISION_DEBUG
        cout << "\t---Taylor Series coeff for position---" << endl;
        cout << "\t\tdrho: " << drho
             << "\tdtheta: " << dtheta
             << "\tdphi: " << dphi
             << endl;
        cout << "\t\td2rho: " << d2rho
             << "\td2theta: " << d2theta
             << "\td2phi: " << d2phi
             << endl;
    #endif

    position->rho( _wave._curr->position.rho(de,az)
        + drho * time_water + 0.5 * d2rho * dtime2 ) ;
    position->theta( _wave._curr->position.theta(de,az)
        + dtheta * time_water + 0.5 * d2theta * dtime2 ) ;
    position->phi( _wave._curr->position.phi(de,az)
        + dphi * time_water + 0.5 * d2phi * dtime2 ) ;

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

    #ifdef COLLISION_DEBUG
        cout << "\t---Taylor Series coeff for ndirection---" << endl;
        cout << "\t\tdrho: " << drho
             << "\tdtheta: " << dtheta
             << "\tdphi: " << dphi
             << endl;
        cout << "\t\td2rho: " << d2rho
             << "\td2theta: " << d2theta
             << "\td2phi: " << d2phi
             << endl;
    #endif

    ndirection->rho( _wave._curr->ndirection.rho(de,az)
        + drho * time_water + 0.5 * d2rho * dtime2 ) ;
    ndirection->theta( _wave._curr->ndirection.theta(de,az)
        + dtheta * time_water + 0.5 * d2theta * dtime2 ) ;
    ndirection->phi( _wave._curr->ndirection.phi(de,az)
        + dphi * time_water + 0.5 * d2phi * dtime2 ) ;
}

/**
 * Re-initialize an individual ray after reflection.
 */
void reflection_model::reflection_reinit(
    unsigned de, unsigned az, double time_water,
    const wposition1& position, const wvector1& ndirection, double speed )
{

    // create temporary 1x1 wavefront elements

    wave_front past( _wave._ocean, _wave._frequencies, 1, 1 ) ;
    wave_front prev( _wave._ocean, _wave._frequencies, 1, 1 ) ;
    wave_front curr( _wave._ocean, _wave._frequencies, 1, 1 ) ;
    wave_front next( _wave._ocean, _wave._frequencies, 1, 1 ) ;
    wave_front temp( _wave._ocean, _wave._frequencies, 1, 1 ) ;

    // initialize current entry with reflected position and direction
    // adapted from wave_front::init_wave()

    temp.position.rho(   0, 0, position.rho() ) ;
    temp.position.theta( 0, 0, position.theta() ) ;
    temp.position.phi(   0, 0, position.phi() ) ;

    temp.ndirection.rho(   0, 0, ndirection.rho()  ) ;
    temp.ndirection.theta( 0, 0, ndirection.theta() ) ;
    temp.ndirection.phi(   0, 0, ndirection.phi() ) ;
    temp.update() ;

    // Runge-Kutta to initialize current entry "time_water" seconds in the past
    // adapted from wave_queue::init_wavefronts()

    ode_integ::rk1_pos(  - time_water, &temp, &next ) ;
    ode_integ::rk1_ndir( - time_water, &temp, &next ) ;
    next.update() ;

    ode_integ::rk2_pos(  - time_water, &temp, &next, &past ) ;
    ode_integ::rk2_ndir( - time_water, &temp, &next, &past ) ;
    past.update() ;

    ode_integ::rk3_pos(  - time_water, &temp, &next, &past, &curr ) ;
    ode_integ::rk3_ndir( - time_water, &temp, &next, &past, &curr ) ;
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

    ode_integ::rk2_pos(  - time_step, &prev, &next, &temp ) ;
    ode_integ::rk2_ndir( - time_step, &prev, &next, &temp ) ;
    past.update() ;

    ode_integ::rk3_pos(  - time_step, &prev, &next, &temp, &past ) ;
    ode_integ::rk3_ndir( - time_step, &prev, &next, &temp, &past ) ;
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
