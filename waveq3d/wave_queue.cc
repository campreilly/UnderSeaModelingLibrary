/**
 * @file wave_queue.cc
 * Wavefront propagation as a function of time.
 */
#include <usml/waveq3d/wave_queue.h>
#include <usml/waveq3d/ode_integ.h>
#include <usml/waveq3d/reflection_model.h>
#include <usml/waveq3d/spreading_ray.h>
#include <usml/waveq3d/spreading_hybrid_gaussian.h>
#include <usml/waveq3d/proploss.h>

#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublas/triangular.hpp>
#include <boost/numeric/ublas/lu.hpp>

#include <iomanip>

#define DEBUG_EIGENRAYS
//#define DEBUG_CAUSTICS

using namespace usml::waveq3d ;

/**
 * Initialize a propagation scenario.
 */
wave_queue::wave_queue(
    ocean_model& ocean,
    const seq_vector& freq,
    const wposition1& pos,
    const seq_vector& de, const seq_vector& az,
    double time_step,
    proploss* prop_loss,
    spreading_type type
) :
    _ocean( ocean ),
    _frequencies( freq.clone() ),
    _source_pos( pos ),
    _source_de( de.clone() ),
    _source_az( az.clone() ),
    _time_step( time_step ),
    _time( 0.0 ),
    _proploss( prop_loss ),
    _nc_file( NULL )
{
    // create references between proploss and wavefront objects.

    const wposition* targets = NULL ;
    const matrix<double>* target_sin_theta = NULL ;
    if ( _proploss ) {
    	targets = _proploss->_targets ;
    	_proploss->initialize( _frequencies, &_source_pos,
    		_source_de, _source_az, _time_step ) ;
        target_sin_theta = &_proploss->_sin_theta ;
    }

    // create storage space for all wavefront elements

    _past = new wave_front( _ocean, _frequencies, de.size(), az.size(), targets, target_sin_theta ) ;
    _prev = new wave_front( _ocean, _frequencies, de.size(), az.size(), targets, target_sin_theta ) ;
    _curr = new wave_front( _ocean, _frequencies, de.size(), az.size(), targets, target_sin_theta ) ;
    _next = new wave_front( _ocean, _frequencies, de.size(), az.size(), targets, target_sin_theta ) ;

    // initialize wave front elements

    _curr->init_wave( pos, de, az ) ;
    _curr->update() ;
    init_wavefronts() ;
    _reflection_model = new reflection_model( *this ) ;
    _spreading_model = NULL ;
    if ( _proploss ) {
        switch( type ) {
            case HYBRID_GAUSSIAN :
                _spreading_model = new spreading_hybrid_gaussian( *this ) ;
                break ;
            default :
                _spreading_model = new spreading_ray( *this ) ;
                break ;
        }
    }
}

/** Destroy all temporary memory. */
wave_queue::~wave_queue() {
    delete _frequencies ;
    if ( _spreading_model ) delete _spreading_model ;
    delete _reflection_model ;
    delete _source_de ;
    delete _source_az ;
    delete _past ;
    delete _prev ;
    delete _curr ;
    delete _next ;
}

/**
 * Register a bottom reverberation model.
 */
void wave_queue::set_bottom_reverb( reverb_model* model ) {
    _reflection_model->_bottom_reverb = model ;
}

/**
 * Register a surface reverberation model.
 */
void wave_queue::set_surface_reverb( reverb_model* model ) {
    _reflection_model->_surface_reverb = model ;
}

/**
 * Initialize wavefronts at the start of propagation using a
 * 3rd order Runge-Kutta algorithm.
 */
