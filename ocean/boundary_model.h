/**
 * @file boundary_model.h
 * Generic interface for the ocean's surface or bottom.
 */
#pragma once

#include <usml/ocean/reflect_loss_constant.h>
#include <usml/ocean/scattering_constant.h>

namespace usml {
namespace ocean {

using boost::numeric::ublas::vector;

/// @ingroup boundaries
/// @{

/**
 * A "boundary model" computes the environmental parameters of
 * the ocean's surface or bottom.  The modeled properties include
 * the depth, reflection properties, and reverberation scattering
 * strength of the interface.
 * This class implements a reflection loss model through delegation.
 * The delegated model is defined separately and added to its host
 * during/after construction.  The host is defined as a reflect_loss_model
 * subclass so that it's children can share the reflection loss model
 * through this delegation.
 *
 * This implementation defines the unit normal using Cartesian coordinates
 * in the \f$(\rho,\theta,\phi)\f$ directions relative to its location.
 * Given this definition, the normal can be computed from the depth
 * derivatives or slope angles using:
 * \f[
 *          s_\theta = tan(\sigma_\theta) = \frac{1}{\rho} \frac{ \partial h }{ \partial \theta }
 * \f]\f[
 *          s_\phi = tan(\sigma_\phi) = \frac{1}{\rho sin(\theta)} \frac{ \partial h }{ \partial \phi}
 * \f]\f[
 *          n_\theta = - sin(\sigma_\theta) = - \frac{ s_\theta }{ \sqrt{ 1 + s_\theta^2 } }
 * \f]\f[
 *          n_\phi = - sin(\sigma_\phi) = - \frac{ s_\phi }{ \sqrt{ 1 + s_\phi^2 } }
 * \f]\f[
 *          n_\rho = \sqrt{ 1 - ( n_\theta^2 + n_\phi^2 ) }
 * \f]
 * where:
 *   - \f$ (\rho,\theta,\phi) \f$ = location at which normal is computed,
 *   - \f$ ( \frac{\partial h}{\partial \theta}, \frac{\partial h}{\partial \phi} ) \f$
 *     = depth derivative in the \f$(\rho,\theta)\f$ and \f$(\rho,\phi)\f$ planes (meters/radian),
 *   - \f$ ( s_\theta,s_\phi ) \f$
 *     = slope in the \f$(\rho,\theta)\f$ and \f$(\rho,\phi)\f$ planes (meters/meter),
 *   - \f$ ( \sigma_\theta,\sigma_\phi ) \f$
 *     = slope angle in the \f$(\rho,\theta)\f$ and \f$(\rho,\phi)\f$ planes (radians), and
 *   - \f$ ( n_\rho,n_\theta,n_\phi ) \f$
 *     = unit normal components in "rho","theta", and "phi" directions (meters).
 *
 * This definition of the unit normal saves processing time during
 * reflection processing.
 */
class USML_DECLSPEC boundary_model : public reflect_loss_model, scattering_model {

    //**************************************************
    // height model

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
    virtual void height( const wposition& location,
        matrix<double>* rho, wvector* normal=NULL, bool quick_interp=false ) = 0 ;

    /**
     * Compute the height of the boundary and it's surface normal at
     * a single location.  Often used during reflection processing.
     *
     * @param location      Location at which to compute boundary.
     * @param rho           Surface height in spherical earth coords (output).
     * @param normal        Unit normal relative to location (output).
     * @param quick_interp  Determines if you want a fast nearest or pchip interp
     */
    virtual void height( const wposition1& location,
        double* rho, wvector1* normal=NULL, bool quick_interp=false ) = 0 ;

    //**************************************************
    // reflection loss model

    /**
     * Define a new reflection loss model.
     *
     * @param reflect_loss   Reflection loss model.
     */
    void reflect_loss( reflect_loss_model* reflect_loss ) {
        if ( _reflect_loss ) delete _reflect_loss ;
        _reflect_loss = reflect_loss ;
    }

   /**
     * Computes the broadband reflection loss and phase change.
     *
     * @param location      Location at which to compute attenuation.
     * @param frequencies   Frequencies over which to compute loss. (Hz)
     * @param angle         Grazing angle relative to the interface (radians).
     * @param amplitude     Change in ray strength in dB (output).
     * @param phase         Change in ray phase in dB (output).
     */
    virtual void reflect_loss(
        const wposition1& location,
        const seq_vector& frequencies, double angle,
        vector<double>* amplitude, vector<double>* phase=NULL )
    {
        _reflect_loss->reflect_loss( location,
            frequencies, angle, amplitude, phase ) ;
    }

    //**************************************************
    // reverberation scattering strength model

    /**
     * Define a new reverberation scattering strength model.
     *
     * @param scattering	Scattering model for this boundary
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

    /**
     * Computes the broadband scattering strength for a collection of
     * scattering angles from a common incoming ray. Each scattering
     * has its own location, de_scattered, and az_scattered.
     * The result is a broadband reverberation scattering strength for
     * each scattering.
     *
     * @param location      Location at which to compute attenuation.
     * @param frequencies   Frequencies over which to compute loss. (Hz)
     * @param de_incident   Depression incident angle (radians).
     * @param de_scattered  Depression scattered angle (radians).
     * @param az_incident   Azimuthal incident angle (radians).
     * @param az_scattered  Azimuthal scattered angle (radians).
     * @param amplitude     Reverberation scattering strength ratio (output).
     */
    virtual void scattering( const wposition& location,
        const seq_vector& frequencies, double de_incident, matrix<double> de_scattered,
        double az_incident, matrix<double> az_scattered, matrix< vector<double> >* amplitude )
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
     * @param reflect_loss  Reflection loss model.
     * @param scattering	Reverberation scattering strength model.
     */
    boundary_model( reflect_loss_model* reflect_loss=NULL,
                    scattering_model* scattering=NULL ) :
        _reflect_loss( reflect_loss )
    {
		if ( reflect_loss ) {
			_reflect_loss = reflect_loss ;
		} else {
			_reflect_loss = new reflect_loss_constant( 0.0, 0.0 ) ;
		}
		if ( scattering ) {
			_scattering = scattering ;
		} else {
			_scattering = new scattering_constant() ;
		}
    }

    /**
     * Delete reflection loss model.
     */
    virtual ~boundary_model() {
        if ( _reflect_loss ) delete _reflect_loss ;
        if ( _scattering ) delete _scattering ;
    }

  private:

    /** Reference to the reflection loss model **/
    reflect_loss_model* _reflect_loss ;

    /** Reference to the scattering strength model **/
    scattering_model* _scattering ;

};

/// @}
}  // end of namespace ocean
}  // end of namespace usml
