/**
 * @file wave_queue_reverb.cc
 * Wavefront propagation as a function of time.
 */
#include <usml/waveq3d/wave_queue_reverb.h>
#include <usml/waveq3d/ode_integ.h>
#include <usml/waveq3d/reflection_model.h>
#include <usml/waveq3d/spreading_ray.h>
#include <usml/waveq3d/spreading_hybrid_gaussian.h>

#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublas/triangular.hpp>
#include <boost/numeric/ublas/lu.hpp>

#include <iomanip>

using namespace usml::waveq3d ;

/**
 * Initialize a propagation scenario.
 */
wave_queue_reverb::wave_queue_reverb(
    ocean_model& ocean,
    const seq_vector& freq,
    const wposition1& pos,
    const seq_vector& de,
    const seq_vector& az,
    double time_step,
    double pulse,
    unsigned num_bins,
    double max_time,
    reverberation_model* reverb,
    unsigned origin,
    const wposition* targets,
    spreading_type type
) :
    wave_queue(ocean, freq, pos, de, az, time_step, targets, type),
    _origin( origin )
{
    // Define reverberation model
    if ( reverb ) { _reverberation_model = reverb ; }
    else {
        _reverberation_model = new
        eigenverb_monostatic( ocean, *this, pulse, num_bins, max_time ) ;
    }
    delete _reflection_model ;
    _reflection_model = new reflection_model( *this, _reverberation_model ) ;
}

/**
 * Detect and process boundary reflections and caustics.
 */
void wave_queue_reverb::detect_reflections() {

    // process all surface and bottom reflections, and vertices
    // note that multiple rays can reflect in the same time step

    for (unsigned de = 0; de < num_de(); ++de) {
        for (unsigned az = 0; az < num_az(); ++az) {
            if ( !detect_reflections_surface(de,az) ) {
                if( !detect_reflections_bottom(de,az) ) {
                    detect_vertices(de,az) ;
                }
            }
        }
    }

    detect_volume_reflections() ;

    // search for other changes in wavefront
    _next->find_edges() ;
    detect_caustics() ;
}

/**
 * Detect volume boundary reflections for reverberation contributions
 */
void wave_queue_reverb::detect_volume_reflections() {
    unsigned n = _ocean.volume()->getNumberOfLayers() ;
    for(unsigned i=0; i<n; ++i) {
        boundary_model* layer = _ocean.volume()->getLayer(i) ;
        for (unsigned de = 0; de < num_de(); ++de) {
            for (unsigned az = 0; az < num_az(); ++az) {
                double height ;
                wposition1 pos_curr( _curr->position, de, az ) ;
                wposition1 pos_next( _next->position, de, az ) ;
                layer->height( pos_next, &height, NULL, true ) ;
                double d1 = height - pos_next.rho() ;
                double d2 = height - pos_curr.rho() ;
                if ( d1 > 0 && d2 < 0 ) { collide_from_above(de,az, d1, i) ; }
                if ( d1 < 0 && d2 > 0 ) { collide_from_below(de,az, d2, i) ; }
            }
        }
    }
}