void wave_queue::init_wavefronts() {

    // Runge-Kutta to estimate _prev wavefront from _curr entry

    ode_integ::rk1_pos(  - _time_step, _curr, _next ) ;
    ode_integ::rk1_ndir( - _time_step, _curr, _next ) ;
    _next->update() ;

    ode_integ::rk2_pos(  - _time_step, _curr, _next, _past ) ;
    ode_integ::rk2_ndir( - _time_step, _curr, _next, _past ) ;
    _past->update() ;

    ode_integ::rk3_pos(  - _time_step, _curr, _next, _past, _prev ) ;
    ode_integ::rk3_ndir( - _time_step, _curr, _next, _past, _prev ) ;
    _prev->update() ;

    // Runge-Kutta to estimate _past wavefront from _prev entry

    ode_integ::rk1_pos(  - _time_step, _prev, _next ) ;
    ode_integ::rk1_ndir( - _time_step, _prev, _next ) ;
    _next->update() ;

    ode_integ::rk2_pos(  - _time_step, _prev, _next, _past ) ;
    ode_integ::rk2_ndir( - _time_step, _prev, _next, _past ) ;
    _past->update() ;

    ode_integ::rk3_pos(  - _time_step, _prev, _next, _past, _past, false ) ;
    ode_integ::rk3_ndir( - _time_step, _prev, _next, _past, _past, false ) ;
    _past->update() ;

    // Adams-Bashforth to estimate _next wavefront
    // from _past, _prev, and _curr entries

    ode_integ::ab3_pos(  _time_step, _past, _prev, _curr, _next ) ;
    ode_integ::ab3_ndir( _time_step, _past, _prev, _curr, _next ) ;
    _next->update() ;
}

/**
 * Marches to the next integration step in the acoustic propagation.
 */
void wave_queue::step() {

    // search for caustics and boundary reflections

    detect_reflections() ;

    // rotate wavefront queue to the next step.

    wave_front* save = _past ;
    _past = _prev ;
    _prev = _curr ;
    _curr = _next ;
    _next = save ;
    _time += _time_step ;

    #if defined(DEBUG_EIGENRAYS) || defined(DEBUG_CAUSTICS)
        cout << "*** wave_queue::step: time=" << time() << endl ;
    #endif

    // compute position, direction, and environment parameters for next entry

    ode_integ::ab3_pos(  _time_step, _past, _prev, _curr, _next ) ;
    ode_integ::ab3_ndir( _time_step, _past, _prev, _curr, _next ) ;

    _next->update() ;

    _next->attenuation += _curr->attenuation ;
    _next->phase += _curr->phase ;
    _next->surface = _curr->surface ;
    _next->bottom = _curr->bottom ;
    _next->caustic = _curr->caustic ;

    // search for eigenray collisions with acoustic targets

    detect_eigenrays() ;
}

/**
 * Detect and process boundary reflections and caustics.
 */
void wave_queue::detect_reflections() {

    // process all surface and bottom reflections
    // note that multiple rays can reflect in the same time step

    for (unsigned de = 0; de < num_de(); ++de) {
        for (unsigned az = 0; az < num_az(); ++az) {
            if ( !detect_reflections_surface(de,az) ) {
                detect_reflections_bottom(de,az) ;
            }
        }
    }

    // search for other changes in wavefront

    _next->find_edges() ;
    detect_caustics() ;
}

/**
 * Detect and process reflection for a single (DE,AZ) combination.
 */
bool wave_queue::detect_reflections_surface( unsigned de, unsigned az ) {
    if (_next->position.altitude(de,az) > 0.0) {
        if (_reflection_model->surface_reflection(de,az)) {
            _next->surface(de,az) += 1;
            _curr->surface(de,az) = _prev->surface(de,az)
                    = _past->surface(de,az) = _next->surface(de,az) ;
            detect_reflections_bottom(de,az);
            return true; // indicate a surface reflection
        }
    }
    return false; // indicates no reflection
}

/**
 * Detect and process reflection for a single (DE,AZ) combination.
 */
bool wave_queue::detect_reflections_bottom( unsigned de, unsigned az ) {
    double height ;
    wposition1 pos( _next->position, de, az ) ;
    _ocean.bottom().height( pos, &height ) ;
    const double depth = height - _next->position.rho(de,az) ;
    if ( depth > 0.0 ) {
        if ( _reflection_model->bottom_reflection( de, az, depth ) ) {
            _next->bottom(de,az) += 1 ;
            _curr->bottom(de,az) = _prev->bottom(de,az)
                    = _past->bottom(de,az) = _next->bottom(de,az) ;
            detect_reflections_surface( de, az ) ;
            return true ;	// indicate a surface reflection
        }
    }
    return false ;	// indicates no reflection
}

/**
 *  Detects and processes the caustics along the next wavefront
 */
