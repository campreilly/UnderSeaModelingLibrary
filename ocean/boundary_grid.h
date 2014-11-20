/**
 * @file boundary_grid.h
 * Creates a bottom model from a 1-D or 2-D data grid.
 */
#pragma once

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
 * to reduce sudden changes in surface normal direction.  Values outside of the
 * latitude/longitude axes defined by the data grid at limited to the values
 * at the grid edge.
 */
template< class DATA_TYPE, int NUM_DIMS > class boundary_grid
    : public boundary_model
{
    //**************************************************
    // height model

private:

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
     * @param quick_interp  Determines if you want a fast nearest or pchip interp
     */
    virtual void height(const wposition& location, matrix<double>* rho,
        wvector* normal = NULL, bool quick_interp = false) {
        switch (NUM_DIMS) {

        //***************
        // 1-D grids

        case 1:
            if(quick_interp) { this->_height->interp_type(0, GRID_INTERP_LINEAR); }
            else{ this->_height->interp_type(0, GRID_INTERP_PCHIP); }
            if (normal) {

                matrix<double> gtheta(location.size1(), location.size2());
                matrix<double> t(location.size1(), location.size2());
                this->_height->interpolate(location.theta(), rho, &gtheta);
                t = min(element_div(gtheta,*rho),1.0);  // slope = tan(angle)
                normal->theta(                          // normal = -sin(angle)
                    element_div( -t, sqrt(1.0+abs2(t)) ));
                normal->phi(scalar_matrix<double>(location.size1(),location.size2(),0.0));
                normal->rho( sqrt(1.0-abs2(normal->theta())) ) ; // r=sqrt(1-t^2)
            } else {
                this->_height->interpolate(location.theta(), rho);
            }
            break;

            //***************
            // 2-D grids

        case 2:
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
     * @param quick_interp  Determines if you want a fast nearest or pchip interp
     */
    virtual void height(const wposition1& location, double* rho,
        wvector1* normal = NULL, bool quick_interp = false) {
        switch (NUM_DIMS) {

        //***************
        // 1-D grids

        case 1:
            if(quick_interp) { this->_height->interp_type(0, GRID_INTERP_LINEAR); }
            else{ this->_height->interp_type(0, GRID_INTERP_PCHIP); }
            if (normal) {
                double theta = location.theta();
                double gtheta;
                *rho = this->_height->interpolate(&theta, &gtheta);
                const double t = gtheta / (*rho);       // slope = tan(angle)
                normal->theta(-t / sqrt(1.0 + t * t));  // normal = -sin(angle)
                normal->phi(0.0);
                const double N = normal->theta() * normal->theta() ;
                normal->rho( sqrt(1.0-N) );				// r=sqrt(1-t^2)
            } else {
                double theta = location.theta();
                *rho = this->_height->interpolate(&theta);
            }
            break;

            //***************
            // 2-D grids

        case 2:
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
        this->_height->interp_type(0,GRID_INTERP_PCHIP);
        this->_height->interp_type(1,GRID_INTERP_PCHIP);
        this->_height->edge_limit(0,true);
        this->_height->edge_limit(1,true);
        if ( reflect_loss == NULL ) {
            this->reflect_loss( new reflect_loss_rayleigh(
                reflect_loss_rayleigh::SAND) ) ;
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
