/**
 * @file volume_model.h
 * Generic interface for volume scattering layers.
 */

#ifndef USML_OCEAN_volume_model_H
#define USML_OCEAN_volume_model_H

#include <usml/ocean/scattering_constant.h>

namespace usml {
namespace ocean {

using boost::numeric::ublas::vector;

/// @ingroup boundaries
/// @{

/**
 * A "volume scattering layer model" computes the environmental parameters of
 * a single volume scattering layer in the ocean.  The modeled properties
 * include the depth, thickness, and reverberation scattering strength
 * of the layer.  Depth is defined using the average (center) distance
 * of the layer relative to the center of the earth.  Thickness is
 * the full distance from the bottom to the top of the layer.
 */
class USML_DECLSPEC volume_model : public scattering_model {

	// @todo Resolve differences between interface and volume scattering strength

    //**************************************************
    // depth model

  public:

    /**
     * Compute the depth of the layer and it's thickness at
     * a series of locations.  Often used to detect collisions
     * between ray paths and the volume scattering layer.
     *
     * @param location      Location at which to compute boundary.
     * @param rho           Depth of layer center in spherical earth coords (output).
     * @param thickness     Layer thickness (output).
     */
    virtual void depth( const wposition& location,
        matrix<double>* rho, matrix<double>* thickness=NULL ) = 0 ;

    /**
     * Compute the depth of the layer and it's thickness at
     * a single location.  Often used to process the reverberation
     * for individual collisions with volume scattering layer.
     *
     * @param location      Location at which to compute boundary.
     * @param rho           Depth of layer center in spherical earth coords (output).
     * @param thickness     Layer thickness (output).
     */
    virtual void depth( const wposition1& location,
        double* rho, double* thickness=NULL ) = 0 ;

    //**************************************************
    // reverberation scattering strength model

    /**
     * Define a new reverberation scattering strength model.
     *
     * @param scattering	Scattering model for this layer
     */
    void scattering( scattering_model* scattering ) {
        if( _scattering ) delete _scattering ;
        _scattering = scattering ;
    }

    /**
     * Computes the broadband scattering strength for a single location.
     *
     * @param location      Location at which to compute attenuation.
     * @param frequencies   Frequencies over which to compute loss. (Hz)
     * @param de_incident   Depression incident angle (radians).
     * @param de_scattered  Depression scattered angle (radians).
     * @param az_incident   Azimuthal incident angle (radians).
     * @param az_scattered  Azimuthal scattered angle (radians).
     * @param amplitude     Change in ray strength in dB (output).
     */
    virtual void scattering( const wposition1& location,
        const seq_vector& frequencies, double de_incident, double de_scattered,
        double az_incident, double az_scattered, vector<double>* amplitude )
    {
    	_scattering->scattering( location,
    			frequencies, de_incident, de_scattered,
				az_incident, az_scattered, amplitude ) ;
    }

    //**************************************************
    // initialization

    /**
     * Initialize reflection loss components for a boundary.
     *
     * @param scatter		Reverberation scattering strength model
     */
    volume_model( scattering_model* scatter=NULL ) :
        _scattering( scatter )
    {
		if ( scatter ) {
			_scattering = scatter ;
		} else {
			_scattering = new scattering_constant() ;
		}
    }

    /**
     * Delete reflection loss model.
     */
    virtual ~volume_model() {
        if ( _scattering ) delete _scattering ;
    }

  private:

    /** Reference to the scattering strength model **/
    scattering_model* _scattering ;

};

/// @}
}  // end of namespace ocean
}  // end of namespace usml

#endif
