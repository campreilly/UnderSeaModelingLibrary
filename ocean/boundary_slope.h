/** 
 * @file boundary_slope.h
 * Models a sloping flat bottom.
 */

#ifndef USML_OCEAN_BOUNDARY_SLOPE_H
#define USML_OCEAN_BOUNDARY_SLOPE_H

#include <usml/ocean/boundary_model.h>

namespace usml {
namespace ocean {

/// @ingroup boundaries
/// @{

/**
 * Models a sloping flat bottom. The surface normal makes a constant angle 
 * to the theta and phi components of the current position.  Given this 
 * definition of surface normal,  the change in height from the 
 * reference point is given by:
 * 
 * \f[
 *          \rho_1 = \rho_0 + \rho_0 sin(\theta) tan(slope_\phi) \Delta \phi
 * \f]\f[
 *          \rho_{calc} = \rho_1 + \rho_1 tan(slope_\theta) \Delta \theta
 * \f] 
 * where:
 *   - \f$ (\rho,\theta,\phi) \f$ = reference point at which slope is defined.
 *   - \f$ \rho_0       \f$ = surface height (meters) at reference point.
 *   - \f$ slope_\theta \f$ = surface slope in "theta" direction (radians).
 *   - \f$ slope_\phi   \f$ = surface slope in "phi" direction (radians).
 *   - \f$ (\Delta \theta,\Delta \phi) \f$ = offset from reference point.
 *   - \f$ \rho_{calc}  \f$ = calculated height (meters) at new location.
 */
class USML_DECLSPEC boundary_slope : public boundary_model {

    //**************************************************
    // height model

  private:

    /** Location at which initial depth and slope are specified. */
    const wposition1& _location ;

    /** Surface height in spherical earth coords. */
    const double _height ;

    /** Surface normal. */
    wvector1 _normal ;

  public:

    /**
     * Compute the height of the boundary and it's surface normal at
     * a series of locations.
     *
     * @param location      Location at which to compute boundary.
     * @param rho           Surface height in spherical earth coords (output).
     * @param normal        Unit normal relative to location (output).
     */
    virtual void height( const wposition& location, 
        matrix<double>* rho, wvector* normal=NULL ) ;

    /**
     * Compute the height of the boundary and it's surface normal at
     * a single location.  Often used during reflection processing.
     *
     * @param location      Location at which to compute boundary.
     * @param rho           Surface height in spherical earth coords (output).
     * @param normal        Unit normal relative to location (output).
     */
    virtual void height( const wposition1& location, 
        double* rho, wvector1* normal=NULL ) ;

    //**************************************************
    // initialization

    /**
     * Initialize depth and reflection loss components for a boundary.
     * 
     * @param location      Location at which initial depth and slope
     *                      are specified.
     * @param depth         Water depth (meters) at this location.
     * @param lat_slope     Slope angle in latitude direction (radians).
     * @param lng_slope     Slope angle in longitude direction (radians).
     * @param reflect_loss  Reflection loss model.  Assumes depth=0 is used to
     *                      define the water surface and any other depths 
     *                      define the ocean bottom. Use perfect surface or 
     *                      bottom reflection if no model specified.
     *                      The boundary_model takes over ownship of this
     *                      reference and deletes it as part of its 
     *                      destructor.
     */
    boundary_slope( const wposition1& location, 
        double depth, double lat_slope, double lng_slope=0.0, 
        reflect_loss_model* reflect_loss=NULL ) ;

};

/// @}
}  // end of namespace ocean
}  // end of namespace usml

#endif
