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
    const size_t run_id,
    spreading_type type
) :
    _ocean( ocean ),
    _frequencies( freq.clone() ),
    _source_pos( pos ),
    _source_de( de.clone() ),
    _max_de( de.size()-1 ),
    _source_az( az.clone() ),
    _max_az( az.size()-1 ),
    _time_step( time_step ),
    _time( 0.0 ),
    _targets( targets ),
    _run_id(run_id),
    _nc_file( NULL )
{
    const double az_first = abs((*_source_az)(0)) ;
    const double az_last = abs((*_source_az)(_source_az->size()-1)) ;
    _az_boundary = ( fmod(az_first+360.0, 360.0) == fmod(az_last+360.0, 360.0) ) ;

    _intensity_threshold = 300.0 ; //In dB
    if ( _targets ) {
        _targets_sin_theta = sin( _targets->theta() ) ;
    }

    // check for sources outside of the water column

    const double offset = 0.1 ;
    double bottom_rho = 0.0 ;
    _ocean.bottom().height( _source_pos, &bottom_rho ) ;
    bottom_rho += offset ;
    if ( _source_pos.altitude() > -offset ) {
        _source_pos.altitude(-offset) ;
    } else if ( _source_pos.rho() < bottom_rho ) {
        _source_pos.rho( bottom_rho ) ;
    }

    // create storage space for all wavefront elements

    _past = new wave_front( _ocean, _frequencies, de.size(), az.size(), _targets, &_targets_sin_theta ) ;
    _prev = new wave_front( _ocean, _frequencies, de.size(), az.size(), _targets, &_targets_sin_theta ) ;
    _curr = new wave_front( _ocean, _frequencies, de.size(), az.size(), _targets, &_targets_sin_theta ) ;
    _next = new wave_front( _ocean, _frequencies, de.size(), az.size(), _targets, &_targets_sin_theta ) ;

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
 * Assigns an eigeverb_collection to the reflection model
 */
void wave_queue::add_eigenverb_listener(
    eigenverb_collection* collection )
{
    if( _reflection_model->_collection ) delete _reflection_model->_collection ;
    _reflection_model->_collection = collection ;
}

/**
 * Determines if the ray is a valid candidate to produce an eigenverb
 */
bool wave_queue::is_ray_valid( size_t de, size_t az ) {
    if( _reflection_model->_collection ) {
        if( de != _max_de )
            if( az != _max_az )
                if( _time > 0 )
                    return true ;
    }
    return false ;
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

    for (size_t de = 0; de < num_de(); ++de) {
        for (size_t az = 0; az < num_az(); ++az) {
            if ( !detect_reflections_surface(de,az) ) {
                if( !detect_reflections_bottom(de,az) ) {
                    detect_vertices(de,az) ;
                    detect_caustics(de,az) ;
                }
            }
        }
    }

    // Check for volume interactions if we need to and there are
    // volume layers in the ocean model.
    if ( _reflection_model->_collection && _ocean.num_volume() ) {
        detect_volume_reflections() ;
    }

    // search for other changes in wavefront
    _next->find_edges() ;
}

/**
 * Detect and process reflection for a single (DE,AZ) combination.
 */
bool wave_queue::detect_reflections_surface( size_t de, size_t az ) {
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
bool wave_queue::detect_reflections_bottom( size_t de, size_t az ) {
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
            return true ;    // indicate a surface reflection
        }
    }
    return false ;    // indicates no reflection
}

/**
 *  Detects upper and lower vertices along the wavefront
 */
void wave_queue::detect_vertices( size_t de, size_t az ) {
    double L = _curr->ndirection.rho(de,az) ;
    double R = _next->ndirection.rho(de,az) ;
    if( L*R < 0.0 && R < 0.0 ) _next->upper(de,az)++ ;
    else if( L*R < 0.0 && 0.0 < R ) _next->lower(de,az)++ ;
}

/**
 *  Detects and processes the caustics along the next wavefront
 */
void wave_queue::detect_caustics( size_t de, size_t az ) {
    if ( de < _max_de ) {
        double A = _curr->position.rho(de+1,az) ;
        double B = _curr->position.rho(de,az) ;
        double C = _next->position.rho(de+1,az) ;
        double D = _next->position.rho(de,az) ;
        bool fold = false ;
        if ( (_next->surface(de+1,az) == _next->surface(de,az)) &&
             (_next->bottom(de+1,az) == _next->bottom(de,az)) ) { fold = true; }
        if ( (C-D)*(A-B) < 0 && fold ) {
            _next->caustic(de+1,az)++;
            for (size_t f = 0; f < _frequencies->size(); ++f) {
                _next->phase(de+1,az)(f) -= M_PI_2;
            }
        }
    }
}

