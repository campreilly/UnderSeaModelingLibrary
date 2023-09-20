/**
 * @file wave_queue.cc
 * Wavefront propagation as a function of time.
 */
#include <usml/eigenverbs/eigenverb_model.h>
#include <usml/waveq3d/ode_integ.h>
#include <usml/waveq3d/reflection_model.h>
#include <usml/waveq3d/spreading_hybrid_gaussian.h>
#include <usml/waveq3d/spreading_ray.h>
#include <usml/waveq3d/wave_queue.h>

#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/triangular.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <cmath>
#include <iomanip>
#include <utility>

// #define DEBUG_EIGENRAYS_DETAIL
// #define DEBUG_EIGENRAYS
// #define DEBUG_EIGENVERBS

using namespace usml::eigenrays;
using namespace usml::eigenverbs;
using namespace usml::waveq3d;

/**
 * Initialize a propagation scenario.
 */
wave_queue::wave_queue(const ocean_model::csptr& ocean,
                       const seq_vector::csptr& freq, const wposition1& pos,
                       const seq_vector::csptr& de, const seq_vector::csptr& az,
                       double time_step, const wposition* target_pos,
                       const size_t run_id, spreading_type type)
    : _ocean(ocean),
      _frequencies(freq),
      _source_pos(pos),
      _source_de(de),
      _source_az(az),
      _max_de(de->size() - 1),
      _max_az(az->size() - 1),
      _time_step(time_step),
      _time(0.0),
      _target_pos(target_pos),
      _run_id(run_id),
      _nc_file(nullptr) {
    _az_boundary = false;
    if (_source_az->size() > 1) {
        const double az_first = abs((*_source_az)(0));
        const double az_last = abs((*_source_az)(_source_az->size() - 1));
        _az_boundary =
            (fmod(az_first + 360.0, 360.0) == fmod(az_last + 360.0, 360.0));
    }
    if (_target_pos != nullptr) {
        _targets_sin_theta = sin(_target_pos->theta());
    }

    // check for sources outside of the water column

    const double offset = 0.1;
    double bottom_rho = 0.0;
    _ocean->bottom()->height(_source_pos, &bottom_rho);
    bottom_rho += offset;
    if (_source_pos.altitude() > -offset) {
        _source_pos.altitude(-offset);
    } else if (_source_pos.rho() < bottom_rho) {
        _source_pos.rho(bottom_rho);
    }

    // create storage space for all wavefront elements

    _past = new wave_front(_ocean, _frequencies, de->size(), az->size(),
                           _target_pos, &_targets_sin_theta);
    _prev = new wave_front(_ocean, _frequencies, de->size(), az->size(),
                           _target_pos, &_targets_sin_theta);
    _curr = new wave_front(_ocean, _frequencies, de->size(), az->size(),
                           _target_pos, &_targets_sin_theta);
    _next = new wave_front(_ocean, _frequencies, de->size(), az->size(),
                           _target_pos, &_targets_sin_theta);

    // initialize wave front elements

    _curr->init_wave(pos, de, az);
    _curr->update();
    init_wavefronts();
    _reflection_model = new reflection_model(*this);
    _spreading_model = nullptr;
    if (_source_de->size() >= 3 && _source_az->size() >= 3) {
        switch (type) {
            case HYBRID_GAUSSIAN:
                _spreading_model = new spreading_hybrid_gaussian(*this);
                break;
            default:
                _spreading_model = new spreading_ray(*this);
                break;
        }
    }
}

/** Destroy all temporary memory. */
wave_queue::~wave_queue() {
    delete _spreading_model;
    delete _reflection_model;
    delete _past;
    delete _prev;
    delete _curr;
    delete _next;
}

/**
 * Initialize wavefronts at the start of propagation using a
 * 3rd order Runge-Kutta algorithm.
 */
void wave_queue::init_wavefronts() {
    // Runge-Kutta to estimate _prev wavefront from _curr entry

    ode_integ::rk1_pos(-_time_step, _curr, _next);
    ode_integ::rk1_ndir(-_time_step, _curr, _next);
    _next->update();

    ode_integ::rk2_pos(-_time_step, _curr, _next, _past);
    ode_integ::rk2_ndir(-_time_step, _curr, _next, _past);
    _past->update();

    ode_integ::rk3_pos(-_time_step, _curr, _next, _past, _prev);
    ode_integ::rk3_ndir(-_time_step, _curr, _next, _past, _prev);
    _prev->update();

    // Runge-Kutta to estimate _past wavefront from _prev entry

    ode_integ::rk1_pos(-_time_step, _prev, _next);
    ode_integ::rk1_ndir(-_time_step, _prev, _next);
    _next->update();

    ode_integ::rk2_pos(-_time_step, _prev, _next, _past);
    ode_integ::rk2_ndir(-_time_step, _prev, _next, _past);
    _past->update();

    ode_integ::rk3_pos(-_time_step, _prev, _next, _past, _past, false);
    ode_integ::rk3_ndir(-_time_step, _prev, _next, _past, _past, false);
    _past->update();

    // Adams-Bashforth to estimate _next wavefront
    // from _past, _prev, and _curr entries

    ode_integ::ab3_pos(_time_step, _past, _prev, _curr, _next);
    ode_integ::ab3_ndir(_time_step, _past, _prev, _curr, _next);
    _next->update();
    _next->path_length = _next->distance + _curr->path_length;
}

/**
 * Marches to the next integration step in the acoustic propagation.
 */
