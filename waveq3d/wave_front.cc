/**
 * @file wave_front.cc
 * Wavefront characteristics at a specific point in time.
 */
#include <usml/waveq3d/wave_front.h>

//#define SSP_DEBUG

using namespace usml::waveq3d ;

/**
 * Create workspace for all properties.
 */
wave_front::wave_front(
    ocean_model& ocean,
    const seq_vector* freq,
    unsigned num_de,
    unsigned num_az,
    const wposition* targets,
    const matrix<double>* sin_theta
) :
    position( num_de, num_az ),
    pos_gradient( num_de, num_az ),
    ndirection( num_de, num_az ),
    ndir_gradient( num_de, num_az ),
    sound_speed( num_de, num_az ),
    sound_gradient( num_de, num_az ),
    attenuation( num_de, num_az ),
    phase( num_de, num_az ),
    distance( num_de, num_az ),
    surface( num_de, num_az ),
    bottom( num_de, num_az ),
    caustic( num_de, num_az ),
    on_edge( num_de, num_az ),
    targets( targets ),
    _ocean( ocean ),
    _frequencies( freq ),
    _dc_c( num_de, num_az ),
    _c2_r( num_de, num_az ),
    _sin_theta( num_de, num_az ),
    _cot_theta( num_de, num_az ),
    _target_sin_theta( sin_theta )
{
    sound_speed.clear() ;
    distance.clear() ;
    surface.clear() ;
    bottom.clear() ;
    caustic.clear() ;
    on_edge.clear() ;

    for ( unsigned n1=0 ; n1 < num_de ; ++n1 ) {
        for ( unsigned n2=0 ; n2 < num_az ; ++n2 ) {
            attenuation(n1,n2).resize( freq->size() ) ;
            attenuation(n1,n2).clear() ;
            phase(n1,n2).resize( freq->size() ) ;
            phase(n1,n2).clear() ;
        }
    }

    if ( this->targets ) {
        distance2.resize( this->targets->size1(), this->targets->size2() ) ;
        for ( unsigned n1=0 ; n1 < this->targets->size1() ; ++n1 ) {
            for ( unsigned n2=0 ; n2 < this->targets->size2() ; ++n2 ) {
                distance2(n1,n2).resize( num_de, num_az ) ;
                distance2(n1,n2).clear() ;
            }
        }
    }
}

/**
 * Initialize position and direction components of the wavefront.
 */
void wave_front::init_wave(
    const wposition1& pos, const seq_vector& de, const seq_vector& az )
{
    // compute direction for all D/E and AZ combinations

    for ( unsigned r=0 ; r < de.size() ; ++r ) {
        double cos_de = cos( to_radians( de(r) ) ) ;
        double sin_de = sin( to_radians( de(r) ) );
        for ( unsigned c=0 ; c < az.size() ; ++c ) {
            double cos_az = cos( to_radians( az(c) ) ) ;
            double sin_az = sin( to_radians( az(c) ) ) ;

            // compute direction relative to east, north, up coord system

            ndirection.rho( r, c, sin_de ) ;
            ndirection.theta( r, c, -cos_de * cos_az ) ;
            ndirection.phi( r, c, cos_de * sin_az ) ;

            // copy position from pos argument

            position.rho(   r, c, pos.rho() ) ;
            position.theta( r, c, pos.theta() ) ;
            position.phi(   r, c, pos.phi() ) ;
        }
    }

    // normalize direction using sound speed at initial location

    wposition position(1,1) ;
    position.rho( 0, 0, pos.rho() ) ;
    position.theta( 0, 0, pos.theta() ) ;
    position.phi( 0, 0, pos.phi() ) ;

    matrix<double> c(1,1) ;
    _ocean.profile().sound_speed( position, &c ) ;
    ndirection.rho(   ndirection.rho()   / c(0,0), false ) ;
    ndirection.theta( ndirection.theta() / c(0,0), false ) ;
    ndirection.phi(   ndirection.phi()   / c(0,0), false ) ;
}
/**
 * Update properties based on the current position and direction vectors.
 */