/**
 * Detect volume boundary reflections for reverberation contributions
 */
void wave_queue::detect_volume_reflections() {
    std::size_t n = _ocean.num_volume() ;
    for(std::size_t i=0; i<n; ++i) {
        volume_model& layer = _ocean.volume(i) ;
        for (size_t de = 0; de < num_de(); ++de) {
            for (size_t az = 0; az < num_az(); ++az) {
                if( _curr->on_edge(de,az) ) continue ;
                double height ;
                wposition1 pos_curr( _curr->position, de, az ) ;
                wposition1 pos_next( _next->position, de, az ) ;
                layer.depth( pos_next, &height, NULL ) ;
                double d1 = height - pos_next.rho() ;
                double d2 = height - pos_curr.rho() ;
                if ( d1 > 0 && d2 < 0 ) { collide_from_above(de,az, d1, i) ; }
                else if ( d1 < 0 && d2 > 0 ) { collide_from_below(de,az, d2, i) ; }
            }
        }
    }
}

void wave_queue::collide_from_above(
        size_t de, size_t az, double depth, size_t layer )
{
    if( is_ray_valid(de,az) ) {
        double MIN_REFLECT = 6.0 ;
        wposition1 position( _curr->position, de, az ) ;
        wvector1 ndirection( _curr->ndirection, de, az ) ;
        double c = _curr->sound_speed( de, az ) ;
        double c2 = c*c ;

        double layer_rho ;
        wvector1 layer_normal( 1.0, 0.0, 0.0 ) ;
        volume_model& volume = _ocean.volume(layer) ;
        volume.depth( position, &layer_rho ) ;
        double height_water = position.rho() - layer_rho ;

        ndirection.rho(   c2 * ndirection.rho() ) ;
        ndirection.theta( c2 * ndirection.theta() ) ;
        ndirection.phi(   c2 * ndirection.phi() ) ;
        double dot_full = layer_normal.rho() * ndirection.rho()
                        + layer_normal.theta() * ndirection.theta()
                        + layer_normal.phi() * ndirection.phi() ;

        double max_dot = - max( MIN_REFLECT, (height_water+depth)*layer_normal.rho() ) ;
        if ( dot_full >= max_dot ) dot_full = max_dot ;

        const double dot_water = -height_water * layer_normal.rho() ;
        double time_water = max( 0.0, dot_water / dot_full ) ;

        collision_location( de, az, time_water, &position, &ndirection, &c ) ;
        volume.depth( position, &layer_rho ) ;
        c2 = c*c ;
        height_water = position.rho() - layer_rho ;

        ndirection.rho(   c2 * ndirection.rho() ) ;
        ndirection.theta( c2 * ndirection.theta() ) ;
        ndirection.phi(   c2 * ndirection.phi() ) ;
        dot_full = layer_normal.rho() * ndirection.rho()
            + layer_normal.theta() * ndirection.theta()
            + layer_normal.phi() * ndirection.phi() ;  // negative #
        max_dot = - max( MIN_REFLECT, (height_water+depth)*layer_normal.rho() ) ;
        if( dot_full >= max_dot )
            dot_full = max_dot ;

        double grazing = 0.0 ;
        if( dot_full / c >= 1.0 )
            grazing = -M_PI_2 ;
        else if( dot_full / c <= -1.0 )
            grazing = M_PI_2 ;
        else
            grazing = asin( -dot_full / c ) ;

        // build the eigenverb
        _reflection_model->build_eigenverb( de, az, time_water,
            grazing, c, position, ndirection, VOLUME_LOWER ) ;
    }
}