void wave_queue::step() {
    // search for caustics and boundary reflections

    detect_reflections();

    // rotate wavefront queue to the next step.

    wave_front* save = _past;
    _past = _prev;
    _prev = _curr;
    _curr = _next;
    _next = save;
    _time += _time_step;

    // compute position, direction, and environment parameters for next entry

    ode_integ::ab3_pos(_time_step, _past, _prev, _curr, _next);
    ode_integ::ab3_ndir(_time_step, _past, _prev, _curr, _next);

    _next->update();
    _next->path_length = _next->distance + _curr->path_length;

    _next->attenuation += _curr->attenuation;
    _next->phase += _curr->phase;
    _next->surface = _curr->surface;
    _next->bottom = _curr->bottom;
    _next->upper = _curr->upper;
    _next->lower = _curr->lower;
    _next->caustic = _curr->caustic;

    // search for eigenray collisions with acoustic targets

    detect_eigenrays();

    // notify listeners that this step is complete

    check_eigenray_listeners(_time, runID());
}

/**
 * Detect and process boundary reflections and caustics.
 */
void wave_queue::detect_reflections() {
    // process all surface and bottom reflections, and vertices
    // note that multiple rays can reflect in the same time step

    for (size_t de = 0; de < num_de(); ++de) {
        for (size_t az = 0; az < num_az(); ++az) {
            detect_volume_scattering(de, az);
            if (!detect_reflections_surface(de, az)) {
                if (!detect_reflections_bottom(de, az)) {
                    detect_vertices(de, az);
                    detect_caustics(de, az);
                }
            }
        }
    }

    // search for other changes in wavefront

    _next->find_edges();
}

/**
 * Detect and process reflection for a single (DE,AZ) combination.
 */
// NOLINTNEXTLINE(misc-no-recursion)
bool wave_queue::detect_reflections_surface(size_t de, size_t az) {
    if (_next->position.altitude(de, az) > 0.0) {
        if (_reflection_model->surface_reflection(de, az)) {
            _next->surface(de, az) += 1;
            _curr->surface(de, az) = _prev->surface(de, az) =
                _past->surface(de, az) = _next->surface(de, az);
            detect_volume_scattering(de, az);
            detect_reflections_bottom(de, az);
            return true;  // indicate a surface reflection
        }
    }
    return false;  // indicates no reflection
}

/**
 * Detect and process reflection for a single (DE,AZ) combination.
 */
// NOLINTNEXTLINE(misc-no-recursion)
bool wave_queue::detect_reflections_bottom(size_t de, size_t az) {
    double height;
    wposition1 pos(_next->position, de, az);
    _ocean->bottom()->height(pos, &height, nullptr);
    const double depth = height - _next->position.rho(de, az);
    if (depth > 0.0) {
        if (_reflection_model->bottom_reflection(de, az, depth)) {
            _next->bottom(de, az) += 1;
            _curr->bottom(de, az) = _prev->bottom(de, az) =
                _past->bottom(de, az) = _next->bottom(de, az);
            detect_volume_scattering(de, az);
            detect_reflections_surface(de, az);
            return true;  // indicate a surface reflection
        }
    }
    return false;  // indicates no reflection
}

/**
 *  Detects upper and lower vertices along the wavefront
 */
void wave_queue::detect_vertices(size_t de, size_t az) {
    double L = _curr->ndirection.rho(de, az);
    double R = _next->ndirection.rho(de, az);
    if (L * R < 0.0 && R < 0.0) {
        _next->upper(de, az)++;
    } else if (L * R < 0.0 && 0.0 < R) {
        _next->lower(de, az)++;
    }
}

/**
 *  Detects and processes the caustics along the next wavefront
 */
void wave_queue::detect_caustics(size_t de, size_t az) {
    if (de < _max_de) {
        double A = _curr->position.rho(de + 1, az);
        double B = _curr->position.rho(de, az);
        double C = _next->position.rho(de + 1, az);
        double D = _next->position.rho(de, az);
        bool fold = false;
        if ((_next->surface(de + 1, az) == _next->surface(de, az)) &&
            (_next->bottom(de + 1, az) == _next->bottom(de, az))) {
            fold = true;
        }
        if ((C - D) * (A - B) < 0 && fold) {
            _next->caustic(de + 1, az)++;
            for (size_t f = 0; f < _frequencies->size(); ++f) {
                _next->phase(de + 1, az)(f) -= M_PI_2;
            }
        }
    }
}

/**
 * Detect volume boundary reflections for reverberation contributions
 */