void wave_queue::detect_caustics() {
    const unsigned max_de = num_de() - 1 ;

    for ( unsigned a=0 ; a < num_az() ; a++ ) {
        for ( unsigned d=1 ; d < max_de ; d++ ) {
            double A = _curr->position.rho(d+1,a) ;
            double B = _curr->position.rho(d,a) ;
            double C = _next->position.rho(d+1,a) ;
            double D = _next->position.rho(d,a) ;
            bool fold = false ;
            if ( (_next->surface(d+1,a) == _next->surface(d,a)) &&
                 (_next->bottom(d+1,a) == _next->bottom(d,a)) ) { fold = true; }
            if ( (C-D)*(A-B) < 0 && fold ) {
                _next->caustic(d+1,a)++;
                for (unsigned f = 0; f < _frequencies->size(); ++f) {
                    _next->phase(d+1,a)(f) -= M_PI_2;
                }
            }
        }
    }
}

/**
 * Detect and process wavefront closest point of approach (CPA) with target.
 */
void wave_queue::detect_eigenrays() {
    if ( _proploss == NULL ) return ;
    double distance2[3][3][3] ;

    // loop over all targets

    for ( unsigned t1=0 ; t1 < _proploss->size1() ; ++t1 ) {
        for ( unsigned t2=0 ; t2 < _proploss->size2() ; ++t2 ) {

            // loop over all ray paths

            for ( unsigned de=1 ; de < num_de()-1 ; ++de ) {
                for ( unsigned az=1 ; az < num_az()-1 ; ++az ) {
                    if ( is_closest_ray(t1,t2,de,az,distance2) ) {
                        add_eigenray(t1,t2,de,az,distance2) ;
                    }
                }
            }

        }   // end t2 loop
    }   // end t1 loop
}

/**
 * Used by detect_eigenrays() to discover if the current ray is the
 * closest point of approach to the current target.
 */
bool wave_queue::is_closest_ray(
   unsigned t1, unsigned t2,
   unsigned de, unsigned az,
   double distance2[3][3][3]
) {
    // exit early if central ray is at the edge of ray family
    // prevents edges from acting as CPA

    if ( _curr->on_edge(de,az) ) {
        return false ;
    }

    // test the central ray

    memset( distance2, 0, 3*3*3*sizeof(double) ) ;
    double& center = distance2[1][1][1] ;
    center = _curr->distance2(t1,t2)(de,az) ;

    distance2[2][1][1] = _next->distance2(t1,t2)(de,az) ;
    if ( distance2[2][1][1] <= center ) return false ;

    distance2[0][1][1] = _prev->distance2(t1,t2)(de,az) ;
    if ( distance2[0][1][1] < center ) return false ;

    // test all neigbors that are not the central ray

    for ( unsigned nde=0 ; nde < 3 ; ++nde ) {
        for ( unsigned naz=0 ; naz < 3 ; ++naz ) {
            if ( nde == 1 && naz == 1 ) continue ;

            // compute distances on the current, next, and previous wavefronts

            unsigned d = de + nde - 1 ;
            unsigned a = az + naz - 1 ;
            distance2[0][nde][naz] = _prev->distance2(t1,t2)(d,a) ;
            distance2[1][nde][naz] = _curr->distance2(t1,t2)(d,a) ;
            distance2[2][nde][naz] = _next->distance2(t1,t2)(d,a) ;

            #ifdef USML_DEBUG
            // test all distances to make sure they are valid numbers
                if( isnan(distance2[0][nde][naz]) ) {
                    cout << "Oops, the distance for distance2[0"
                    << "][" << nde << "][" << naz << "] is NaN!" << endl;
                }
                if( isnan(distance2[1][nde][naz]) ) {
                    cout << "Oops, the distance for distance2[1"
                    << "][" << nde << "][" << naz << "] is NaN!" << endl;
                }
                if( isnan(distance2[2][nde][naz]) ) {
                    cout << "Oops, the distance for distance2[2"
                    << "][" << nde << "][" << naz << "] is NaN!" << endl;
                }
            #endif

            // skip to next iteration if tested ray is on edge of ray family
            // allows extrapolation outside of ray family

            if ( a == 0 || a == num_az()-1 ) continue;
            if ( _curr->on_edge(d,a) ) continue ;

            // test to see if the center value is the smallest

            if ( nde == 2 || naz == 2 ) {
                if ( distance2[1][nde][naz] <= center ) return false ;
            } else {
                if ( distance2[1][nde][naz] < center ) return false ;
            }
            if ( distance2[2][nde][naz] <= center ) return false ;
            if ( distance2[0][nde][naz] < center ) return false ;
        }
    }
    return true ;
}