void wave_queue_reverb::collide_from_above(
        unsigned de, unsigned az, double depth, unsigned layer )
{
    double MIN_REFLECT = 6.0 ;
    // extract position, direction, and sound speed from this ray
    // at a point just before it goes below the layer

    wposition1 position( _curr->position, de, az ) ;
    wvector1 ndirection( _curr->ndirection, de, az ) ;
    double c = _curr->sound_speed( de, az ) ;
    double c2 = c*c ;

    // extract radial height and slope at current location
    // height_water = initial ray height above the layer (must be positive)

    double layer_rho ;
    wvector1 layer_normal ;
    boundary_model* boundary = _ocean.volume()->getLayer(layer) ;
    boundary->height( position, &layer_rho, &layer_normal ) ;
    double height_water = position.rho() - layer_rho ;

    // compute dot_full = dot product of the full dr/dt with layer_normal (negative #)
    // converts ndirection to dr/dt in rectangular coordinates relative to reflection point

    ndirection.rho(   c2 * ndirection.rho() ) ;
    ndirection.theta( c2 * ndirection.theta() ) ;
    ndirection.phi(   c2 * ndirection.phi() ) ;
    double dot_full = layer_normal.rho() * ndirection.rho()
                    + layer_normal.theta() * ndirection.theta()
                    + layer_normal.phi() * ndirection.phi() ;

    // compute the smallest "dot_full" that could have led to this penetration depth
    // assume minimum depth change, along normal, of 1.0 meters

    double max_dot = - max( MIN_REFLECT, (height_water+depth)*layer_normal.rho() ) ;
    if ( dot_full >= max_dot ) dot_full = max_dot ;

    // compute time_water = fraction of time step needed to strike the layer
    // time step = ratio of in water dot product to full dot product
    // dot_water = component of "height_water" parallel to layer normal (negative #)

    const double dot_water = -height_water * layer_normal.rho() ;
    double time_water = max( 0.0, dot_water / dot_full ) ;

    // compute the more precise values for position, direction,
    // sound speed, layer height, layer slope, and grazing angle at the point of collision.
    // reduces grazing angle errors in highly refractive environments.

    collision_location( de, az, time_water, &position, &ndirection, &c ) ;
    boundary->height( position, &layer_rho, &layer_normal ) ;
    c2 = c*c ;
    height_water = position.rho() - layer_rho ;

    ndirection.rho(   c2 * ndirection.rho() ) ;
    ndirection.theta( c2 * ndirection.theta() ) ;
    ndirection.phi(   c2 * ndirection.phi() ) ;
    dot_full = layer_normal.rho() * ndirection.rho()
        + layer_normal.theta() * ndirection.theta()
        + layer_normal.phi() * ndirection.phi() ;  // negative #
    max_dot = - max( MIN_REFLECT, (height_water+depth)*layer_normal.rho() ) ;
    if ( dot_full >= max_dot ) dot_full = max_dot ;
    double grazing = 0.0 ;                                       //added proper logic to account for instances when abs(dot_full/c) >= 1
    if ( dot_full / c >= 1.0 ) { grazing = -M_PI_2 ; }
    else if ( dot_full / c <= -1.0 ) { grazing = M_PI_2 ; }
    else { grazing = asin( -dot_full / c ) ; }

    /// @todo create a reflection class purely for reverberation
    unsigned ID = 10 ;
//    unsigned ID = _origin ;
    ID += layer ;
    _reverberation_model->notifyLowerCollision( de, az, _time, time_water,
        grazing, c, *(_frequencies), position,  ndirection, ID ) ;
        // Still need to calculate eigenray ampltiude and phase for
        // reverberation callback. Just passing bogus values currently.
}

/// @todo correct the logic for colliding from below the layer, this should be opposite of colliding from above
void wave_queue_reverb::collide_from_below(
        unsigned de, unsigned az, double depth, unsigned layer )
{
    double MIN_REFLECT = 6.0 ;
    // extract position, direction, and sound speed from this ray
    // at a point just before it goes below the layer

    wposition1 position( _curr->position, de, az ) ;
    wvector1 ndirection( _curr->ndirection, de, az ) ;
    double c = _curr->sound_speed( de, az ) ;
    double c2 = c*c ;

    // extract radial height and slope at current location
    // height_water = initial ray height above the layer (must be positive)

    double layer_rho ;
    wvector1 layer_normal ;
    boundary_model* boundary = _ocean.volume()->getLayer(layer) ;
    boundary->height( position, &layer_rho, &layer_normal ) ;
    double height_water = layer_rho - position.rho() ;

    // compute dot_full = dot product of the full dr/dt with layer_normal (negative #)
    // converts ndirection to dr/dt in rectangular coordinates relative to reflection point

    ndirection.rho(   c2 * ndirection.rho() ) ;
    ndirection.theta( c2 * ndirection.theta() ) ;
    ndirection.phi(   c2 * ndirection.phi() ) ;
    double dot_full = layer_normal.rho() * ndirection.rho()
                    + layer_normal.theta() * ndirection.theta()
                    + layer_normal.phi() * ndirection.phi() ;

    // compute the smallest "dot_full" that could have led to this penetration depth
    // assume minimum depth change, along normal, of 1.0 meters

    double max_dot = - max( MIN_REFLECT, (height_water-depth)*layer_normal.rho() ) ;
    if ( dot_full >= max_dot ) dot_full = max_dot ;

    // compute time_water = fraction of time step needed to strike the layer
    // time step = ratio of in water dot product to full dot product
    // dot_water = component of "height_water" parallel to layer normal (negative #)

    const double dot_water = -height_water * layer_normal.rho() ;
    double time_water = max( 0.0, dot_water / dot_full ) ;

    // compute the more precise values for position, direction,
    // sound speed, layer height, layer slope, and grazing angle at the point of collision.
    // reduces grazing angle errors in highly refractive environments.

    collision_location( de, az, time_water, &position, &ndirection, &c ) ;
    boundary->height( position, &layer_rho, &layer_normal ) ;
    c2 = c*c ;
    height_water = layer_rho - position.rho() ;

    ndirection.rho(   c2 * ndirection.rho() ) ;
    ndirection.theta( c2 * ndirection.theta() ) ;
    ndirection.phi(   c2 * ndirection.phi() ) ;
    dot_full = layer_normal.rho() * ndirection.rho()
        + layer_normal.theta() * ndirection.theta()
        + layer_normal.phi() * ndirection.phi() ;  // negative #
    max_dot = - max( MIN_REFLECT, (height_water-depth)*layer_normal.rho() ) ;
    if ( dot_full >= max_dot ) dot_full = max_dot ;
    double grazing = 0.0 ;                                       //added proper logic to account for instances when abs(dot_full/c) >= 1
    if ( dot_full / c >= 1.0 ) { grazing = -M_PI_2 ; }
    else if ( dot_full / c <= -1.0 ) { grazing = M_PI_2 ; }
    else { grazing = asin( -dot_full / c ) ; }

    /// @todo create a reflection class purely for reverberation
    unsigned ID = 10 ;
//    unsigned ID = _origin ;
    ID += layer ;
    _reverberation_model->notifyUpperCollision( de, az, _time, time_water,
        grazing, c, *(_frequencies), position,  ndirection, ID ) ;
        // Still need to calculate eigenray ampltiude and phase for
        // reverberation callback. Just passing bogus values currently.
}