void wave_queue::detect_volume_scattering(size_t de, size_t az) {
    if (!has_eigenverb_listeners()) {
        return;
    }
    if (above_bounce_threshold(_curr, de, az)) {
        return;
    }
    for (size_t i = 0; i < _ocean->num_volume(); ++i) {
        volume_model::csptr layer = _ocean->volume(i);
        wposition1 pos_curr(_curr->position, de, az);
        wposition1 pos_next(_next->position, de, az);
        double height;
        layer->depth(pos_next, &height, nullptr);
        double d1 = height - pos_next.rho();  // positive when next below layer
        double d2 = height - pos_curr.rho();  // positive when curr below layer
        if (d1 * d2 >= 0) {
            continue;  // no collision if same sign
        }

        // compute the grazing angle

        double c = _curr->sound_speed(de, az);
        const double d = c * c * _curr->ndirection.rho(de, az);
        double time_water =
            (d == 0.0) ? 0.0 : -_curr->position.altitude(de, az) / d;

        // compute the precise values for position, direction,
        // sound speed, and grazing angle at the point of collision

        wposition1 position;
        wvector1 ndirection;
        collision_location(de, az, time_water, &position, &ndirection, &c);
        // clang-format off
        double grazing = atan2(_curr->ndirection.rho(de, az), sqrt(
        		_curr->ndirection.theta(de, az) * _curr->ndirection.theta(de, az) +
				_curr->ndirection.phi(de, az) *_curr->ndirection.phi(de, az)));
        // clang-format off

        // determine whether the collision is from below or from above the layer

        if (d1 > 0) {
            build_eigenverb(de, az, time_water, grazing, c, position,
                            ndirection, eigenverb_model::VOLUME_LOWER + i * 2);
        } else {
            build_eigenverb(de, az, time_water, grazing, c, position,
                            ndirection, eigenverb_model::VOLUME_UPPER + i * 2);
        }
    }
}

/**
 * Detect and process wavefront closest point of approach (CPA) with target.
 */
//NOLINTNEXTLINE(readability-function-cognitive-complexity)
void wave_queue::detect_eigenrays() {
    if (_target_pos == nullptr) {
        return;
    }

    double distance2[3][3][3];
    double& center = distance2[1][1][1];
    size_t az_start = (_az_boundary) ? 0 : 1;

    // loop over all targets
    for (size_t t1 = 0; t1 < _target_pos->size1(); ++t1) {
        for (size_t t2 = 0; t2 < _target_pos->size2(); ++t2) {
            _de_branch = false;
            if (abs(_source_pos.latitude() - _target_pos->latitude(t1, t2)) <
                    1e-4 &&
                abs(_source_pos.longitude() - _target_pos->longitude(t1, t2)) <
                    1e-4) {
                _de_branch = true;
            }

            // Loop over all rays
            for (size_t de = 1; de < _max_de; ++de) {
                for (size_t az = az_start; az < _max_az; ++az) {
                    // *******************************************
                    // When central ray is at the edge of ray family
                    // it prevents edges from acting as CPA, if so, go to next
                    // de/az Also check to see if this ray is a duplicate.

                    if (_curr->on_edge(de, az)) {
                        continue;
                    }

                    // get the central ray for testing
                    center = _curr->distance2(t1, t2)(de, az);

                    distance2[2][1][1] = _next->distance2(t1, t2)(de, az);
                    if (distance2[2][1][1] <= center) {
                        continue;
                    }

                    distance2[0][1][1] = _prev->distance2(t1, t2)(de, az);
                    if (distance2[0][1][1] < center) {
                        continue;
                    }

                    // *******************************************
                    if (is_closest_ray(t1, t2, de, az, center, distance2)) {
                        build_eigenray(t1, t2, de, az, distance2);
                    }
                }  // end az loop
            }      // end de loop
        }          // end t2 loop
    }              // end t1 loop
}

/**
 * Used by detect_eigenrays() to discover if the current ray is the
 * closest point of approach to the current target.
 */
//NOLINTNEXTLINE(readability-function-cognitive-complexity)
bool wave_queue::is_closest_ray(size_t t1, size_t t2, size_t de, size_t az,
                                const double& center, double distance2[3][3][3]) {
    // test all neighbors that are not the central ray

    for (size_t nde = 0; nde < 3; ++nde) {
        for (size_t naz = 0; naz < 3; ++naz) {
            if (nde == 1 && naz == 1) {
                continue;
            }

            // compute distances on the current, next, and previous wavefronts

            size_t d = de + nde - 1;
            size_t a = az + naz - 1;
            if (_az_boundary) {
                if (az + naz == 0) {  // aka if a < 0
                    a = num_az() - 2;
                } else if (a >= _max_az) {
                    a = 0;
                }
            }

            distance2[0][nde][naz] = _prev->distance2(t1, t2)(d, a);
            distance2[1][nde][naz] = _curr->distance2(t1, t2)(d, a);
            distance2[2][nde][naz] = _next->distance2(t1, t2)(d, a);

            // skip to next iteration if tested ray is on edge of ray family
            // allows extrapolation outside of ray family

            if (a == 0 && !_az_boundary) {
                continue;
            }
            if (a == _max_az) {
                continue;
            }
            if (_de_branch) {
                if (_curr->on_edge(d, a)) {
                    continue;
                }
            } else {
                if (nde != 1) {
                    if (_curr->on_edge(d, a)) {
                        continue;
                    }
                }
            }

            // test to see if the center value is the smallest

            if (nde == 2 || naz == 2) {
                if (_de_branch) {
                    if (az == 0) {
                        if (distance2[1][nde][naz] < center) {
                            return false;
                        }
                    } else {
                        return false;
                    }
                } else {
                    if (distance2[1][nde][naz] <= center) {
                        return false;
                    }
                }
            } else {
                if (distance2[1][nde][naz] < center) {
                    return false;
                }
            }
            if (distance2[2][nde][naz] <= center) {
                return false;
            }
            if (distance2[0][nde][naz] < center) {
                return false;
            }
        }
    }
    return true;
}  // end is_closest_ray

/**
 * Used by detect_eigenrays() to compute eigenray parameters and
 * add a new eigenray entry to the current target.
 */