/**
 * Used by detect_eigenrays() to compute eigneray parameters and
 * add a new eigenray entry to the current target.
 */
void wave_queue::add_eigenray(
   unsigned t1, unsigned t2,
   unsigned de, unsigned az,
   double distance2[3][3][3]
) {
    #ifdef DEBUG_EIGENRAYS
        cout << "*** wave_queue::step: time=" << time() << endl ;
        wposition1 tgt( *(_curr->targets), t1, t2 ) ;
        cout << "*** wave_queue::add_eigenray:"
             << " target(" << t1 << "," << t2 << ")="
             << tgt.altitude() << "," << tgt.latitude() << "," << tgt.longitude()
             << " time=" << _time
             << " de(" << de << ")=" << (*_source_de)(de)
             << " az(" << az << ")=" << (*_source_az)(az)
             << endl ;
        cout << "\tsurface=" << _curr->surface(de,az)
             << " bottom=" << _curr->bottom(de,az)
             << " caustic=" << _curr->caustic(de,az) << endl ;
        cout << "\tdistance2:" << endl ;
        for ( unsigned n1=0 ; n1 < 3 ; ++n1 ) {
            cout << "\t    " ;
            for ( unsigned n2=0 ; n2 < 3 ; ++n2 ) {
                cout << ((n2)? "; " : "[ " ) ;
                for ( unsigned n3=0 ; n3 < 3 ; ++n3 ) {
                    cout << ((n3)? "," : "[" ) << distance2[n1][n2][n3] ;
                }
                cout << "]" ;
            }
            cout << " ]" << endl ;
        }
        cout << "\t de index: (slow) [ " << de-1 << " " << de << " " << de+1
             << " ]\n\t az index: (fast) [ " << az-1 << " " << az << " " << az+1
             << " ]" << endl;
        cout << "\t prev  [";
        for ( unsigned n2=de-1 ; n2 < de+2 ; ++n2 ) {
            cout << " [" ;
            for ( unsigned n3=az-1 ; n3 < az+2 ; ++n3 ) {
               cout << " " << _past->on_edge(n2,n3) ;
            }
            cout << " ];";
        }
        cout << " ]"  << endl;
        cout << "\t curr  [";
        for ( unsigned n2=de-1 ; n2 < de+2 ; ++n2 ) {
            cout << " [" ;
            for ( unsigned n3=az-1 ; n3 < az+2 ; ++n3 ) {
               cout << " " << _curr->on_edge(n2,n3) ;
            }
            cout << " ];";
        }
        cout << " ]"  << endl;
        cout << "\t next  [";
        for ( unsigned n2=de-1 ; n2 < de+2 ; ++n2 ) {
            cout << " [" ;
            for ( unsigned n3=az-1 ; n3 < az+2 ; ++n3 ) {
               cout << " " << _next->on_edge(n2,n3) ;
            }
            cout << " ];";
        }
        cout << " ]"  << endl;
    #endif

    // compute offsets
    // limit to simple inverse if path types change in this neighborhood

    c_vector<double,3> delta, offset, distance ;
    delta(0) = _time_step ;
    delta(1) = _source_de->increment(de) ;
    delta(2) = _source_az->increment(az) ;

    bool unstable = false ;
    const int surface = _curr->surface(de,az) ;
    const int bottom = _curr->bottom(de,az) ;
    const int caustic = _curr->caustic(de,az) ;
    for ( unsigned nde=0 ; nde < 3 && !unstable ; ++nde ) {
        unsigned d = de + nde -1 ;
        for ( unsigned naz=0 ; naz < 3 && !unstable ; ++naz ) {
            unsigned a = az + naz -1 ;
            if ( _prev->surface(d,a) != surface ||
                 _curr->surface(d,a) != surface ||
                 _next->surface(d,a) != surface ||
                 _prev->bottom(d,a) != bottom ||
                 _curr->bottom(d,a) != bottom ||
                 _next->bottom(d,a) != bottom ||
                 _prev->caustic(d,a) != caustic ||
                 _curr->caustic(d,a) != caustic ||
                 _next->caustic(d,a) != caustic )
            {
                unstable = true ;
                #ifdef DEBUG_EIGENRAYS
                cout << "\tpath change" << endl ;
                #endif
            }
        }
    }
    compute_offsets( distance2, delta, offset, distance, unstable ) ;

    // build basic eigenray products

    eigenray ray ;
    ray.time        = _time + offset(0) ;
    ray.source_de   = (*_source_de)(de) + offset(1) ;
    ray.source_az   = (*_source_az)(az) + offset(2) ;
    ray.frequencies = _frequencies ;
    ray.surface     = _curr->surface(de,az) ;
    ray.bottom      = _curr->bottom(de,az) ;
    ray.caustic     = _curr->caustic(de,az) ;
    ray.phase       = _curr->phase(de,az) ;

    // compute spreading components of intensity

    const vector<double> spread_intensity =
        _spreading_model->intensity(
            wposition1( *(_curr->targets), t1, t2 ), de, az, offset, distance );
    if ( isnan(spread_intensity(0)) ) {
        #ifdef DEBUG_EIGENRAYS
            std::cerr << "warning: wave_queue::add_eigenray()"  << endl
                      << "\tignores eigenray because intensity is NaN" << endl
                      << "\tt1=" << t1 << " t2=" << t2
                      << " de=" << de << " az=" << az << endl ;
        #endif
        return ;
    } else if ( spread_intensity(0) <= 1e-20 ) {
        #ifdef DEBUG_EIGENRAYS
            std::cerr << "warning: wave_queue::add_eigenray()" << endl
                      << "\tignores eigenray because intensity is "
                      << spread_intensity(0) << endl
                      << "\tt1=" << t1 << " t2=" << t2
                      << " de=" << de << " az=" << az << endl ;
        #endif
        return ;
    }

    ray.intensity = -10.0 * log10( spread_intensity ) ; // positive value

    // compute attenuation components of intensity

    double dt = offset(0) / _time_step ;
    if ( dt >= 0.0 ) {
        ray.intensity = ray.intensity
            + _curr->attenuation(de,az) * ( 1.0 - dt )
            + _next->attenuation(de,az) * dt ;
    } else {
        dt = 1.0 + dt ;
        ray.intensity = ray.intensity
            + _prev->attenuation(de,az) * ( 1.0 - dt )
            + _curr->attenuation(de,az) * dt ;
    }

    // estimate target D/E angle using 2nd order vector Taylor series
    // re-uses "distance2" variable to store D/E angles
    for ( unsigned nde=0 ; nde < 3 ; ++nde ) {
        for ( unsigned naz=0 ; naz < 3 ; ++naz ) {
            unsigned d = de + nde - 1 ;
            unsigned a = az + naz - 1 ;
            double dummy ;
            {
                wvector1 ndir( _prev->ndirection, d, a ) ;
                ndir.direction( &distance2[0][nde][naz], &dummy ) ;
            }
            {
                wvector1 ndir( _curr->ndirection, d, a ) ;
                ndir.direction( &distance2[1][nde][naz], &dummy ) ;
            }
            {
                wvector1 ndir( _next->ndirection, d, a ) ;
                ndir.direction( &distance2[2][nde][naz], &dummy ) ;
            }
        }
    }

    double center ;
    c_vector<double,3> gradient ;
    c_matrix<double,3,3> hessian ;
    make_taylor_coeff( distance2, delta, center, gradient, hessian, unstable ) ;
    ray.target_de = center + inner_prod( gradient, offset )
                  + 0.5 * inner_prod( offset, prod( hessian, offset ) ) ;

    // estimate target AZ angle using 2nd order vector Taylor series
    // re-uses "distance2" variable to store AZ angles

    for ( unsigned nde=0 ; nde < 3 ; ++nde ) {
        for ( unsigned naz=0 ; naz < 3 ; ++naz ) {
            unsigned d = de + nde - 1 ;
            unsigned a = az + naz - 1 ;
            double dummy ;
            {
                wvector1 ndir( _prev->ndirection, d, a ) ;
                ndir.direction( &dummy, &distance2[0][nde][naz] ) ;
            }
            {
                wvector1 ndir( _curr->ndirection, d, a ) ;
                ndir.direction( &dummy, &distance2[1][nde][naz] ) ;
            }
            {
                wvector1 ndir( _next->ndirection, d, a ) ;
                ndir.direction( &dummy, &distance2[2][nde][naz] ) ;
            }
        }
    }

    make_taylor_coeff( distance2, delta, center, gradient, hessian, unstable ) ;
    ray.target_az = center + inner_prod( gradient, offset )
                  + 0.5 * inner_prod( offset, prod( hessian, offset ) ) ;

    // accumulate eigenray list

    #ifdef DEBUG_EIGENRAYS
        cout << "\ttarget(" << t1 << "," << t2 << "):"
             << " t=" << ray.time << " de=" << ray.source_de << " az=" << ray.source_az
             << " pl=" << ray.intensity << endl ;
    #endif
    _proploss->_eigenrays(t1,t2).push_back( ray ) ;
    ++_proploss->_num_eigenrays ;
}

