/**
 * @file boundary_grid.h
 * Creates a bottom model from a 1-D or 2-D data grid.
 */
#pragma once

#include <usml/ocean/boundary_model.h>
#include <usml/ocean/reflect_loss_model.h>
#include <usml/ocean/reflect_loss_rayleigh.h>
#include <usml/types/data_grid.h>
#include <usml/types/wposition.h>
#include <usml/types/wposition1.h>
#include <usml/types/wvector.h>
#include <usml/types/wvector1.h>
#include <usml/ublas/matrix_math.h>

#include <boost/numeric/ublas/matrix.hpp>

namespace usml {
namespace ocean {

using namespace usml::types;

/**
 * Bottom model constructed from a 1-D or 2-D data grid.
 * The coordinate system for each kind of data set is:
 *
 *      - 1-D: Assumes that the bottom depth is a function of latitude and
 *             that the geodetic axes have been transformed to
 *             their spherical earth equivalents (theta).
 *      - 2-D: Assumes that the order of axes in the grid is
 *             (latitude, longitude) and that the geodetic
 *             axes have been transformed to their spherical earth
 *             equivalents (theta,phi).
 *
 * Uses the interp_enum::pchip interpolation in both directions
 * to reduce sudden changes in surface normal direction.  Values outside of the
 * latitude/longitude axes defined by the data grid at limited to the values
 * at the grid edge.
 */
template <size_t NUM_DIMS>
class boundary_grid : public boundary_model {
   public:
    /**
     * Initialize depth and reflection loss components for a boundary.
     *
     * @param height            Bottom depth (meters) as a function of position.
     * @param reflect_loss      Reflection loss model.  Defaults to a
     *                          Rayleigh reflection for "sand" if nullptr.
     * @param scattering    	Reverberation scattering strength model.
     */
    boundary_grid(typename data_grid<NUM_DIMS>::csptr height,
                  const reflect_loss_model::csptr& reflect_loss = nullptr,
                  const scattering_model::csptr& scattering = nullptr)
        : boundary_model(reflect_loss, scattering), _height(height) {
        if (reflect_loss == nullptr) {
            this->reflect_loss(reflect_loss_model::csptr(
                new reflect_loss_rayleigh(bottom_type_enum::sand)));
        }
    }

    /**
     * Compute the height of the boundary and it's surface normal at
     * a series of locations. The interpolation of the height returns
     * the gradients in the theta (co-latitude) and phi (longitude) directions
     * in units of meters per radian. To scale these gradients into unitless
     * values (meters per meter), we must scale them by the meters per radian
     * in the theta and phi directions.
     * \f[
     * 		\frac{dz}{dx} = \frac{ \frac{dz}{d\phi} }{R \sin{\theta} }
     * \f]\f[
     * 		\frac{dz}{dy} = \frac{ \frac{dz}{d\theta} }{R}
     * \f]
     * The surface normal vector in the \f$\rho, \theta, \phi\f$ directions is
     * given by
     * \f[
     * 		\vec{N} = \left( 1, \frac{dz}{dy}, \frac{dz}{dx} \right)
     * \f]
     * normalized to unit length.
     *
     * @xref Weisstein, Eric W. "Normal Vector." From MathWorld--A Wolfram Web
     * Resource. https://mathworld.wolfram.com/NormalVector.html
     *
     * @param location      Location at which to compute boundary.
     * @param rho           Surface height in spherical earth coords (output).
     * @param normal        Unit normal relative to location (output).
     */
    void height(const wposition& location, matrix<double>* rho,
                wvector* normal = nullptr) const override {
        switch (NUM_DIMS) {
            case 1:
                if (normal) {
                    matrix<double> gtheta(location.size1(), location.size2());
                    matrix<double> t(location.size1(), location.size2());
                    matrix<double> N(location.size1(), location.size2());

                    _height->interpolate(location.theta(), rho, &gtheta);

                    t = -gtheta / *rho;
                    N = sqrt(1 + t * t);
                    normal->rho(1.0 / N);
                    normal->theta(t / N);
                    normal->phi(scalar_matrix<double>(location.size1(),
                                                      location.size2(), 0.0));
                } else {
                    _height->interpolate(location.theta(), rho);
                }
                break;

            case 2:
                if (normal) {
                    matrix<double> gtheta(location.size1(), location.size2());
                    matrix<double> gphi(location.size1(), location.size2());
                    matrix<double> t(location.size1(), location.size2());
                    matrix<double> p(location.size1(), location.size2());
                    matrix<double> N(location.size1(), location.size2());

                    _height->interpolate(location.theta(), location.phi(), rho,
                                         &gtheta, &gphi);

                    t = element_div(-gtheta, *rho);
                    p = element_div(-gphi,
                                    element_prod(*rho, sin(location.theta())));
                    N = sqrt(1 + t * t + p * p);
                    normal->rho(1.0 / N);
                    normal->theta(t / N);
                    normal->phi(p / N);

                } else {
                    _height->interpolate(location.theta(), location.phi(), rho);
                }
                break;

            default:
                throw std::invalid_argument("dataset must be 1-D or 2-D");
                break;
        }
    }

    /**
     * Compute the height of the boundary and it's surface normal at
     * a single location.  Often used during reflection processing.
     *
     * @see boundary_grid::height
     *
     * @param location      Location at which to compute boundary.
     * @param rho           Surface height in spherical earth coords (output).
     * @param normal        Unit normal relative to location (output).
     */
    void height(const wposition1& location, double* rho,
                wvector1* normal = nullptr) const override {
        switch (NUM_DIMS) {
            case 1:
                if (normal) {
                    double theta = location.theta();
                    double gtheta;
                    *rho = _height->interpolate(&theta, &gtheta);

                    const double t = -gtheta / (*rho);
                    const double N = sqrt(1 + t * t);
                    normal->rho(1.0 / N);
                    normal->theta(t / N);
                    normal->phi(0.0);

                } else {
                    double theta = location.theta();
                    *rho = _height->interpolate(&theta);
                }
                break;

            case 2:
                if (normal) {
                    double loc[2] = {location.theta(), location.phi()};
                    double grad[2];

                    *rho = _height->interpolate(loc, grad);

                    const double t = -grad[0] / (*rho);
                    const double p =
                        -grad[1] / ((*rho) * sin(location.theta()));
                    const double N = sqrt(1 + t * t + p * p);
                    normal->rho(1.0 / N);
                    normal->theta(t / N);
                    normal->phi(p / N);

                } else {
                    double loc[2] = {location.theta(), location.phi()};
                    *rho = _height->interpolate(loc);
                }
                break;

            default:
                throw std::invalid_argument("bathymetry must be 1-D or 2-D");
                break;
        }
    }

   private:
    /** Boundary for all locations. */
    typename data_grid<NUM_DIMS>::csptr _height;
};

}  // end of namespace ocean
}  // end of namespace usml