//NOLINTNEXTLINE(readability-function-cognitive-complexity)
void wave_queue::build_eigenray(size_t t1, size_t t2, size_t de, size_t az,
                                double distance2[3][3][3]) {
    if (above_bounce_threshold(_curr, de, az)) {
        return;
    }
#ifdef DEBUG_EIGENRAYS_DETAIL
    // cout << "*** wave_queue::step: time=" << time() << endl ;
    wposition1 tgt(*(_curr->targets), t1, t2);
    cout << "*** wave_queue::build_eigenray:" << endl
         << "\ttarget(" << t1 << "," << t2 << ")=" << tgt.altitude() << ","
         << tgt.latitude() << "," << tgt.longitude() << " time=" << _time
         << " de(" << de << ")=" << (*_source_de)(de) << " az(" << az
         << ")=" << (*_source_az)(az) << endl;
    cout << "\tsurface=" << _curr->surface(de, az)
         << " bottom=" << _curr->bottom(de, az)
         << " caustic=" << _curr->caustic(de, az) << endl;
// cout << "\tdistance2:" << endl ;
// for ( unsigned n1=0 ; n1 < 3 ; ++n1 ) {
//    cout << "\t    " ;
//    for ( unsigned n2=0 ; n2 < 3 ; ++n2 ) {
//        cout << ((n2)? "; " : "[ " ) ;
//        for ( unsigned n3=0 ; n3 < 3 ; ++n3 ) {
//            cout << ((n3)? "," : "[" ) << distance2[n1][n2][n3] ;
//        }
//        cout << "]" ;
//    }
//    cout << " ]" << endl ;
//}
#endif

    // find relative offsets and true distances in time, D/E, and azimuth

    c_vector<double, 3> delta;
    c_vector<double, 3> offset;
    c_vector<double, 3> distance;
    delta(0) = _time_step;
    delta(1) = _source_de->increment(de);
    delta(2) = _source_az->increment(az);

    bool unstable = false;
    const int surface = _curr->surface(de, az);
    const int bottom = _curr->bottom(de, az);
    const int caustic = _curr->caustic(de, az);

    for (size_t nde = 0; nde < 3 && !unstable; ++nde) {
        size_t d = de + nde - 1;
        for (size_t naz = 0; naz < 3 && !unstable; ++naz) {
            size_t a = az + naz - 1;
            if (_az_boundary) {
                if (az + naz == 0) {  // aka if a < 0
                    a = _max_az - 1;
                } else if (a >= _max_az) {
                    a = 0;
                }
            }
            if (_prev->surface(d, a) != surface ||
                _curr->surface(d, a) != surface ||
                _next->surface(d, a) != surface ||
                _prev->bottom(d, a) != bottom ||
                _curr->bottom(d, a) != bottom ||
                _next->bottom(d, a) != bottom ||
                _prev->caustic(d, a) != caustic ||
                _curr->caustic(d, a) != caustic ||
                _next->caustic(d, a) != caustic) {
                unstable = true;
            }
        }
    }

    compute_offsets(t1, t2, de, az, distance2, delta, offset, distance);

    // build basic eigenray products

    auto* ray = new eigenray_model();
    auto ray_csptr = eigenray_model::csptr(ray);
    ray->travel_time = _time + offset(0);
    ray->source_de = (*_source_de)(de) + offset(1);
    ray->source_az = (*_source_az)(az) + offset(2);
    ray->frequencies = _frequencies;
    ray->surface = _curr->surface(de, az);
    ray->bottom = _curr->bottom(de, az);
    ray->caustic = _curr->caustic(de, az);
    ray->upper = _curr->upper(de, az);
    ray->lower = _curr->lower(de, az);
    ray->phase = _curr->phase(de, az);

    // compute spreading components of intensity

    const vector<double> spread_intensity = _spreading_model->intensity(
        wposition1(*(_curr->targets), t1, t2), de, az, offset, distance);
    for (size_t i = 0; i < ray->intensity.size(); ++i) {
        if (std::isnan(spread_intensity(i))) {
            #ifdef USML_DEBUG
                std::cerr << "warning: wave_queue::build_eigenray()" << endl
                      << "\tignores eigenray because intensity is NaN" << endl
                      << "\tt1=" << t1 << " t2=" << t2 << " de=" << de
                      << " az=" << az << endl;
            #endif
            return;
        }
    }

    ray->intensity =
        -10.0 * log10(max(spread_intensity, 1e-30));  // positive value

    // compute attenuation components of intensity

    double dt = offset(0) / _time_step;
    if (dt >= 0.0) {
        ray->intensity = ray->intensity +
                         _curr->attenuation(de, az) * (1.0 - dt) +
                         _next->attenuation(de, az) * dt;
    } else {
        dt = 1.0 + dt;
        ray->intensity = ray->intensity +
                         _prev->attenuation(de, az) * (1.0 - dt) +
                         _curr->attenuation(de, az) * dt;
    }

    // determine if intensity is weaker than the intensity threshold.

    if (!above_intensity_threshold(ray->intensity)) {
        return;
    }

    // estimate target D/E angle using 2nd order vector Taylor series
    // re-uses "distance2" variable to store D/E angles

    for (size_t nde = 0; nde < 3; ++nde) {
        for (size_t naz = 0; naz < 3; ++naz) {
            size_t d = de + nde - 1;
            size_t a = az + naz - 1;
            if (_az_boundary) {
                if (az + naz == 0) {  // aka if a < 0
                    a = _max_az - 1;
                } else if (a >= _max_az) {
                    a = 0;
                }
            }
            double dummy;
            {
                wvector1 ndir(_prev->ndirection, d, a);
                ndir.direction(&distance2[0][nde][naz], &dummy);
            }
            {
                wvector1 ndir(_curr->ndirection, d, a);
                ndir.direction(&distance2[1][nde][naz], &dummy);
            }
            {
                wvector1 ndir(_next->ndirection, d, a);
                ndir.direction(&distance2[2][nde][naz], &dummy);
            }
        }
    }

    double center;
    c_vector<double, 3> gradient;
    c_matrix<double, 3, 3> hessian;
    make_taylor_coeff(distance2, delta, center, gradient, hessian);
    ray->target_de = center + inner_prod(gradient, offset) +
                     0.5 * inner_prod(offset, prod(hessian, offset));

    // estimate target AZ angle using 2nd order vector Taylor series
    // re-uses "distance2" variable to store AZ angles

    for (size_t nde = 0; nde < 3; ++nde) {
        for (size_t naz = 0; naz < 3; ++naz) {
            size_t d = de + nde - 1;
            size_t a = az + naz - 1;
            if (_az_boundary) {
                if (az + naz == 0) {  // aka if a < 0
                    a = _max_az - 1;
                } else if (a >= _max_az) {
                    a = 0;
                }
            }
            double dummy;
            {
                wvector1 ndir(_prev->ndirection, d, a);
                ndir.direction(&dummy, &distance2[0][nde][naz]);
            }
            {
                wvector1 ndir(_curr->ndirection, d, a);
                ndir.direction(&dummy, &distance2[1][nde][naz]);
            }
            {
                wvector1 ndir(_next->ndirection, d, a);
                ndir.direction(&dummy, &distance2[2][nde][naz]);
            }
        }
    }

    make_taylor_coeff(distance2, delta, center, gradient, hessian);
    ray->target_az = center + inner_prod(gradient, offset) +
                     0.5 * inner_prod(offset, prod(hessian, offset));

    #ifdef DEBUG_EIGENRAYS
        cout << "wave_queue::build_eigenray() " << endl
             << "\ttarget(" << t1 << "," << t2 << "):" << endl
             << "\tt=" << ray->travel_time << " inten=" << ray->intensity
             << " de=" << ray->source_de << " az=" << ray->source_az << endl
             << "\tsurface=" << ray->surface << " bottom=" << ray->bottom
             << " caustic=" << ray->caustic << endl;
    #endif

    // Add eigenray to those objects which requested them
    notify_eigenray_listeners(t1, t2, ray_csptr, runID());
}

