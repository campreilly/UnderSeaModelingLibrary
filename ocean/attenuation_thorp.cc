/** 
 * @file attenuation_thorp.cc
 * Models attenuation loss as a Thorp factor.
 */

#include <usml/ocean/attenuation_thorp.h>

using namespace usml::ocean;

/**
 * Computes the broadband absorption loss of sea water.
 */
void attenuation_thorp::attenuation(
        const wposition& location,
        const seq_vector& frequencies,
        const matrix<double>& distance,
        matrix< vector<double> >* attenuation) {

	// initialize the cache for the attenuation coefficients
    vector <double> alpha(frequencies.size());
    for (unsigned f = 0; f < frequencies.size(); ++f) {
		double F2 = frequencies(f);
		F2 = 1e-6 * F2 * F2;
		alpha(f) = 1e-3 *
			(3.3e-3 + F2 * (0.11 / (1.0 + F2)
			+ 44.0 / (4100.0 + F2) + 3.0e-4))
			/ (1.0 - 5.88264e-6 * 1000.0);
    }
    // apply attenuation coefficients and depth corrections
    for (unsigned row = 0; row < location.size1(); ++row) {
        for (unsigned col = 0; col < location.size2(); ++col) {
            (*attenuation)(row, col) =
                distance(row, col)
                * alpha
                * (1.0 + 5.88264e-6 * location.altitude(row, col)) ;
        }
    }
}
