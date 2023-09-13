/**
 * @file attenuation_constant.cc
 * Models attenuation loss as a constant factor.
 */

#include <usml/ocean/attenuation_constant.h>
#include <usml/types/wvector.h>

using namespace usml::ocean;

/**
 * Computes the broadband absorption loss of sea water.
 */
void attenuation_constant::attenuation(
    const wposition& location, const seq_vector::csptr& frequencies,
    const matrix<double>& distance,
    matrix<vector<double> >* attenuation) const {
    for (size_t row = 0; row < location.size1(); ++row) {
        for (size_t col = 0; col < location.size2(); ++col) {
            for (size_t f = 0; f < frequencies->size(); ++f) {
                (*attenuation)(row, col)(f) =
                    _coefficient * distance(row, col) * (*frequencies)(f);
            }
        }
    }
}
