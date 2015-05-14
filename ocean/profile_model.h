/** 
 * @file profile_model.h
 * Generic interface for ocean water models.
 */
#pragma once

#include <usml/ocean/attenuation_thorp.h>

namespace usml {
namespace ocean {

using boost::numeric::ublas::vector;

/// @ingroup profiles
/// @{

/**
 * A "profile model" computes the environmental parameters of 
 * ocean water. The modeled properties include the sound velocity 
 * profile and the attenuation due to sea water absorption.
 * This class implements an attenuation model through delegation.
 * The delegated model is defined separately and added to its host
 * during/after construction.  The host is defined as an attenuation_model 
 * subclass so that its children can share the attenuation model 
 * through this delegation.
 */
class USML_DECLSPEC profile_model : public attenuation_model {

public:

    /**
     * Constructor - Initialize component models within ocean profile.
     *
     * @param attenuation   In-water attenuation model.
     *                      Uses Thorp model if none specified.
     */
    profile_model( attenuation_model* attenuation = NULL ) : _flat_earth(false) {
        if ( attenuation ) {
            _attenuation = attenuation ;
        } else {
            _attenuation = new attenuation_thorp() ;
        }
    }

    /**
     * Destructor - Delete attenuation model.
     */
    virtual ~profile_model() {
        delete _attenuation ;
    }

    /**
     * Anti-correction term to make the earth seem flat.
     * Used to compare propagation effect to models that have 
     * been computed in rectangular coordinates.  
     *
     * @param flat          Make the earth seem flat when true.
     */
    inline void flat_earth( bool flat ) {
        _flat_earth = flat ;
    }

    /**
     * Compute the speed of sound and it's first derivatives at
     * a series of locations.
     *
     * @param location      Location at which to compute attenuation.
     * @param speed         Speed of sound (m/s) at each location (output).
     * @param gradient      Sound speed gradient at each location (output).
     */
    virtual void sound_speed( const wposition& location,
        matrix<double>* speed, wvector* gradient=NULL ) = 0 ;

   /**
    * Define a new in-water attenuation model.
    *
    * @param attenuation    In-water attenuation model.
    */
   void attenuation( attenuation_model* attenuation ) {
       if ( _attenuation ) delete _attenuation ;
       _attenuation = attenuation ;
   }

   /**
    * Computes the broadband absorption loss of sea water.
    *
    * @param location      Location at which to compute attenuation.
    * @param frequencies   Frequencies over which to compute loss. (Hz)
    * @param distance      Distance travelled through the water (meters).
    * @param attenuation   Absorption loss of sea water in dB (output).
    */
   virtual void attenuation(
       const wposition& location,
       const seq_vector& frequencies,
       const matrix<double>& distance,
       matrix< vector<double> >* attenuation)
   {
       _attenuation->attenuation(
           location, frequencies, distance, attenuation ) ;
   }


  protected:

    /**
     * When the flat earth option is enabled, this routine 
     * applies an anti-correction term to the profile.
     * Used to compare propagation effect to models that have 
     * been computed in rectangular coordinates. The corrected sound
     * speed and gradient are given by:
     * <pre>
     *      c' = c r / R
     *
     *      dc/dr' = dc/dr r / R + c / R
     *
     * where:
     *
     *      c, dc/dr    = uncorrected sound speed and gradient
     *      c', dc/dr'  = anti-corrected sound speed and gradient
     *      r           = radial component of wavefront position
     *      R           = earth's radius of curvature
     * </pre>
     * An additional horizontal correction must be made to properly compare 
     * the results of this model to flat earth models.  To convert lat/long
     * changes (dA) into horizontal range (dx), you can not use the 
     * average earth radius (R) such that dx = R dA. Instead you must use
     * the radial position (r) at each point in the wavefront such that
     * dx = r dA.  This routine does not provide horizontal correction.
     *
     * @xref S.M. Reilly, M.S. Goodrich, "Geodetic Acoustic Rays in the 
     * Time Domain, Comprehensive Test Results", Alion Science and 
     * Technology, Norfolk, VA, September, 2006.
     *
     * @param location      Location at which to compute attenuation.
     * @param speed         Speed of sound (m/s) at each location (in/out).
     * @param gradient      Sound speed gradient at each location (in/out).
     */
    virtual void adjust_speed( const wposition& location,
        matrix<double>* speed, wvector* gradient=NULL ) ;

    /** Anti-correction term to make the earth seem flat. */
    bool _flat_earth ;

  private:

    /** Reference to the in-water attenuation model. */
    attenuation_model* _attenuation ;

};

/// @}
}  // end of namespace ocean
}  // end of namespace usml
