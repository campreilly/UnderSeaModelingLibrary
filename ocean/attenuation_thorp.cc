/**
 * @file attenuation_thorp.cc
 * Models attenuation loss using Thorp's model.
 */

#include <usml/ocean/attenuation_thorp.h>
#include <usml/types/wvector.h>

using namespace usml::ocean;

/**
 * Computes the broadband absorption loss of sea water.
 */
void attenuation_thorp::attenuation(
    const wposition& location, seq_vector::csptr frequencies,
    const matrix<double>& distance,
    matrix<vector<double> >* attenuation) const {
    // initialize the cache for the attenuation coefficients
    vector<double> alpha(frequencies->size());
    for (size_t f = 0; f < frequencies->size(); ++f) {
        double F2 = (*frequencies)(f);
        F2 = 1e-6 * F2 * F2;
        alpha(f) = 1e-3 *
                   (3.3e-3 +
                    F2 * (0.11 / (1.0 + F2) + 44.0 / (4100.0 + F2) + 3.0e-4)) /
                   (1.0 - 5.88264e-6 * 1000.0);
    }
    // apply attenuation coefficients and depth corrections
    for (size_t row = 0; row < location.size1(); ++row) {
        for (size_t col = 0; col < location.size2(); ++col) {
            (*attenuation)(row, col) =
                distance(row, col) * alpha *
                (1.0 + 5.88264e-6 * location.altitude(row, col));
        }
    }
}
