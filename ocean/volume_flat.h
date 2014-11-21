/**
 * @file volume_flat.h
 * Models a simple volume reverberation layer in the ocean.
 */
#pragma once

#include <usml/ocean/volume_model.h>

namespace usml {
namespace ocean {

using boost::numeric::ublas::vector;

/// @ingroup boundaries
/// @{

/**
 * Models a simple volume reverberation layer in the ocean
 * with constant depth, thickness, and scattering strength.
 */
class USML_DECLSPEC volume_flat : public volume_model {

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
     * Initialize depth and reflection loss components for a boundary.
     *
     * @param depth         Depth of layer relative to mean sea level.
     * @param thickness     Height of the layer from the bottom to the top.
     * @param amplitude     Reverberation scattering strength ratio.
     */
    volume_flat( double depth=0.0, double thickness=0.0, double amplitude=-300.0  ) :
    	volume_model( new scattering_constant(amplitude) )
    {

    }

    /**
     * Initialize depth and reflection loss components for a boundary.
     *
     * @param depth         Depth of layer relative to mean sea level.
     * @param thickness     Height of the layer from the bottom to the top.
     * @param scattering    Reverberation scattering strength model.
     */
    volume_flat( double depth=0.0, double thickness=0.0, scattering_model* scattering ) :
    	volume_model( scattering )
    {

    }

    //**************************************************
    // initialization

    /**
     * Initialize reflection loss components for a boundary.
     *
     * @param scatter		Reverberation scattering strength model
     */
    volume_flat( scattering_model* scatter=NULL ) :
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
