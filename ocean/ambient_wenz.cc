/**
 * @file ambient_wenz.cc
 * Wenz model for ambient noise in the ocean.
 */

#include <usml/ocean/ambient_wenz.h>
#include <usml/ublas/vector_math.h>

using namespace usml::ublas;
using namespace usml::ocean;

/**
 * Computes the power spectral density of ambient noise.
 */
void ambient_wenz::ambient(const wposition1& /*location*/,
                           const seq_vector::csptr& frequency,
                           vector<double>* noise) const {
    vector<double> nothing = scalar_vector<double>(frequency->size(), -300.0);
    vector<double> logf = frequency->data();
    logf = log10(logf);

    // compute turbulence and thermal noise components in dB as polynomials

    vector<double> turbulence = 107.0 - 30.0 * logf;
    vector<double> thermal = -75.0 + 20.0 * logf;

    // compute shipping noise in dB as polynomial

    vector<double> shipping = nothing;
    if (_shipping_level >= 1 && _shipping_level <= 7) {
        shipping = 76.0 - 20.0 * pow(logf - log10(30), 2.0) +
                   5.0 * (_shipping_level - 4.0);
    }

    // compute surface wind noise in dB as polynomial

    vector<double> wind = nothing;
    if (_wind_speed >= 0) {
        double knots = _wind_speed / 0.51444444;  // wind speed in knots
        for (int n = 0; n < frequency->size(); ++n) {
            if ((*frequency)(n) < 1000) {
                wind(n) = 44.0 + sqrt(21.0 * knots) +
                          17.0 * (3 - logf(n)) * (logf(n) - 2);
            } else {
                wind(n) = 95.0 + sqrt(21.0 * knots) - 17. * logf(n);
            }
        }
    }

    // compute rain  noise in dB as polynomial

    vector<double> rain = nothing;
    if (_rain_rate > 0 && _rain_rate <= 3) {
        for (int n = 0; n < frequency->size(); ++n) {
            if ((*frequency)(n) >= 550.0 && (*frequency)(n) <= 15500.0) {
                double f = logf(n);
                switch (_rain_rate) {
                    case 1:
                        rain(n) = 64.942 + (19.917 - 5.242 * f) * f;
                        break;
                    case 2:
                        rain(n) = 19.628 + (42.933 - 7.516 * f) * f;
                        break;
                    case 3:
                        rain(n) =
                            222.491 - (135.904 - (43.893 - 4.737 * f) * f) * f;
                        break;
                    default:
                        break;
                }
            }
        }
    }

    // power sum contributions and return level in intensity units

    turbulence = pow(10.0, 0.1 * turbulence);
    thermal = pow(10.0, 0.1 * thermal);
    shipping = pow(10.0, 0.1 * shipping);
    wind = pow(10.0, 0.1 * wind);
    rain = pow(10.0, 0.1 * rain);
    *noise = turbulence + thermal + shipping + wind + rain;
}
