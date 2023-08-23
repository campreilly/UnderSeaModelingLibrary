/**
 * @file bp_arb.cc
 * Models a beam pattern based on arbitrary 3D element locations.
 */

#include <usml/beampatterns/bp_arb.h>

#include <boost/numeric/ublas/vector_expression.hpp>
#include <cmath>
#include <ext/type_traits.h>

using namespace usml::beampatterns;

/**
 * Computes the beam level gain for an arrival vector in the body coordinates
 * of the array,
 */
void bp_arb::beam_level(const bvector& arrival, const seq_vector::csptr& frequencies,
                        vector<double>* level, const bvector& steering,
                        double sound_speed) const {
    // set gain to zero in backplane when baffle is on

    if (_back_baffle) {
        if (arrival.front() <= 0.0) {
            *level *= 0.0;
            return;
        }
    }

    // normalize power to peak of one

    double scale = abs(sum(_weights));
    scale = 1.0 / (scale * scale);

    // wave number scaling

    vector<double> kscale = frequencies->data();
    kscale *= (2.0 * M_PI / sound_speed);

    // compute IDFT for each requested frequency
    //
    // Note: looping over frequency turns out to be about 40% faster
    // than computing with asignal() as a vector of frequencies.

    vector<double> diff = arrival.data() - steering.data();
    for (unsigned f = 0; f < frequencies->size(); f++) {
        complex<double> A(0.0, -kscale[f]);
        complex<double> acc(0.0);
        for (unsigned n = 0; n < _N_elements; n++) {
            matrix_row<const matrix<double> > locs(_elem_locs, n);
            acc += _weights(n) * exp(A * inner_prod(diff, locs));
        }
        (*level)(f) = std::norm(acc) * scale;
    }
}
