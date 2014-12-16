/**
 * @file wave_queue.cc
 * Wavefront propagation as a function of time.
 */
#include <usml/waveq3d/wave_queue.h>
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
wave_queue::wave_queue(
    ocean_model& ocean,
    const seq_vector& freq,
    const wposition1& pos,
    const seq_vector& de,
    const seq_vector& az,
    double time_step,
    const wposition* targets,
    const unsigned long run_id,
    spreading_type type
) :
    _ocean( ocean ),
    _frequencies( freq.clone() ),
    _source_pos( pos ),
    _source_de( de.clone() ),
    _source_az( az.clone() ),
    _time_step( time_step ),
    _time( 0.0 ),
    _targets( targets ),
    _run_id(run_id),
    _nc_file( NULL )
{

	// create references between targets and wavefront objects.
    const matrix<double>* pTargets_sin_theta = NULL ;
    double az_first = abs((*_source_az)(0)) ;
    double az_last = abs((*_source_az)(_source_az->size()-1)) ;
    double boundary_check = az_first + az_last ;
    if ( boundary_check == 360.0 &&
        ( fmod(az_first, 360.0) == fmod(az_last, 360.0) ) ) { _az_boundary = true ; }
    else { _az_boundary = false ;}
    _intensity_threshold = 300.0; //In dB

    if ( _targets ) {
    	_targets_sin_theta = sin( _targets->theta() ) ;
    	pTargets_sin_theta = &_targets_sin_theta;
    }

    // create storage space for all wavefront elements

    _past = new wave_front( _ocean, _frequencies, de.size(), az.size(), _targets, pTargets_sin_theta ) ;
    _prev = new wave_front( _ocean, _frequencies, de.size(), az.size(), _targets, pTargets_sin_theta ) ;
    _curr = new wave_front( _ocean, _frequencies, de.size(), az.size(), _targets, pTargets_sin_theta ) ;
    _next = new wave_front( _ocean, _frequencies, de.size(), az.size(), _targets, pTargets_sin_theta ) ;

    // initialize wave front elements

    _curr->init_wave( pos, de, az ) ;
    _curr->update() ;
    init_wavefronts() ;
    _reflection_model = new reflection_model( *this ) ;
    _spreading_model = NULL ;
    if ( _targets ) {
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
    _next->path_length = _next->distance + _curr->path_length ;
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

    // compute position, direction, and environment parameters for next entry

    ode_integ::ab3_pos(  _time_step, _past, _prev, _curr, _next ) ;
    ode_integ::ab3_ndir( _time_step, _past, _prev, _curr, _next ) ;

    _next->update() ;
    _next->path_length = _next->distance + _curr->path_length ;

    _next->attenuation += _curr->attenuation ;
    _next->phase += _curr->phase ;
    _next->surface = _curr->surface ;
    _next->bottom = _curr->bottom ;
    _next->upper = _curr->upper ;
    _next->lower = _curr->lower ;
    _next->caustic = _curr->caustic ;

    // search for eigenray collisions with acoustic targets

    detect_eigenrays() ;
}

/**
 * Detect and process boundary reflections and caustics.
 */
void wave_queue::detect_reflections() {

    // process all surface and bottom reflections, and vertices
    // note that multiple rays can reflect in the same time step

    for (unsigned de = 0; de < num_de(); ++de) {
        for (unsigned az = 0; az < num_az(); ++az) {
            if ( !detect_reflections_surface(de,az) ) {
                if( !detect_reflections_bottom(de,az) ) {
                    detect_vertices(de,az) ;
                    detect_caustics(de,az) ;
                }
            }
        }
    }

    // search for other changes in wavefront

    _next->find_edges() ;
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
    _ocean.bottom().height( pos, &height, NULL, true ) ;
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
 *  Detects upper and lower vertices along the wavefront
 */
void wave_queue::detect_vertices( unsigned de, unsigned az ) {
    double L = _curr->ndirection.rho(de,az) ;
    double R = _next->ndirection.rho(de,az) ;
    if( L*R < 0.0 && R < 0.0 ) _next->upper(de,az)++ ;
    else if( L*R < 0.0 && 0.0 < R ) _next->lower(de,az)++ ;
}

/**
 *  Detects and processes the caustics along the next wavefront
 */
void wave_queue::detect_caustics( unsigned de, unsigned az ) {
    const unsigned max_de = num_de() - 1 ;
    if ( de < max_de ) {
        double A = _curr->position.rho(de+1,az) ;
        double B = _curr->position.rho(de,az) ;
        double C = _next->position.rho(de+1,az) ;
        double D = _next->position.rho(de,az) ;
        bool fold = false ;
        if ( (_next->surface(de+1,az) == _next->surface(de,az)) &&
             (_next->bottom(de+1,az) == _next->bottom(de,az)) ) { fold = true; }
        if ( (C-D)*(A-B) < 0 && fold ) {
            _next->caustic(de+1,az)++;
            for (unsigned f = 0; f < _frequencies->size(); ++f) {
                _next->phase(de+1,az)(f) -= M_PI_2;
            }
        }
    }
}

/**
 * Detect and process wavefront closest point of approach (CPA) with target.
 */
void wave_queue::detect_eigenrays() {
    if ( _targets == NULL ) return ;

    double distance2[3][3][3] ;
    double& center = distance2[1][1][1] ;
    double az_start = 0 ;

    if(!_az_boundary) {
        az_start = 1 ;
    }

    // loop over all targets
    for ( unsigned t1=0 ; t1 < _targets->size1() ; ++t1 ) {
        for ( unsigned t2=0 ; t2 < _targets->size2() ; ++t2 ) {
            _de_branch = false ;
            if ( abs(_source_pos.latitude() - _targets->latitude(t1,t2)) < 1e-4 &&
                 abs(_source_pos.longitude() - _targets->longitude(t1,t2)) < 1e-4 ) {
                _de_branch = true ;
            }

            // Loop over all rays
            for ( unsigned de=1 ; de < num_de() - 1 ; ++de ) {
                for ( unsigned az=az_start ; az < num_az() - 1 ; ++az ) {

                    // *******************************************
                    // When central ray is at the edge of ray family
                    // it prevents edges from acting as CPA, if so, go to next de/az
                    // Also check to see if this ray is a duplicate.

                    if ( _curr->on_edge(de,az) ) { continue; }

                    // get the central ray for testing
                    center = _curr->distance2(t1,t2)(de,az) ;

                    distance2[2][1][1] = _next->distance2(t1,t2)(de,az) ;
                    if ( distance2[2][1][1] <= center ) {
                        continue;
                    }

                    distance2[0][1][1] = _prev->distance2(t1,t2)(de,az) ;
                    if ( distance2[0][1][1] < center ) {
                        continue;
                    }

                    // *******************************************
                    if ( is_closest_ray(t1,t2,de,az,center,distance2) ) {
                        build_eigenray(t1,t2,de,az,distance2) ;
                    }
                }   // end az loop
            }   // end de loop
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
   const double& center,
   double distance2[3][3][3]
) {
    /**
     * In order to speed up the code, it required splitting the code
     * to run faster. This then only checks the _az_boundary condition
     * once per function call.
     */
    if(_az_boundary) {
        // test all neighbors that are not the central ray
        for ( unsigned nde=0 ; nde < 3 ; ++nde ) {
            for ( unsigned naz=0 ; naz < 3 ; ++naz ) {
                if ( nde == 1 && naz == 1 ) continue ;

                // compute distances on the current, next, and previous wavefronts

                unsigned d = de + nde - 1 ;
                unsigned a = az + naz - 1 ;
                if( (int)a < 0.0 ) {
                    a = num_az() - 2 ;
                } else
                if( a >= (num_az() - 1) ) {
                    a = 0 ;
                }

                distance2[0][nde][naz] = _prev->distance2(t1,t2)(d,a) ;
                distance2[1][nde][naz] = _curr->distance2(t1,t2)(d,a) ;
                distance2[2][nde][naz] = _next->distance2(t1,t2)(d,a) ;

                // skip to next iteration if tested ray is on edge of ray family
                // allows extrapolation outside of ray family

                if ( a == num_az()-1 ) continue;
                if ( _de_branch ) {
                    if ( _curr->on_edge(d,a) ) continue ;
                } else {
                    if ( nde != 1 ) {
                        if ( _curr->on_edge(d,a) ) continue ;
                    }
                }

                // test to see if the center value is the smallest

                if ( nde == 2 || naz == 2 ) {
                    if ( _de_branch ) {
                        if ( az == 0 ) {
                            if ( distance2[1][nde][naz] < center ) return false ;
                        } else { return false ; }
                    } else {
                        if ( distance2[1][nde][naz] <= center ) return false ;
                    }
                } else {
                    if ( distance2[1][nde][naz] < center ) return false ;
                }
                if ( distance2[2][nde][naz] <= center ) return false ;
                if ( distance2[0][nde][naz] < center ) return false ;
            }
        }
        return true ;
    } else {
        for ( unsigned nde=0 ; nde < 3 ; ++nde ) {
            for ( unsigned naz=0 ; naz < 3 ; ++naz ) {
                if ( nde == 1 && naz == 1 ) continue ;
                // compute distances on the current, next, and previous wavefronts

                unsigned d = de + nde - 1 ;
                unsigned a = az + naz - 1 ;

                distance2[0][nde][naz] = _prev->distance2(t1,t2)(d,a) ;
                distance2[1][nde][naz] = _curr->distance2(t1,t2)(d,a) ;
                distance2[2][nde][naz] = _next->distance2(t1,t2)(d,a) ;

                // skip to next iteration if tested ray is on edge of ray family
                // allows extrapolation outside of ray family

                if ( a == 0 || a == num_az()-1 ) continue;
                if ( _de_branch ) {
                    if ( _curr->on_edge(d,a) ) continue ;
                } else {
                    if ( nde != 1 ) {
                        if ( _curr->on_edge(d,a) ) continue ;
                    }
                }

                // test to see if the center value is the smallest

                if ( nde == 2 || naz == 2 ) {
                    if ( _de_branch ) {
                        if ( az == 0 ) {
                            if ( distance2[1][nde][naz] < center ) return false ;
                        } else { return false ; }
                    } else {
                        if ( distance2[1][nde][naz] <= center ) return false ;
                    }
                } else {
                    if ( distance2[1][nde][naz] < center ) return false ;
                }
                if ( distance2[2][nde][naz] <= center ) return false ;
                if ( distance2[0][nde][naz] < center ) return false ;
            }
        }
        return true ;
    } // end if _az_boundary

} // end is_closest_ray

/**
 * Used by detect_eigenrays() to compute eigenray parameters and
 * add a new eigenray entry to the current target.
 */
void wave_queue::build_eigenray(
   unsigned t1, unsigned t2,
   unsigned de, unsigned az,
   double distance2[3][3][3]
) {
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
    /**
     * In order to speed up the code, it required splitting the code
     * to run faster. This then only checks the _az_boundary condition
     * once per function call.
     */
    if(_az_boundary) {
        for ( unsigned nde=0 ; nde < 3 && !unstable ; ++nde ) {
            unsigned d = de + nde -1 ;
            for ( unsigned naz=0 ; naz < 3 && !unstable ; ++naz ) {
                unsigned a = az + naz -1 ;
                if( (int)a < 0.0 ) {
                    a = num_az() - 2 ;
                } else
                if( a >= (num_az() - 1) ) {
                    a = 0 ;
                }
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
                }
            }
        }
    }
    else {
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
                }
            }
        }
    } // end if _az_boundary

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
    for ( unsigned int i = 0; i < ray.intensity.size(); ++i) {
        if ( isnan(spread_intensity(i)) ) {
            #ifdef USML_DEBUG
                std::cerr << "warning: wave_queue::build_eigenray()"  << endl
                          << "\tignores eigenray because intensity is NaN" << endl
                          << "\tt1=" << t1 << " t2=" << t2
                          << " de=" << de << " az=" << az << endl ;
            #endif
            return ;
        }
    }

    ray.intensity = -10.0 * log10( max(spread_intensity,1e-30) ) ; // positive value

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

    // Determine if intensity is weaker than the _intensity_threshold.
    // Note ray.intensity is a positive value.
    // Thus if ray.intensity at any frequency is less than the _intensity_threshold
    // complete the ray build and send to listeners; discard otherwise.

    bool bKeepRay = false ;
    for ( unsigned int i = 0; i < ray.intensity.size(); ++i) {
		if ( ray.intensity(i) < _intensity_threshold  ) {
			bKeepRay = true ;
			break ;
		}
	}

    if (!bKeepRay) {
		return ;
	}

    // estimate target D/E angle using 2nd order vector Taylor series
    // re-uses "distance2" variable to store D/E angles
    /**
     * In order to speed up the code, it required splitting the code
     * to run faster. This then only checks the _az_boundary condition
     * once per function call.
     */
    if(_az_boundary) {
        for ( unsigned nde=0 ; nde < 3 ; ++nde ) {
            for ( unsigned naz=0 ; naz < 3 ; ++naz ) {
                unsigned d = de + nde - 1 ;
                unsigned a = az + naz - 1 ;
                if( (int)a < 0.0 ) {
                    a = num_az() - 2 ;
                } else
                if( a >= (num_az() - 1) ) {
                    a = 0 ;
                }
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
    } else {
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
    } // end if _az_boundary

    double center ;
    c_vector<double,3> gradient ;
    c_matrix<double,3,3> hessian ;
    make_taylor_coeff( distance2, delta, center, gradient, hessian, unstable ) ;
    ray.target_de = center + inner_prod( gradient, offset )
                  + 0.5 * inner_prod( offset, prod( hessian, offset ) ) ;

    // estimate target AZ angle using 2nd order vector Taylor series
    // re-uses "distance2" variable to store AZ angles

    if(_az_boundary) {
        for ( unsigned nde=0 ; nde < 3 ; ++nde ) {
            for ( unsigned naz=0 ; naz < 3 ; ++naz ) {
                unsigned d = de + nde - 1 ;
                unsigned a = az + naz - 1 ;
                double dummy ;
                if( (int)a < 0.0 ) {
                    a = num_az() - 2 ;
                } else
                if( (int)a >= (num_az() - 1) ) {
                    a = 0 ;
                }
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
    } else {
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
    }// end if _az_boundary

    make_taylor_coeff( distance2, delta, center, gradient, hessian, unstable ) ;
    ray.target_az = center + inner_prod( gradient, offset )
                  + 0.5 * inner_prod( offset, prod( hessian, offset ) ) ;

    // Add eigenray to those objects which requested them
    notifyEigenrayListeners(t1,t2,ray);

}

/**
 * Find relative offsets and true distances in time, D/E, and azimuth.
 */
void wave_queue::compute_offsets(
    const double distance2[3][3][3], const c_vector<double,3>& delta,
    c_vector<double,3>& offset, c_vector<double,3>& distance,
    bool& unstable )
{
    // compute 1st and 2nd derivatives of distance2
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

    }

    // compute distances from offsets
    // for each coordinate, assumes the other two offsets are zero
    // fixes DE distance instablity outside of ray fan
    for ( unsigned n=0 ; n < 3 ; ++n ) {
        distance(n) = -gradient(n)*offset(n)
                -0.5*hessian(n,n)*offset(n)*offset(n) ;
    }
    if ( unstable ) {
        distance(1) = center - distance(0) - distance(2) ;
    }

    // take sqrt() of distance and give it same sign as offset
    // clip offsets to +/- one beam

    for ( unsigned n=0 ; n < 3 ; ++n ) {
        distance(n) = sqrt( max( 0.0, distance(n) ) ) ;
        if ( offset(n) < 0.0 ) distance(n) *= -1.0 ;
        offset(n) = max( -delta(n), min(delta(n),offset(n)) ) ;
    }
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

/**
* Add a eigenrayListener to the _eigenrayListenerVec vector
*/
bool wave_queue::addEigenrayListener(eigenrayListener* pListener) {

	std::vector<eigenrayListener*>::iterator iter = find(_eigenrayListenerVec.begin(), _eigenrayListenerVec.end(), pListener);
	if ( iter != _eigenrayListenerVec.end() ) {
		return false;
	}
	_eigenrayListenerVec.push_back(pListener);
	return true;
}


/**
 * Remove a eigenrayListener from the _eigenrayListenerVec vector
 */
bool wave_queue::removeEigenrayListener(eigenrayListener* pListener){

	std::vector<eigenrayListener*>::iterator iter = find(_eigenrayListenerVec.begin(), _eigenrayListenerVec.end(), pListener);
	if ( iter == _eigenrayListenerVec.end() ){
		return false;
	} else {
		_eigenrayListenerVec.erase(remove(_eigenrayListenerVec.begin(), _eigenrayListenerVec.end(), pListener));
	}
	return true;
}

/**
 * For each eigenrayListener in the _eigenrayListenerVec vector
 * call the addEigenray method to provide eigenrays.
 */
bool wave_queue::notifyEigenrayListeners(unsigned targetRow, unsigned targetCol, eigenray pEigenray){

	for (std::vector<eigenrayListener*>::iterator iter = _eigenrayListenerVec.begin();
												iter != _eigenrayListenerVec.end(); ++iter){
		eigenrayListener* pListener = *iter;
		pListener->addEigenray(targetRow, targetCol, pEigenray, _run_id);
	}

	return (_eigenrayListenerVec.size() > 0);
}