/**
 * Find relative offsets and true distances in time, D/E, and azimuth.
 */
void wave_queue::compute_offsets(
    const double distance2[3][3][3], const c_vector<double,3>& delta,
    c_vector<double,3>& offset, c_vector<double,3>& distance,
    bool& unstable )
{
    // compute 1st and 2nd derviatives of distance2
    // use analytic solution for the determinant of a 3x3 matrix

    double center ;
    c_vector<double,3> gradient ;
    c_matrix<double,3,3> hessian ;
    make_taylor_coeff( distance2, delta, center, gradient, hessian, unstable ) ;

    // fallback offset calculation using just diagonals
    // if inverse can not be computed because determinant is zero
    // non-positive hessian diags are an indication that offset is unstable

    for ( unsigned n=0 ; n < 3 ; ++n ) {
        const double h = max( 1e-10, hessian(n,n) ) ;
        offset(n) = -gradient(n) / h ;
    }
    if ( abs(offset(1)/delta(1)) > 0.5 ) unstable = true ;

    // compute offsets
    // solves H x = g using x = inv(H) g ;
    // uses analytic solution for the inverse of a symmetric 3x3 matrix

    const double determinant = ( unstable ) ? 0.0 :
        hessian(0,0) * ( hessian(1,1) * hessian(2,2) - hessian(1,2) * hessian(2,1) )
      + hessian(0,1) * ( hessian(1,2) * hessian(2,0) - hessian(1,0) * hessian(2,2) )
      + hessian(0,2) * ( hessian(1,0) * hessian(2,1) - hessian(2,1) * hessian(2,0) ) ;
    if ( abs(determinant) > 1e-10 ) {
        #ifdef DEBUG_EIGENRAYS
            cout << "\tfull inverse" ;
        #endif

        c_matrix<double,3,3> inverse ;
        inverse(0,0) = hessian(1,1) * hessian(2,2) - hessian(1,2) * hessian(2,1) ;
        inverse(1,0) = hessian(1,2) * hessian(2,0) - hessian(1,0) * hessian(2,2) ;
        inverse(2,0) = hessian(1,0) * hessian(2,1) - hessian(1,1) * hessian(2,0) ;
        inverse(0,1) = inverse(1,0) ;
        inverse(1,1) = hessian(0,0) * hessian(2,2) - hessian(0,2) * hessian(2,0) ;
        inverse(2,1) = hessian(2,0) * hessian(0,1) - hessian(0,0) * hessian(2,1) ;
        inverse(0,2) = inverse(2,0) ;
        inverse(1,2) = inverse(2,1) ;
        inverse(2,2) = hessian(0,0) * hessian(1,1) - hessian(0,1) * hessian(1,0) ;
        inverse /= determinant ;
        noalias(offset) = prod( inverse, -gradient ) ;

    } else {
        #ifdef DEBUG_EIGENRAYS
            if (unstable) cout << "\tsimple inv" ;
        #endif
    }

    // compute distances from offsets
    // for each coordinate, assumes the other two offsets are zero
    // fixes DE distance instablity outside of ray fan

    for ( unsigned n=0 ; n < 3 ; ++n ) {
        distance(n) = -gradient(n)*offset(n)
                -0.5*hessian(n,n)*offset(n)*offset(n) ;
    }
    if ( unstable ) {
        #ifdef DEBUG_EIGENRAYS
            if (unstable) cout << " unstable de" ;
        #endif
        distance(1) = center - distance(0) - distance(2) ;
    }

    // take sqrt() of distance and give it same sign as offset
    // clip offsets to +/- one beam

    for ( unsigned n=0 ; n < 3 ; ++n ) {
        distance(n) = sqrt( max( 0.0, distance(n) ) ) ;
        if ( offset(n) < 0.0 ) distance(n) *= -1.0 ;
        offset(n) = max( -delta(n), min(delta(n),offset(n)) ) ;
    }

    #ifdef DEBUG_EIGENRAYS
        cout << " gradient: " << gradient(0) << "," << gradient[1] << "," << gradient[2]
             << " curvature:  " << hessian(0,0) << "," << hessian(1,1) << "," << hessian(2,2) << endl
             << "\toffset: " << offset << " distance: " << distance << endl ;
    #endif
//    // used to build spreadsheets of offset and distance
//    cout << offset(0) << "," << offset(1) << "," << offset(2) << ","
//         << distance(0) << "," << distance(1) << "," << distance(2) << ","
//         << (distance(0)*distance(0)+distance(1)*distance(1)+distance(2)*distance(2)) << ","
//         << center << ","
//         << gradient(0) << "," << gradient[1] << "," << gradient[2] << ","
//         << hessian(0,0) << "," << hessian(1,1) << "," << hessian(2,2) << endl ;
}

