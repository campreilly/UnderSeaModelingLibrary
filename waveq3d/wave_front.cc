/**
 * @file wave_front.cc
 * Wavefront characteristics at a specific point in time.
 */

#include <usml/ocean/profile_model.h>
#include <usml/waveq3d/wave_front.h>

#include <algorithm>
#include <boost/numeric/ublas/detail/definitions.hpp>
#include <boost/numeric/ublas/expression_types.hpp>
#include <boost/numeric/ublas/matrix_expression.hpp>
#include <boost/numeric/ublas/vector_expression.hpp>
#include <cmath>
#include <cstdlib>
#include <deque>

using namespace usml::waveq3d;

/**
 * Create workspace for all properties.
 */
wave_front::wave_front(ocean_model::csptr ocean, const seq_vector::csptr& freq,
                       size_t num_de, size_t num_az, const wposition* targets,
                       const matrix<double>* sin_theta)
    : position(num_de, num_az),
      pos_gradient(num_de, num_az),
      ndirection(num_de, num_az),
      ndir_gradient(num_de, num_az),
      sound_speed(num_de, num_az),
      sound_gradient(num_de, num_az),
      attenuation(num_de, num_az),
      phase(num_de, num_az),
      distance(num_de, num_az),
      path_length(num_de, num_az),
      surface(num_de, num_az),
      bottom(num_de, num_az),
      caustic(num_de, num_az),
      upper(num_de, num_az),
      lower(num_de, num_az),
      on_edge(num_de, num_az),
      targets(targets),
      _ocean(std::move(std::move(ocean))),
      _frequencies(freq),
      _dc_c(num_de, num_az),
      _c2_r(num_de, num_az),
      _sin_theta(num_de, num_az),
      _cot_theta(num_de, num_az),
      _target_sin_theta(sin_theta) {
    sound_speed.clear();
    distance.clear();
    path_length.clear();
    surface.clear();
    bottom.clear();
    caustic.clear();
    upper.clear();
    lower.clear();
    on_edge.clear();

    for (size_t n1 = 0; n1 < num_de; ++n1) {
        for (size_t n2 = 0; n2 < num_az; ++n2) {
            attenuation(n1, n2).resize(freq->size());
            attenuation(n1, n2).clear();
            phase(n1, n2).resize(freq->size());
            phase(n1, n2).clear();
        }
    }

    if (this->targets != nullptr) {
        distance2.resize(this->targets->size1(), this->targets->size2());
        for (size_t n1 = 0; n1 < this->targets->size1(); ++n1) {
            for (size_t n2 = 0; n2 < this->targets->size2(); ++n2) {
                distance2(n1, n2).resize(num_de, num_az);
                distance2(n1, n2).clear();
            }
        }
    }
}

/**
 * Initialize position and direction components of the wavefront.
 */
void wave_front::init_wave(const wposition1& pos, const seq_vector::csptr& de,
                           const seq_vector::csptr& az) {
    // compute direction for all D/E and AZ combinations

    for (size_t r = 0; r < de->size(); ++r) {
        double cos_de = cos(to_radians((*de)(r)));
        double sin_de = sin(to_radians((*de)(r)));
        for (size_t c = 0; c < az->size(); ++c) {
            double cos_az = cos(to_radians((*az)(c)));
            double sin_az = sin(to_radians((*az)(c)));

            // compute direction relative to east, north, up coord system

            ndirection.rho(r, c, sin_de);
            ndirection.theta(r, c, -cos_de * cos_az);
            ndirection.phi(r, c, cos_de * sin_az);

            // copy position from pos argument

            position.rho(r, c, pos.rho());
            position.theta(r, c, pos.theta());
            position.phi(r, c, pos.phi());
        }
    }

    // normalize direction using sound speed at initial location

    wposition position(1, 1);
    position.rho(0, 0, pos.rho());
    position.theta(0, 0, pos.theta());
    position.phi(0, 0, pos.phi());

    matrix<double> c(1, 1);
    _ocean->profile()->sound_speed(position, &c);
    ndirection.rho(ndirection.rho() / c(0, 0), false);
    ndirection.theta(ndirection.theta() / c(0, 0), false);
    ndirection.phi(ndirection.phi() / c(0, 0), false);
}
/*
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
    pos_gradient.phi(
        element_prod(element_div(_c2_r, _sin_theta), ndirection.phi()));

    // update wave propagation direction derivatives
    // Reilly eqns. 39-41

    // clang-format off
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
    // clang-format on

    // update data that relies on new wavefront locations

    if (targets != nullptr) {
        compute_target_distance();
    }
}

/**
 * Search for points on either side of wavefront folds in the
 * D/E direction.
 */
void wave_front::find_edges() {
    on_edge.clear();
    const size_t max_de = num_de() - 1;

    // mark the perimeter of the ray fan
    // also treat the case where num_de()=1 or num_az()=1

    for (size_t az = 0; az < num_az(); ++az) {
        on_edge(0, az) = on_edge(max_de, az) = true;
    }

    // search for a local maxima or minima in the rho direction

    for (size_t az = 0; az < num_az(); az += 1) {
        for (size_t de = 1; de < max_de; de += 1) {
            if ((position.rho(de, az) < position.rho(de + 1, az) &&
                 position.rho(de, az) < position.rho(de - 1, az)) ||
                (position.rho(de, az) > position.rho(de + 1, az) &&
                 position.rho(de, az) > position.rho(de - 1, az))) {
                on_edge(de, az) = true;

                // search for neighboring point with change in direction

                if (abs(ndirection.rho(de, az) - ndirection.rho(de - 1, az)) >
                    abs(ndirection.rho(de, az) - ndirection.rho(de + 1, az))) {
                    on_edge(de - 1, az) = true;
                } else {
                    on_edge(de + 1, az) = true;
                }
            }
        }
    }
}

/*
 * Compute a fast approximation of the distance squared from each
 * target to each point on the wavefront.
 */
void wave_front::compute_target_distance() {
    for (size_t n1 = 0; n1 < targets->size1(); ++n1) {
        for (size_t n2 = 0; n2 < targets->size2(); ++n2) {
            wvector1 from(*targets, n1, n2);
            // clang-format off
            noalias(distance2(n1,n2)) = abs(
                abs2(position.rho()) + from.rho()*from.rho() - 2.0 * from.rho()
                * element_prod( position.rho(), 1.0 - 2.0 * (
                abs2(0.5*(position.theta()-from.theta()))
                + (*_target_sin_theta)(n1,n2) * element_prod( _sin_theta,
                abs2(0.5*(position.phi()-from.phi())) )) ) );
            // clang-format on
        }
    }
}

/**
 * Compute terms in the sound speed profile as fast as possible.
 */
void wave_front::compute_profile() {
    profile_model::csptr profile = _ocean->profile();
    profile->sound_speed(position, &sound_speed, &sound_gradient);
    profile->attenuation(position, _frequencies, distance, &attenuation);
    for (size_t de = 0; de < position.size1(); ++de) {
        for (size_t az = 0; az < position.size2(); ++az) {
            phase(de, az).clear();
        }
    }
}
