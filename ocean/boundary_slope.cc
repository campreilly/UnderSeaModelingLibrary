/**
 * @file boundary_slope.cc
 * Models a sloping flat bottom.
 */

#include <usml/ocean/boundary_slope.h>

using namespace usml::ocean;

/**
 * Initialize depth and reflection loss components for a boundary.
 */
boundary_slope::boundary_slope(const wposition1& location, double depth,
                               double lat_slope, double lng_slope,
                               reflect_loss_model::csptr reflect_loss,
                               scattering_model::csptr scattering)
    : boundary_model(reflect_loss,scattering),
      _location(location),
      _height(wposition::earth_radius - depth),
      _normal(0.0, 0.0, 0.0) {
    _normal.theta(sin(lat_slope));  // note the sign change
    _normal.phi(-sin(lng_slope));
    _normal.rho(sqrt(1.0 - _normal.theta() * _normal.theta() -
                     _normal.phi() * _normal.phi()));
}

/**
 * Compute the height of the boundary and it's surface normal at
 * a series of locations.
 */
void boundary_slope::height(const wposition& location, matrix<double>* rho,
                            wvector* normal) const {
    const double alpha = -asin(_normal.theta());
    const double beta = -asin(_normal.phi());
    *rho = element_prod(
        _height * (1.0 + sin(_location.theta()) * tan(beta) *
                             (location.phi() - _location.phi())),
        (1.0 + tan(alpha) * (location.theta() - _location.theta())));

    if (normal != nullptr) {
        normal->rho(scalar_matrix<double>(location.size1(), location.size2(),
                                          _normal.rho()));
        normal->theta(scalar_matrix<double>(location.size1(), location.size2(),
                                            _normal.theta()));
        normal->phi(scalar_matrix<double>(location.size1(), location.size2(),
                                          _normal.phi()));
    }
}

/**
 * Compute the height of the boundary and it's surface normal at
 * a single location.  Often used during reflection processing.
 */
void boundary_slope::height(const wposition1& location, double* rho,
                            wvector1* normal) const {
    const double alpha = -asin(_normal.theta());
    const double beta = -asin(_normal.phi());
    *rho = _height *
           (1.0 + sin(_location.theta()) * tan(beta) *
                      (location.phi() - _location.phi())) *
           (1.0 + tan(alpha) * (location.theta() - _location.theta()));

    if (normal != nullptr) {
        normal->rho(_normal.rho());
        normal->theta(_normal.theta());
        normal->phi(_normal.phi());
    }
}
