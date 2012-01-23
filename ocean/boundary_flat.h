/** 
 * @file boundary_flat.h
 * Models any flat boundary in the ocean including the ocean surface.
 */

#ifndef USML_OCEAN_BOUNDARY_FLAT_H
#define USML_OCEAN_BOUNDARY_FLAT_H

#include <usml/ocean/boundary_model.h>

namespace usml {
namespace ocean {

/// @ingroup boundaries
/// @{

/**
 * Models any flat boundary in the ocean including the ocean surface.
 */
class boundary_flat : public boundary_model {

    //**************************************************
    // height model

  private:

    /** Surface height in spherical earth coords. */
    const double _height ;

    /** Rho component of the surface normal. */
    double _normal_rho ;

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
     * @param depth         Water depth relative to mean sea level.
     * @param reflect_loss  Reflection loss model.  Assumes depth=0 is used to
     *                      define the water surface and any other depths 
     *                      define the ocean bottom. Use perfect surface or 
     *                      bottom reflection if no model specified.
     *                      The boundary_model takes over ownship of this
     *                      reference and deletes it as part of its 
     *                      destructor.
     */
    boundary_flat(double depth=0.0, reflect_loss_model* reflect_loss=NULL );

};

/// @}
}  // end of namespace ocean
}  // end of namespace usml

#endif