/** @todo correct logic/signs for collisions from below the boundary **/
void wave_queue::collide_from_below(
        size_t de, size_t az, double depth, size_t layer )
{
    if( is_ray_valid(de,az) ) {
        double MIN_REFLECT = 6.0 ;
        wposition1 position( _curr->position, de, az ) ;
        wvector1 ndirection( _curr->ndirection, de, az ) ;
        double c = _curr->sound_speed( de, az ) ;
        double c2 = c*c ;
        double layer_rho ;
        wvector1 layer_normal( -1.0, 0.0, 0.0 ) ;
        volume_model& volume = _ocean.volume(layer) ;
        volume.depth( position, &layer_rho ) ;
        double height_water = position.rho() - layer_rho ;

        ndirection.rho(   c2 * ndirection.rho() ) ;
        ndirection.theta( c2 * ndirection.theta() ) ;
        ndirection.phi(   c2 * ndirection.phi() ) ;
        double dot_full = layer_normal.rho() * ndirection.rho()
                        + layer_normal.theta() * ndirection.theta()
                        + layer_normal.phi() * ndirection.phi() ;

        double max_dot = - max( MIN_REFLECT, (height_water+depth)*layer_normal.rho() ) ;
        if( dot_full >= max_dot )
            dot_full = max_dot ;

        const double dot_water = -height_water * layer_normal.rho() ;
        double time_water = max( 0.0, dot_water / dot_full ) ;

        collision_location( de, az, time_water, &position, &ndirection, &c ) ;
        volume.depth( position, &layer_rho ) ;
        c2 = c*c ;
        height_water = position.rho() - layer_rho ;

        ndirection.rho(   c2 * ndirection.rho() ) ;
        ndirection.theta( c2 * ndirection.theta() ) ;
        ndirection.phi(   c2 * ndirection.phi() ) ;
        dot_full = layer_normal.rho() * ndirection.rho()
            + layer_normal.theta() * ndirection.theta()
            + layer_normal.phi() * ndirection.phi() ;  // negative #
        max_dot = - max( MIN_REFLECT, (height_water+depth)*layer_normal.rho() ) ;
        if( dot_full >= max_dot )
            dot_full = max_dot ;

        double grazing = 0.0 ;
        if( dot_full / c >= 1.0 )
            grazing = -M_PI_2 ;
        else if( dot_full / c <= -1.0 )
            grazing = M_PI_2 ;
        else
            grazing = asin( -dot_full / c ) ;

        // build the eigenverb
        _reflection_model->build_eigenverb( de, az, time_water,
            grazing, c, position, ndirection, VOLUME_UPPER ) ;
    }
}

/**
 * Detect and process wavefront closest point of approach (CPA) with target.
 */