void wave_front::update() {

    // compute the sound_speed, sound_gradient, attenuation, and phase
    // elements of the ocean profile.

    compute_profile();

    // compute commonly used terms in the wave propagation derivatives

    _dc_c.rho(element_div(sound_gradient.rho(), sound_speed));
    _dc_c.theta(element_div(sound_gradient.theta(), sound_speed));
    _dc_c.phi(element_div(sound_gradient.phi(), sound_speed));
    noalias(_sin_theta) = sin(position.theta());
    noalias(_cot_theta) = element_div(cos(position.theta()), _sin_theta);

    // update wave propagation position derivatives
    // Reilly eqns. 36-38

    _c2_r = abs2(sound_speed);
    pos_gradient.rho(element_prod(_c2_r, ndirection.rho()));
    _c2_r = element_div(_c2_r, position.rho());
    pos_gradient.theta(element_prod(_c2_r, ndirection.theta()));
    pos_gradient.phi(element_prod(
        element_div(_c2_r, _sin_theta),
        ndirection.phi()));

    // update wave propagation direction derivatives
    // Reilly eqns. 39-41

    ndir_gradient.rho(
        element_prod(_c2_r, abs2(ndirection.theta()) + abs2(ndirection.phi()))
            - _dc_c.rho()
        );
    ndir_gradient.theta(
        element_prod(-_c2_r,
            element_prod(ndirection.rho(), ndirection.theta())
            - element_prod(abs2(ndirection.phi()), _cot_theta))
            - element_div(_dc_c.theta(), position.rho())
        );
    ndir_gradient.phi(
        element_prod(-_c2_r,
            element_prod(ndirection.phi(),
                ndirection.rho() + element_prod(ndirection.theta(), _cot_theta)
            )
        )
        - element_div(_dc_c.phi(), element_prod(position.rho(), _sin_theta))
        );

    // update data that relies on new wavefront locations

    if (targets) compute_target_distance();
}

/**
 * Find all edges in the ray fan.  Sets on_edge(de,az) to true if
 * it is on the edge of the ray fan or one of its neighbors has
 * a different surface, bottom, or caustic count.
 */
void wave_front::find_edges() {
    on_edge.clear() ;
    const unsigned max_de = num_de() - 1 ;

    // mark the perimeter of the ray fan
    // also treat the case where num_de()=1 or num_az()=1

    for ( unsigned az=0 ; az < num_az() ; ++az ) {
        on_edge(0,az) = on_edge(max_de,az) = true ;
    }

    // search for changes around each (de,az)
    // skip by 2 to avoid counting each change twice

    for ( unsigned az=0 ; az < num_az() ; az += 1 ) {
        for ( unsigned de=1 ; de < max_de ; de += 1 ) {
            if ( (position.rho(de,az) < position.rho(de+1,az) &&
                  position.rho(de,az) < position.rho(de-1,az)) ||
                 (position.rho(de,az) > position.rho(de+1,az) &&
                  position.rho(de,az) > position.rho(de-1,az)) ) {
                    on_edge(de,az) = true;
                    if( abs(ndirection.rho(de,az)-ndirection.rho(de-1,az)) >
                        abs(ndirection.rho(de,az)-ndirection.rho(de+1,az)) ) {
                            on_edge(de-1,az) = true;
                    } else {
                            on_edge(de+1,az) = true;
                    }
            }
        }
    }
}

/**
 * Compute a fast an approximation of the distance squared from each
 * target to each point on the wavefront.
 */
void wave_front::compute_target_distance() {
    for ( unsigned n1=0 ; n1 < targets->size1() ; ++n1 ) {
        for ( unsigned n2=0 ; n2 < targets->size2() ; ++n2 ) {
            wvector1 from( *targets, n1, n2 ) ;
            noalias(distance2(n1,n2)) = abs(
                abs2(position.rho()) + from.rho()*from.rho() - 2.0 * from.rho()
                * element_prod( position.rho(), 1.0 - 2.0 * (
                abs2(0.5*(position.theta()-from.theta()))
                + (*_target_sin_theta)(n1,n2) * element_prod( _sin_theta,
                abs2(0.5*(position.phi()-from.phi())) )) ) );
        }
    }
}

/**
 * Compute terms in the sound speed profile as fast as possible.
 */
void wave_front::compute_profile() {
    #ifdef SSP_DEBUG
        cout << "***Entering wave_front::compute_profile()***" << endl;
    #endif
    _ocean.profile().sound_speed( position, &sound_speed, &sound_gradient);
    _ocean.profile().attenuation( position, *_frequencies, distance, &attenuation);
    #ifdef SSP_DEBUG
        cout << "\tsound_speed: " << sound_speed << endl;
        cout << "\t---sound_gradient---" << endl;
        for ( unsigned n1=0 ; n1 < num_de() ; ++n1 ) {
            cout << "\t\t" << n1 << ",:" << endl;
            for ( unsigned n2=0 ; n2 < num_az() ; ++n2 ) {
                if(n2!=0) {cout << "\t" ;}
                else {cout << "\t\t\t" ;}
                cout << "(" << sound_gradient.rho(n1,n2) << ", "
                       << sound_gradient.theta(n1,n2) << ", "
                       << sound_gradient.phi(n1,n2) << ")" ;
            }
            cout << endl;
        }
    #endif
    for (unsigned de = 0; de < position.size1(); ++de) {
        for (unsigned az = 0; az < position.size2(); ++az) {
            phase(de, az).clear();
        }
    }
}
