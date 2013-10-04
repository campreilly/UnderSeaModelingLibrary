/**
 * @file boundary_grid_fast.h
 * Creates a bottom model from a 1-D or 2-D data grid.
 */

#ifndef USML_OCEAN_BOUNDARY_GRID_FAST_H
#define USML_OCEAN_BOUNDARY_GRID_FAST_H

#include <usml/ocean/boundary_model.h>
#include <usml/ocean/reflect_loss_rayleigh.h>

namespace usml {
namespace ocean {

/**
 * Bottom model constructed from 2-D data_grid_fast_2d.
 * The coordinate system for each kind of data set is:
 *
 *      - 2-D: Assumes that the order of axes in the grid is
 *             (latitude, longitude) and that the geodetic
 *             axes have been transformed to their spherical earth
 *             equivalents (theta,phi).
 *
 * Uses the GRID_INTERP_PCHIP interpolation in both directions
 * to reduce sudden changes in surface normal direction.  Values outside of the
 * latitude/longitude axes defined by the data grid at limited to the values
 * at the grid edge.
 */
class boundary_grid_fast : public boundary_model
{
    //**************************************************
    // height model

protected:

    /** Boundary for all locations. */
    data_grid_fast_2d* _height;

public:

    /**
     * Compute the height of the boundary and it's surface normal at
     * a series of locations.
     *
     * @param location      Location at which to compute boundary.
     * @param rho           Surface height in spherical earth coords (output).
     * @param normal        Unit normal relative to location (output).
     * @param quick_interp  Determines if you want a fast nearest or pchip interp
     */
    virtual void height(const wposition& location, matrix<double>* rho,
        wvector* normal = NULL, bool quick_interp = false) {
            if(quick_interp) {
                this->_height->interp_type(0, GRID_INTERP_LINEAR);
                this->_height->interp_type(1, GRID_INTERP_LINEAR);
            } else{
                this->_height->interp_type(0, GRID_INTERP_PCHIP);
                this->_height->interp_type(1, GRID_INTERP_PCHIP);
            }
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
                normal->rho(sqrt(           	// r=sqrt(1-t^2-p^2)
                		1.0 - abs2(normal->theta()) - abs2(normal->phi()) ));
            } else {
                this->_height->interpolate(location.theta(), location.phi(),
                    rho);
            }
    }

    /**
     * Compute the height of the boundary and it's surface normal at
     * a single location.  Often used during reflection processing.
     *
     * @param location      Location at which to compute boundary.
     * @param rho           Surface height in spherical earth coords (output).
     * @param normal        Unit normal relative to location (output).
     * @param quick_interp  Determines if you want a fast nearest or pchip interp
     */
    virtual void height(const wposition1& location, double* rho,
        wvector1* normal = NULL, bool quick_interp = false) {
            if(quick_interp) {
                this->_height->interp_type(0, GRID_INTERP_LINEAR);
                this->_height->interp_type(1, GRID_INTERP_LINEAR);
            } else{
                this->_height->interp_type(0, GRID_INTERP_PCHIP);
                this->_height->interp_type(1, GRID_INTERP_PCHIP);
            }
            if (normal) {
                double loc[2] = { location.theta(), location.phi() };
                double grad[2];
                *rho = this->_height->interpolate(loc, grad);
                const double t = grad[0] / (*rho);      // slope = tan(angle)
                const double p = grad[1] / ((*rho) * sin(location.theta()));
                normal->theta(-t / sqrt(1.0 + t * t));  // normal = -sin(angle)
                normal->phi(-p / sqrt(1.0 + p * p));
                const double N = normal->theta() * normal->theta()
                               + normal->phi()  * normal->phi();
                normal->rho( sqrt(1.0-N) );				// r=sqrt(1-t^2-p^2)
            } else {
                double loc[2] = { location.theta(), location.phi() };
                *rho = this->_height->interpolate(loc);
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
    boundary_grid_fast(data_grid_fast_2d* height,
        reflect_loss_model* reflect_loss = NULL) :
        boundary_model(reflect_loss), _height(height) {
        this->_height->interp_type(0,GRID_INTERP_PCHIP);
        this->_height->interp_type(1,GRID_INTERP_PCHIP);
        this->_height->edge_limit(0,true);
        this->_height->edge_limit(1,true);
        if (_reflect_loss_model == NULL) {
            _reflect_loss_model = new reflect_loss_rayleigh(
                reflect_loss_rayleigh::SAND);
        }
    }

    /**
     * Delete boundary grid.
     */
    virtual ~boundary_grid_fast() {
        delete _height;
    }

};

}  // end of namespace ocean
}  // end of namespace usml

#endif