/**
 * Computes the Taylor coefficients used to compute eigenrays.
 */
void wave_queue::make_taylor_coeff(
    const double value[3][3][3], const c_vector<double,3>& delta,
    double& center, c_vector<double,3>& gradient, c_matrix<double,3,3>& hessian,
    bool diagonal_only
) {
    const double d0 = 2.0 * delta(0) ;
    const double d1 = 2.0 * delta(1) ;
    const double d2 = 2.0 * delta(2) ;

    // find value at the center point

    center = value[1][1][1] ;

    // compute diagonal terms in Hessian matrix

    hessian.clear() ;
    hessian(0,0) = ( value[2][1][1] + value[0][1][1] - 2.0 * center )
                 / ( delta(0) * delta(0) ) ;
    hessian(1,1) = ( value[1][2][1] + value[1][0][1] - 2.0 * center )
                 / ( delta(1) * delta(1) ) ;
    hessian(2,2) = ( value[1][1][2] + value[1][1][0] - 2.0 * center )
                 / ( delta(2) * delta(2) ) ;

    // compute off-diagonal terms in Hessian matrix, unless told not to

    if ( ! diagonal_only ) {
        gradient(0) = ( value[2][0][1] - value[0][0][1] ) / d0 ;
        gradient(2) = ( value[2][2][1] - value[0][2][1] ) / d0 ;
        hessian(0,1) = ( gradient(2) - gradient(0) )      / d1 ;
        hessian(1,0) = hessian(0,1) ;

        gradient(0) = ( value[2][1][0] - value[0][1][0] ) / d0 ;
        gradient(2) = ( value[2][1][2] - value[0][1][2] ) / d0 ;
        hessian(0,2) = ( gradient(2) - gradient(0) )      / d2 ;
        hessian(2,0) = hessian(0,2) ;

        gradient(0) = ( value[1][2][0] - value[1][0][0] ) / d1 ;
        gradient(2) = ( value[1][2][2] - value[1][0][2] ) / d1 ;
        hessian(1,2) = ( gradient(2) - gradient(0) )      / d2 ;
        hessian(2,1) = hessian(1,2) ;
    }

    // compute gradient near center point

    gradient(0) = ( value[2][1][1] - value[0][1][1] ) ;
    gradient(1) = ( value[1][2][1] - value[1][0][1] ) ;
    gradient(2) = ( value[1][1][2] - value[1][1][0] ) ;
    gradient(0) = ( value[2][1][1] - value[0][1][1] ) / d0 ;
    gradient(1) = ( value[1][2][1] - value[1][0][1] ) / d1 ;
    gradient(2) = ( value[1][1][2] - value[1][1][0] ) / d2 ;

}
