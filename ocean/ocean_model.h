/** 
 * @file ocean_model.h
 * Combines the effects of surface, bottom, and profile into a single model.
 */
#ifndef USML_OCEAN_OCEAN_MODEL_H
#define USML_OCEAN_OCEAN_MODEL_H

#include <usml/ocean/boundary_model.h>
#include <usml/ocean/profile_model.h>

namespace usml {
namespace ocean {

/// @ingroup ocean_model
/// @{

/**
 * Combines the effects of surface, bottom, and profile into a single model.
 */
class ocean_model {

    //**************************************************
    // surface model
    
  private:
  
    /** Model of the ocean surface. */
    boundary_model* _surface ;

  public:
        
    /** Retrieve current model for the ocean surface. */
    inline boundary_model& surface() {
        return *_surface ;
    }
        
    //**************************************************
    // bottom model
    
  private:
  
    /** Model of the ocean bottom. */
    boundary_model* _bottom ;
    
  public:
        
    /** Retrieve current model for the ocean bottom. */
    inline boundary_model& bottom() {
        return *_bottom ;
    }
        
    //**************************************************
    // profile model    
    
  private:
  
    /** Model of the sound speed profile and attenuation. */
    profile_model* _profile ;
    
  public:
        
    /** Retrieve current model for the ocean profile. */
    inline profile_model& profile() {
        return *_profile ;
    }

    //**************************************************
    // initialization
    
    /**
     * Associate ocean parts with this model.
     * The ocean model takes over ownership of these models and
     * destroys them in its destructor.
     */
    ocean_model( 
        boundary_model* surface, boundary_model* bottom, profile_model* profile)
        : _surface(surface), _bottom(bottom), _profile(profile)
        {}

    /**
     * Destroys ocean model components.
     */
    virtual ~ocean_model() {
        delete _surface ;
        delete _bottom ;
        delete _profile ;
    }
                                                        
};

/// @}
}  // end of namespace ocean
}  // end of namespace usml

#endif
