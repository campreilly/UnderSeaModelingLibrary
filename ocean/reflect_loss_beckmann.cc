/**
 * @file reflect_loss_beckmann.cc
 * Models plane wave reflection from a flat fluid-solid interface.
 */
#include <usml/ocean/reflect_loss_beckmann.h>

using namespace usml::ocean;

/**
 * Computes the broadband reflection loss and phase change.
 *
 * @param location      Location at which to compute reflection loss.
 * @param frequencies   Frequencies over which to compute loss. (Hz).
 * @param angle         Grazing angle relative to the interface (radians).
 * @param amplitude     Change in ray intensity in dB (output).
 * @param phase         Change in ray phase in radians (output).
 * 						Hard-coded to a value of PI for this model.
 * 						Phase change not computed if this is nullptr.
 */
void reflect_loss_beckmann::reflect_loss(const wposition1& /*location*/,
                                         seq_vector::csptr frequencies,
                                         double angle,
                                         vector<double>* amplitude,
                                         vector<double>* phase) const {
    const double theta = max(1e-10, abs(angle));
    const double at2 = theta * theta / (0.006 + 10.2e-3 * _wind_speed);
    const double v3 =
        min(0.99,
            sin(abs(theta)) * max(0.5, 1.0 - exp(-at2 / 4) / sqrt(M_PI * at2)));
    const double w4 = _wind_speed * _wind_speed * _wind_speed * _wind_speed;

    noalias(*amplitude) = *frequencies;  // copy sequence into vector
    *amplitude =
        -20.0 * log10(sqrt(1.0 - v3) *
                      (0.3 + 0.7 / (1.0 + 6.0e-11 * w4 * abs2(*amplitude))));
    if (phase != nullptr) {
        noalias(*phase) = scalar_vector<double>(frequencies->size(), M_PI);
    }
}