/**
 * Find relative offsets and true distances in time, D/E, and azimuth.
 */
//NOLINTNEXTLINE(readability-function-cognitive-complexity)
void wave_queue::compute_offsets(size_t t1, size_t t2, size_t de, size_t az,
                                 const double distance2[3][3][3],
                                 const c_vector<double, 3>& delta,
                                 c_vector<double, 3>& offset,
                                 c_vector<double, 3>& distance) {

    // mark as "unstable" if path types change in this neighborhood

    bool unstable = false;
    const int surface = _curr->surface(de, az);
    const int bottom = _curr->bottom(de, az);
    const int caustic = _curr->caustic(de, az);

    for (size_t nde = 0; nde < 3 && !unstable; ++nde) {
        size_t d = de + nde - 1;
        for (size_t naz = 0; naz < 3 && !unstable; ++naz) {
            size_t a = az + naz - 1;
            if (_az_boundary) {
                if (az + naz == 0) {  // aka if a < 0
                    a = num_az() - 2;
                } else if (a >= num_az() - 1) {
                    a = 0;
                }
            }
            if (_prev->surface(d, a) != surface ||
                _curr->surface(d, a) != surface ||
                _next->surface(d, a) != surface ||
                _prev->bottom(d, a) != bottom ||
                _curr->bottom(d, a) != bottom ||
                _next->bottom(d, a) != bottom ||
                _prev->caustic(d, a) != caustic ||
                _curr->caustic(d, a) != caustic ||
                _next->caustic(d, a) != caustic || _prev->on_edge(d, a) ||
                _prev->on_edge(d, a) || _prev->on_edge(d, a)) {
                unstable = true;
            }
        }
    }

    // compute 1st and 2nd derivatives of distance squared

    double center;
    c_vector<double, 3> gradient;
    c_matrix<double, 3, 3> hessian;
    make_taylor_coeff(distance2, delta, center, gradient, hessian);

    // prepare to compute inverse of Hessian matrix (2nd derivative)

    c_matrix<double, 3, 3> inverse;
    double determinant =
        max(1e-6, hessian(0, 0) * (hessian(1, 1) * hessian(2, 2) -
                                   hessian(1, 2) * hessian(2, 1)) +
                      hessian(0, 1) * (hessian(1, 2) * hessian(2, 0) -
                                       hessian(1, 0) * hessian(2, 2)) +
                      hessian(0, 2) * (hessian(1, 0) * hessian(2, 1) -
                                       hessian(2, 1) * hessian(2, 0)));
    unstable = unstable || determinant < norm_2(gradient);

#ifdef DEBUG_EIGENRAYS_DETAIL
    // cout << "*** wave_queue::compute_offsets ***" << endl;
    cout << "\tgradient: " << gradient << " center: " << center << endl;
    cout << "\thessian: " << hessian << endl;
#endif

    // fallback offset calculation using just diagonals
    // if inverse can not be computed because determinant is zero
    // non-positive hessian diags are an indication that offset is unstable

    for (size_t n = 0; n < 3; ++n) {
        const double h = max(1e-10, hessian(n, n));
        offset(n) = -gradient(n) / h;
        if (abs(offset(n) / delta(n)) > 0.5) {
            unstable = true;
            offset(n) = copysign(0.5 * delta(n), offset(n));
        }
    }

    // compute offsets by inverting H x = g to create x = inv(H) g
    // if stable, compute full 3x3 inverse in time, DE, AZ

    if (!unstable) {
#ifdef DEBUG_EIGENRAYS_DETAIL
        cout << "\tcompute full 3x3 inverse in (time,DE,AZ), det="
             << determinant << endl;
#endif
        inverse(0, 0) =
            hessian(1, 1) * hessian(2, 2) - hessian(1, 2) * hessian(2, 1);
        inverse(1, 0) =
            hessian(1, 2) * hessian(2, 0) - hessian(1, 0) * hessian(2, 2);
        inverse(2, 0) =
            hessian(1, 0) * hessian(2, 1) - hessian(1, 1) * hessian(2, 0);
        inverse(0, 1) = inverse(1, 0);
        inverse(1, 1) =
            hessian(0, 0) * hessian(2, 2) - hessian(0, 2) * hessian(2, 0);
        inverse(2, 1) =
            hessian(2, 0) * hessian(0, 1) - hessian(0, 0) * hessian(2, 1);
        inverse(0, 2) = inverse(2, 0);
        inverse(1, 2) = inverse(2, 1);
        inverse(2, 2) =
            hessian(0, 0) * hessian(1, 1) - hessian(0, 1) * hessian(1, 0);
        inverse /= determinant;
        noalias(offset) = prod(inverse, -gradient);
        unstable = unstable || abs(offset(1)) >= 0.5 * abs(delta(1));

        // if unstable, try computing 2x2 inverse in time, AZ

    } else {
        const double save_offset = offset(1);
        // offset(1) = min( 0.5 * abs(delta(1)), abs(gradient(1)/hessian(1,1))
        // );
        gradient(1) = 0.0;
        determinant =
            hessian(0, 0) * hessian(2, 2) - hessian(0, 2) * hessian(2, 0);
        if (determinant >= norm_2(gradient)) {
#ifdef DEBUG_EIGENRAYS_DETAIL
            cout << "\tcompute 2x2 matrix in (time,AZ), det=" << determinant
                 << endl;
#endif
            inverse = zero_matrix<double>(3, 3);
            inverse(0, 0) = hessian(2, 2);
            inverse(0, 2) = -hessian(0, 2);
            inverse(1, 1) = 1.0;
            inverse(2, 0) = -hessian(2, 0);
            inverse(2, 2) = hessian(0, 0);
            inverse /= determinant;
            noalias(offset) = prod(inverse, -gradient);
            offset(1) = save_offset;
        }
    }

    // use forward Taylor series to compute distance along each axis

    for (size_t n = 0; n < 3; ++n) {
        distance(n) = -gradient(n) * offset(n) -
                      0.5 * hessian(n, n) * offset(n) * offset(n);
        distance(n) = sqrt(max(0.0, distance(n)));
        if (offset(n) < 0.0) {
            distance(n) *= -1.0;
        }
    }
#ifdef DEBUG_EIGENRAYS_DETAIL
    cout << "\toffset: " << offset << " distance: " << distance << endl;
#endif

    // if unstable, use distance in time, and AZ to find distance in DE

    if (unstable) {
        // compute distance in DE direction as remainder from total distance

        distance(1) =
            center - distance(0) * distance(0) - distance(2) * distance(2);
        // distance(1) = max( 0.0, distance(1) ) ;
        if (distance(1) <= 0.0) {
            distance(1) = 0.0;
            offset(1) = 0.0;
        } else {
            distance(1) = sqrt(distance(1));
            offset(1) = abs(offset(1));

            // propagate CPA ray and DE+1 ray to time of collision
            // use a second order Taylor series like reflection does

            wposition1 cpa;
            wposition1 nxt;
            wposition1 prv;
            wposition1 tgt;
            wvector1 dir;
            double speed;
            collision_location(de, az, offset(0), &cpa, &dir, &speed);
            collision_location(de + 1, az, offset(0), &nxt, &dir, &speed);
            collision_location(de - 1, az, offset(0), &prv, &dir, &speed);

            // compute direction from CPA ray to DE+1 ray

            c_vector<double, 3> nxt_dir;
            nxt_dir[0] = nxt.rho() - cpa.rho();
            nxt_dir[1] = (nxt.theta() - prv.theta()) * cpa.rho();
            nxt_dir[2] = (nxt.phi() - prv.phi()) * cpa.rho() * sin(cpa.theta());

            // workaround:
            // When the rays are very close together and the wavefront
            // is travelling almost straight down, small errors in the
            // rho difference create large errors in the direction
            // from CPA ray to DE+1 ray.  This check seems to work around
            // that problem.  Needs a better long term fix.

            if (abs(nxt_dir[0]) < 0.01) {
                nxt_dir[0] = 0.0;
            }

            // compute direction from CPA ray to target

            c_vector<double, 3> tgt_dir;
            tgt_dir[0] = _target_pos->rho(t1, t2) - cpa.rho();
            tgt_dir[1] = (_target_pos->theta(t1, t2) - cpa.theta()) * cpa.rho();
            tgt_dir[2] = (_target_pos->phi(t1, t2) - cpa.phi()) * cpa.rho() *
                         sin(cpa.theta());

            // switch the sign of distance and offset if target and DE+1 ray
            // on opposite sides of CPA ray in rho direction

            double dot = inner_prod(nxt_dir, tgt_dir);
            if (dot < 0.0) {
                distance(1) *= -1.0;
                offset(1) *= -1.0;
            }
#ifdef DEBUG_EIGENRAYS_DETAIL
            cout << "\tnxt_dir: " << nxt_dir << " tgt_dir: " << tgt_dir
                 << " dot: " << dot << endl;
#endif
        }
#ifdef DEBUG_EIGENRAYS_DETAIL
        cout << "\trecompute offset(1): " << offset(1)
             << " distance(1): " << distance(1) << endl;
#endif
    }
}

