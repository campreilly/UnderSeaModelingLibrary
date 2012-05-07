/** 
 * @file boundary_grid.h
 * Creates a bottom model from a 1-D or 2-D data grid.
 */

#ifndef USML_OCEAN_BOUNDARY_GRID_H
#define USML_OCEAN_BOUNDARY_GRID_H

#include <usml/ocean/boundary_model.h>
#include <usml/ocean/reflect_loss_rayleigh.h>

namespace usml {
namespace ocean {

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
 * Uses the GRID_INTERP_PCHIP interpolation in both directions
 * to reduce sudden changes in surface normal direction.
 *
 * Clips the radial component of the slope to zero if the sum of the
 * squares of the theta and phi components are greater than 1.
 * This shouldn't happen in realistic bathymetry, but can occur when
 * high frequency jitter in the underlying database create huge gradients.
 * In that case, there can be a round off error in the calculation
 * of the sum of the squares.
 */
template< class DATA_TYPE, int NUM_DIMS > class boundary_grid 
    : public boundary_model 
{
    //**************************************************
    // height model

protected:

    /** Boundary for all locations. */
    data_grid<DATA_TYPE, NUM_DIMS>* _height;

public:

    /**
     * Compute the height of the boundary and it's surface normal at
     * a series of locations.
     *
     * @param location      Location at which to compute boundary.
     * @param rho           Surface height in spherical earth coords (output).
     * @param normal        Unit normal relative to location (output).
     */
    virtual void height(const wposition& location, matrix<double>* rho,
        wvector* normal = NULL) {
        switch (NUM_DIMS) {

        //***************
        // 1-D grids

        case 1:
            if (normal) {

                matrix<double> gtheta(location.size1(), location.size2());
                matrix<double> t(location.size1(), location.size2());
                this->_height->interpolate(location.theta(), rho, &gtheta);
                t = min(element_div(gtheta,*rho),1.0);  // slope = tan(angle)
                normal->theta(                          // normal = -sin(angle)
                    element_div( -t, sqrt(1.0+abs2(t)) ));
                normal->phi(scalar_matrix<double>(location.size1(),location.size2(),0.0));
                normal->rho(sqrt(max(                   // r=sqrt(1-t^2)
                    1.0 - abs2(normal->theta()), 0.0 )));
            } else {
                this->_height->interpolate(location.theta(), rho);
            }
            break;

            //***************
            // 2-D grids

        case 2:
            if (normal) {
                matrix<double> gtheta(location.size1(), location.size2());
                matrix<double> gphi(location.size1(), location.size2());
                matrix<double> t(location.size1(), location.size2());
                matrix<double> p(location.size1(), location.size2());
                this->_height->interpolate(location.theta(), location.phi(),
                    rho, &gtheta, &gphi);

                t = element_div(gtheta, *rho);  // slope = tan(angle)
                p = element_div(gphi, element_prod(*rho, sin(location.theta())));
                normal->theta(                  // normal = -sin(angle)
                    element_div( -t, sqrt(1.0+abs2(t)) ));
                normal->phi(
                    element_div( -p, sqrt(1.0+abs2(p)) ));
                normal->rho(sqrt(max(           // r=sqrt(1-t^2-p^2)
                        1.0 - abs2(normal->theta()) - abs2(normal->phi()),
                        0.0)));
            } else {
                this->_height->interpolate(location.theta(), location.phi(),
                    rho);
            }
            break;

            //***************
            // error

        default:
            throw std::invalid_argument("dataset must be 1-D or 2-D");
            break;
        }
    }

    /**
     * Compute the height of the boundary and it's surface normal at
     * a single location.  Often used during reflection processing.
     *
     * @param location      Location at which to compute boundary.
     * @param rho           Surface height in spherical earth coords (output).
     * @param normal        Unit normal relative to location (output).
     */
    virtual void height(const wposition1& location, double* rho,
        wvector1* normal = NULL) {
        switch (NUM_DIMS) {

        //***************
        // 1-D grids

        case 1:
            if (normal) {
                const double theta = location.theta();
                float gtheta;
                *rho = this->_height->interpolate(&theta, &gtheta);
                const double t = gtheta / (*rho);       // slope = tan(angle)
                normal->theta(-t / sqrt(1.0 + t * t));  // normal = -sin(angle)
                normal->phi(0.0);
                normal->rho(sqrt(max(                   // r=sqrt(1-t^2-p^2)
                    1.0 - normal->theta() * normal->theta(),
                    0.0 )));
            } else {
                const double theta = location.theta();
                *rho = this->_height->interpolate(&theta);
            }
            break;

            //***************
            // 2-D grids

        case 2:
            if (normal) {
                const double loc[2] = { location.theta(), location.phi() };
                float grad[2];
                *rho = this->_height->interpolate(loc, grad);
                const double t = grad[0] / (*rho);      // slope = tan(angle)
                const double p = grad[1] / ((*rho) * sin(location.theta()));
                normal->theta(-t / sqrt(1.0 + t * t));  // normal = -sin(angle)
                normal->phi(-p / sqrt(1.0 + p * p));

                double N = normal->theta() * normal->theta()
                         + normal->phi()  * normal->phi();
                if (N < 1.0) {                          // r=sqrt(1-t^2-p^2)
                    normal->rho(1.0 - N);
                } else {            // fix roundoff errors when r is nearly zero
                    N = sqrt(N);
                    normal->rho(0.0);
                    normal->theta(normal->theta() / N);
                    normal->phi(normal->phi() / N);
                }
//                cout << "gradient=" << grad[0] << "," << grad[1] << " slope="
//                    << t << "," << p << " norm=" << normal->theta() << ","
//                    << normal->phi() << endl;

            } else {
                const double loc[2] = { location.theta(), location.phi() };
                *rho = this->_height->interpolate(loc);
            }
            break;

            //***************
            // error

        default:
            throw std::invalid_argument("bathymetry must be 1-D or 2-D");
            break;
        }
    }

    //**************************************************
    // initialization

    /**
     * Initialize depth and reflection loss components for a boundary.
     * 
     * @param height            Bottom depth (meters) as a function of position.
     *                          Assumes control of this grid and deletes
     *                          it when the class is destroyed.
     * @param reflect_loss      Reflection loss model.  Defaults to a
     *                          Rayleigh reflection for "sand" if NULL.
     *                          The boundary_model takes over ownship of this
     *                          reference and deletes it as part of its destructor.
     */
    boundary_grid(data_grid<DATA_TYPE, NUM_DIMS>* height,
        reflect_loss_model* reflect_loss = NULL) :
        boundary_model(reflect_loss), _height(height) {
        this->_height->interp_type(0, GRID_INTERP_PCHIP);
        this->_height->interp_type(1, GRID_INTERP_PCHIP);
        if (_reflect_loss_model == NULL) {
            _reflect_loss_model = new reflect_loss_rayleigh(
                reflect_loss_rayleigh::SAND);
        }
    }

    /**
     * Delete boundary grid.
     */
    virtual ~boundary_grid() {
        delete _height;
    }

};

}  // end of namespace ocean
}  // end of namespace usml

#endif