void wave_queue::detect_eigenrays() {
    if ( _targets == NULL ) return ;

    double distance2[3][3][3] ;
    double& center = distance2[1][1][1] ;
    double az_start = (_az_boundary) ? 0 : 1 ;

    // loop over all targets
    for ( size_t t1=0 ; t1 < _targets->size1() ; ++t1 ) {
        for ( size_t t2=0 ; t2 < _targets->size2() ; ++t2 ) {
            _de_branch = false ;
            if ( abs(_source_pos.latitude() - _targets->latitude(t1,t2)) < 1e-4 &&
                 abs(_source_pos.longitude() - _targets->longitude(t1,t2)) < 1e-4 ) {
                _de_branch = true ;
            }

            // Loop over all rays
            for ( size_t de=1 ; de < _max_de ; ++de ) {
                for ( size_t az=az_start ; az < _max_az ; ++az ) {

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
   size_t t1, size_t t2,
   size_t de, size_t az,
   const double& center,
   double distance2[3][3][3]
) {
	// test all neighbors that are not the central ray

	for ( size_t nde=0 ; nde < 3 ; ++nde ) {
		for ( size_t naz=0 ; naz < 3 ; ++naz ) {
			if ( nde == 1 && naz == 1 ) continue ;

			// compute distances on the current, next, and previous wavefronts

			size_t d = de + nde - 1 ;
			size_t a = az + naz - 1 ;
			if ( _az_boundary ) {
				if ( az + naz == 0 ) {	// aka if a < 0
					a = num_az() - 2 ;
				} else if( a >= _max_az ) {
					a = 0 ;
				}
			}

			distance2[0][nde][naz] = _prev->distance2(t1,t2)(d,a) ;
			distance2[1][nde][naz] = _curr->distance2(t1,t2)(d,a) ;
			distance2[2][nde][naz] = _next->distance2(t1,t2)(d,a) ;

			// skip to next iteration if tested ray is on edge of ray family
			// allows extrapolation outside of ray family

			if ( a == 0 && ! _az_boundary ) continue;
			if ( a == _max_az ) continue;
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
} // end is_closest_ray

/**
 * Used by detect_eigenrays() to compute eigenray parameters and
 * add a new eigenray entry to the current target.
 */
void wave_queue::build_eigenray(
   size_t t1, size_t t2,
   size_t de, size_t az,
   double distance2[3][3][3] )
{
    // compute offsets
    // limit to simple inverse if path types change in this neighborhood

    c_vector<double, 3> delta, offset, distance;
    delta(0) = _time_step;
    delta(1) = _source_de->increment(de);
    delta(2) = _source_az->increment(az);

    bool unstable = false;
    const int surface = _curr->surface(de, az);
    const int bottom = _curr->bottom(de, az);
    const int caustic = _curr->caustic(de, az);

    for (size_t nde = 0; nde < 3 && !unstable; ++nde) {
        size_t d = de + nde - 1 ;
        for (size_t naz = 0; naz < 3 && !unstable; ++naz) {
            size_t a = az + naz - 1 ;
            if (_az_boundary) {
                if ( az + naz == 0 ) {	// aka if a < 0
                    a = _max_az-1 ;
                } else if (a >= _max_az) {
                    a = 0 ;
                }
            }
            if ( _prev->surface(d, a) != surface
              || _curr->surface(d, a) != surface
              || _next->surface(d, a) != surface
              || _prev->bottom(d, a) != bottom
              || _curr->bottom(d, a) != bottom
              || _next->bottom(d, a) != bottom
              || _prev->caustic(d, a) != caustic
              || _curr->caustic(d, a) != caustic
              || _next->caustic(d, a) != caustic)
            {
                unstable = true;
            }
        }
    }

    compute_offsets(distance2, delta, offset, distance, unstable);

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
    for ( size_t i = 0; i < ray.intensity.size(); ++i) {
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
    for ( size_t i = 0; i < ray.intensity.size(); ++i) {
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

    for ( size_t nde=0 ; nde < 3 ; ++nde ) {
    	for ( size_t naz=0 ; naz < 3 ; ++naz ) {
    		size_t d = de + nde - 1 ;
    		size_t a = az + naz - 1 ;
            if (_az_boundary) {
                if ( az + naz == 0 ) {	// aka if a < 0
                    a = _max_az - 1 ;
                } else if (a >= _max_az) {
                    a = 0 ;
                }
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

    double center ;
    c_vector<double,3> gradient ;
    c_matrix<double,3,3> hessian ;
    make_taylor_coeff( distance2, delta, center, gradient, hessian, unstable ) ;
    ray.target_de = center + inner_prod( gradient, offset )
                  + 0.5 * inner_prod( offset, prod( hessian, offset ) ) ;

    // estimate target AZ angle using 2nd order vector Taylor series
    // re-uses "distance2" variable to store AZ angles
	for ( size_t nde=0 ; nde < 3 ; ++nde ) {
		for ( size_t naz=0 ; naz < 3 ; ++naz ) {
			size_t d = de + nde - 1 ;
			size_t a = az + naz - 1 ;
			if (_az_boundary) {
				if ( az + naz == 0 ) {	// aka if a < 0
					a = _max_az - 1 ;
				} else if (a >= _max_az) {
					a = 0 ;
				}
			}
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

    for ( size_t n=0 ; n < 3 ; ++n ) {
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
    for ( size_t n=0 ; n < 3 ; ++n ) {
        distance(n) = -gradient(n)*offset(n)
                -0.5*hessian(n,n)*offset(n)*offset(n) ;
    }
    if ( unstable ) {
        distance(1) = center - distance(0) - distance(2) ;
    }

    // take sqrt() of distance and give it same sign as offset
    // clip offsets to +/- one beam

    for ( size_t n=0 ; n < 3 ; ++n ) {
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
 * Compute the precise location and direction at the point of collision.
 */
void wave_queue::collision_location(
    size_t de, size_t az, double time_water,
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
bool wave_queue::notifyEigenrayListeners(size_t targetRow, size_t targetCol, eigenray pEigenray){

    for (std::vector<eigenrayListener*>::iterator iter = _eigenrayListenerVec.begin();
                                                iter != _eigenrayListenerVec.end(); ++iter){
        eigenrayListener* pListener = *iter;
        pListener->addEigenray(targetRow, targetCol, pEigenray, _run_id);
    }

    return (_eigenrayListenerVec.size() > 0);
}

/**
 * For each eigenrayListener in the _eigenrayListenerVec vector
 * call the checkEigenrays method to deliver all eigenrays after
 * a certain amount of time has passed.
 */
bool wave_queue::checkEigenrayListeners(long waveTime){

    for (std::vector<eigenrayListener*>::iterator iter = _eigenrayListenerVec.begin();
                                                iter != _eigenrayListenerVec.end(); ++iter){
        eigenrayListener* pListener = *iter;
        pListener->checkEigenrays(_run_id, waveTime);
    }

    return (_eigenrayListenerVec.size() > 0);
}