/**
 * Computes the Taylor coefficients used to compute eigenrays.
 */
void wave_queue::make_taylor_coeff(const double value[3][3][3],
                                   const c_vector<double, 3>& delta,
                                   double& center,
                                   c_vector<double, 3>& gradient,
                                   c_matrix<double, 3, 3>& hessian) {
    const double d0 = 2.0 * delta(0);
    const double d1 = 2.0 * delta(1);
    const double d2 = 2.0 * delta(2);

    // find value at the center point

    center = value[1][1][1];

    // compute diagonal terms in Hessian matrix

    hessian.clear();
    hessian(0, 0) = (value[2][1][1] + value[0][1][1] - 2.0 * center) /
                    (delta(0) * delta(0));
    hessian(1, 1) = (value[1][2][1] + value[1][0][1] - 2.0 * center) /
                    (delta(1) * delta(1));
    hessian(2, 2) = (value[1][1][2] + value[1][1][0] - 2.0 * center) /
                    (delta(2) * delta(2));

    // compute off-diagonal terms in Hessian matrix, unless told not to

    gradient(0) = (value[2][0][1] - value[0][0][1]) / d0;
    gradient(2) = (value[2][2][1] - value[0][2][1]) / d0;
    hessian(0, 1) = (gradient(2) - gradient(0)) / d1;
    hessian(1, 0) = hessian(0, 1);

    gradient(0) = (value[2][1][0] - value[0][1][0]) / d0;
    gradient(2) = (value[2][1][2] - value[0][1][2]) / d0;
    hessian(0, 2) = (gradient(2) - gradient(0)) / d2;
    hessian(2, 0) = hessian(0, 2);

    gradient(0) = (value[1][2][0] - value[1][0][0]) / d1;
    gradient(2) = (value[1][2][2] - value[1][0][2]) / d1;
    hessian(1, 2) = (gradient(2) - gradient(0)) / d2;
    hessian(2, 1) = hessian(1, 2);

    // compute gradient near center point

    gradient(0) = (value[2][1][1] - value[0][1][1]);
    gradient(1) = (value[1][2][1] - value[1][0][1]);
    gradient(2) = (value[1][1][2] - value[1][1][0]);
    gradient(0) = (value[2][1][1] - value[0][1][1]) / d0;
    gradient(1) = (value[1][2][1] - value[1][0][1]) / d1;
    gradient(2) = (value[1][1][2] - value[1][1][0]) / d2;
}