/**
 * Compute the precise location and direction at the point of collision.
 */
void wave_queue_reverb::collision_location(
    unsigned de, unsigned az, double time_water,
    wposition1* position, wvector1* ndirection, double* speed ) const
{
    double drho, dtheta, dphi, d2rho, d2theta, d2phi ;
    const double time1 = 2.0 * _time_step ;
    const double time2 = _time_step * _time_step ;
    const double dtime2 = time_water * time_water ;

    // second order Taylor series for sound speed

    drho = ( _next->sound_speed(de,az)
        - _prev->sound_speed(de,az) )
        / time1 ;

    d2rho = ( _next->sound_speed(de,az)
        + _prev->sound_speed(de,az)
        - 2.0 * _curr->sound_speed(de,az) )
        / time2 ;

    *speed = _curr->sound_speed(de,az)
        + drho * time_water + 0.5 * d2rho * dtime2 ;

    // second order Taylor series for position

    drho = ( _next->position.rho(de,az)
        - _prev->position.rho(de,az) )
        / time1 ;
    dtheta = ( _next->position.theta(de,az)
        - _prev->position.theta(de,az) )
        / time1 ;
    dphi = ( _next->position.phi(de,az)
        - _prev->position.phi(de,az) )
        / time1 ;

    d2rho = ( _next->position.rho(de,az)
        + _prev->position.rho(de,az)
        - 2.0 * _curr->position.rho(de,az) )
        / time2 ;
    d2theta = ( _next->position.theta(de,az)
        + _prev->position.theta(de,az)
        - 2.0 * _curr->position.theta(de,az) )
        / time2 ;
    d2phi = ( _next->position.phi(de,az)
        + _prev->position.phi(de,az)
        - 2.0 * _curr->position.phi(de,az) )
        / time2 ;

    position->rho( _curr->position.rho(de,az)
        + drho * time_water + 0.5 * d2rho * dtime2 ) ;
    position->theta( _curr->position.theta(de,az)
        + dtheta * time_water + 0.5 * d2theta * dtime2 ) ;
    position->phi( _curr->position.phi(de,az)
        + dphi * time_water + 0.5 * d2phi * dtime2 ) ;

    // second order Taylor series for ndirection

    drho = ( _next->ndirection.rho(de,az)
        - _prev->ndirection.rho(de,az) )
        / time1 ;
    dtheta = ( _next->ndirection.theta(de,az)
        - _prev->ndirection.theta(de,az) )
        / time1 ;
    dphi = ( _next->ndirection.phi(de,az)
        - _prev->ndirection.phi(de,az) )
        / time1 ;

    d2rho = ( _next->ndirection.rho(de,az)
        + _prev->ndirection.rho(de,az)
        - 2.0 * _curr->ndirection.rho(de,az) )
        / time2 ;
    d2theta = ( _next->ndirection.theta(de,az)
        + _prev->ndirection.theta(de,az)
        - 2.0 * _curr->ndirection.theta(de,az) )
        / time2 ;
    d2phi = ( _next->ndirection.phi(de,az)
        + _prev->ndirection.phi(de,az)
        - 2.0 * _curr->ndirection.phi(de,az) )
        / time2 ;

    ndirection->rho( _curr->ndirection.rho(de,az)
        + drho * time_water + 0.5 * d2rho * dtime2 ) ;
    ndirection->theta( _curr->ndirection.theta(de,az)
        + dtheta * time_water + 0.5 * d2theta * dtime2 ) ;
    ndirection->phi( _curr->ndirection.phi(de,az)
        + dphi * time_water + 0.5 * d2phi * dtime2 ) ;
}