/**
 * Compute the precise location and direction at the point of collision.
 */
void wave_queue::collision_location(size_t de, size_t az, double time_water,
                                    wposition1* position, wvector1* ndirection,
                                    double* speed) const {
    double drho;
    double dtheta;
    double dphi;
    double d2rho;
    double d2theta;
    double d2phi;
    const double time1 = 2.0 * _time_step;
    const double time2 = _time_step * _time_step;
    const double dtime2 = time_water * time_water;

    // second order Taylor series for sound speed

    drho = (_next->sound_speed(de, az) - _prev->sound_speed(de, az)) / time1;

    d2rho = (_next->sound_speed(de, az) + _prev->sound_speed(de, az) -
             2.0 * _curr->sound_speed(de, az)) /
            time2;

    *speed =
        _curr->sound_speed(de, az) + drho * time_water + 0.5 * d2rho * dtime2;

    // second order Taylor series for position

    drho = (_next->position.rho(de, az) - _prev->position.rho(de, az)) / time1;
    dtheta =
        (_next->position.theta(de, az) - _prev->position.theta(de, az)) / time1;
    dphi = (_next->position.phi(de, az) - _prev->position.phi(de, az)) / time1;

    d2rho = (_next->position.rho(de, az) + _prev->position.rho(de, az) -
             2.0 * _curr->position.rho(de, az)) /
            time2;
    d2theta = (_next->position.theta(de, az) + _prev->position.theta(de, az) -
               2.0 * _curr->position.theta(de, az)) /
              time2;
    d2phi = (_next->position.phi(de, az) + _prev->position.phi(de, az) -
             2.0 * _curr->position.phi(de, az)) /
            time2;

    position->rho(_curr->position.rho(de, az) + drho * time_water +
                  0.5 * d2rho * dtime2);
    position->theta(_curr->position.theta(de, az) + dtheta * time_water +
                    0.5 * d2theta * dtime2);
    position->phi(_curr->position.phi(de, az) + dphi * time_water +
                  0.5 * d2phi * dtime2);

    // second order Taylor series for ndirection

    drho =
        (_next->ndirection.rho(de, az) - _prev->ndirection.rho(de, az)) / time1;
    dtheta =
        (_next->ndirection.theta(de, az) - _prev->ndirection.theta(de, az)) /
        time1;
    dphi =
        (_next->ndirection.phi(de, az) - _prev->ndirection.phi(de, az)) / time1;

    d2rho = (_next->ndirection.rho(de, az) + _prev->ndirection.rho(de, az) -
             2.0 * _curr->ndirection.rho(de, az)) /
            time2;
    d2theta =
        (_next->ndirection.theta(de, az) + _prev->ndirection.theta(de, az) -
         2.0 * _curr->ndirection.theta(de, az)) /
        time2;
    d2phi = (_next->ndirection.phi(de, az) + _prev->ndirection.phi(de, az) -
             2.0 * _curr->ndirection.phi(de, az)) /
            time2;

    ndirection->rho(_curr->ndirection.rho(de, az) + drho * time_water +
                    0.5 * d2rho * dtime2);
    ndirection->theta(_curr->ndirection.theta(de, az) + dtheta * time_water +
                      0.5 * d2theta * dtime2);
    ndirection->phi(_curr->ndirection.phi(de, az) + dphi * time_water +
                    0.5 * d2phi * dtime2);
}

/**
 * Constructs an eigenverb from a collision with a boundary_model
 * or volume_model.
 */
void wave_queue::build_eigenverb(size_t de, size_t az, double dt,
                                 double grazing, double speed,
                                 const wposition1& position,
                                 const wvector1& ndirection, size_t type) {
    grazing = abs(grazing);
    if (!has_eigenverb_listeners() || above_bounce_threshold(_curr, de, az) ||
        _time <= 0.0 || grazing < 1e-6 ||
        (this->_az_boundary && az == this->_max_az) ||
        abs(source_de(de)) > 89.9) {
        return;
    }
#ifdef DEBUG_EIGENVERBS
    cout << "wave_queue::build_eigenverb() " << endl;
#endif

    // compute size of area centered on ray
    //   - use inc halfway to next and prev ray for arbitrary ray spacing
    //   - wrap az-1 around to end of sequence if az=0 and _az_boundary
    //   - otherwise assumes seq_vector::increment() handles end points
    //   - compute average height and width such that area = height * width

    const double de_angle = to_radians((*_source_de)(de));
    const double de_plus =
        de_angle + 0.5 * to_radians(_source_de->increment(de));
    const double de_minus =
        de_angle - 0.5 * to_radians(_source_de->increment(de - 1));

    const double az_angle = to_radians((*_source_az)(az));
    const double az_plus =
        az_angle + 0.5 * to_radians(_source_az->increment(az));
    const size_t az_index = (az == 0 && _az_boundary) ? _max_az : az;
    const double az_minus =
        az_angle - 0.5 * to_radians(_source_az->increment(az_index - 1));

    const double area = (sin(de_plus) - sin(de_minus)) * (az_plus - az_minus);
    const double de_delta = de_plus - de_minus;  // average height
    const double az_delta = area / de_delta;     // average width

    // compute the half-length and half-width of the eigenverb
    //   - assumes change in height and width proportional to path length
    //   - projects Gaussian beam onto the interface

    const double path_length =
        curr()->path_length(de, az) + curr()->sound_speed(de, az) * dt;
    if (path_length <= 0) {
        return;  // ray has not yet started propagating
    }
    const double sin_grazing = sin(grazing);

    auto* verb = new eigenverb_model();
    verb->length = 0.5 * path_length * de_delta / sin_grazing;
    verb->width = 0.5 * path_length * az_delta;

    // compute the frequency dependent total power in this eigenverb
    //    - using attenuation along the path and initial size of beam
    //	  - assuming that curr()->attenuation(de,az) in positive value in dB

    verb->power =
        pow(10.0, -0.1 * curr()->attenuation(de, az)) * area / sin_grazing;
    if (!above_eigenverb_threshold(verb->power)) {
        return;
    }

    // compute the eigenverb direction in local tangent plane

    double d;
    double a = 0.0;
    ndirection.direction(&d, &a);
    verb->direction = to_radians(a);

    // initialize simple eigenverb fields

    verb->travel_time = _time + dt;
    verb->grazing = grazing;
    verb->sound_speed = speed;
    verb->position = position;
    verb->de_index = de;
    verb->az_index = az;
    verb->source_de = to_radians(source_de(de));
    verb->source_az = to_radians(source_az(az));
    verb->frequencies = _frequencies;
    verb->surface = _curr->surface(de, az);
    verb->bottom = _curr->bottom(de, az);
    verb->caustic = _curr->caustic(de, az);
    verb->upper = _curr->upper(de, az);
    verb->lower = _curr->lower(de, az);

    // notify eigenverb listeners of this change

#ifdef DEBUG_EIGENVERBS
    cout << "\ttype " << type << " t=" << verb->travel_time
         << " de=" << to_degrees(verb->source_de)
         << " az=" << to_degrees(verb->source_az)
         << " direction=" << to_degrees(verb->direction)
         << " grazing=" << to_degrees(verb->grazing) << endl
         << "\tpower=" << 10.0 * log10(verb->power)
         << " length=" << verb->length << " width=" << verb->width << endl
         << "\tsurface=" << verb->surface << " bottom=" << verb->bottom
         << " caustic=" << verb->caustic << endl;
#endif
    notify_eigenverb_listeners(eigenverb_model::csptr(verb), type);
}
