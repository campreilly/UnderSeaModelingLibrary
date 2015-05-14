/**
 * @file ocean_model.h
 * Combines the effects of surface, bottom, volume and profile into a single model.
 */
#pragma once

#include <usml/ocean/boundary_model.h>
#include <usml/ocean/profile_model.h>
#include <usml/ocean/volume_model.h>
#include <vector>
#include <iterator>

namespace usml {
namespace ocean {

/// @ingroup ocean_model
/// @{

/**
 * Combines the effects of surface, bottom, volume, and profile into a single model.
 */
class USML_DECLSPEC ocean_model {

  public:

    /** Retrieve current model for the ocean surface. */
    inline boundary_model& surface() {
        return *_surface ;
    }

    /** Retrieve current model for the ocean bottom. */
    inline boundary_model& bottom() {
        return *_bottom ;
    }

    /** Retrieve one layer of the ocean volume. */
    inline volume_model& volume( std::vector<volume_model*>::size_type n ) {
        return *(_volume.at(n)) ;
    }

    /** Retrieve number of ocean volume layers. */
    inline size_t num_volume() {
        return _volume.size() ;
    }

    /** Adds a layer to list of ocean volumes. */
    inline void add_volume( volume_model* layer ) {
        _volume.push_back( layer ) ;
    }

    /** Retrieve current model for the ocean profile. */
    inline profile_model& profile() {
        return *_profile ;
    }

    /**
      * Associate ocean parts with this model.
     * The ocean model takes over ownership of these models and
     * destroys them in its destructor.
     */
    ocean_model( boundary_model* surface,
                 boundary_model* bottom,
                 profile_model* profile,
                 std::vector<volume_model*>* volume = NULL ) :
    _surface(surface), _bottom(bottom), _profile(profile)
    {
        if ( volume ) {
            _volume = *volume ;
        }
    }

    /**
     * Destroys ocean model components.
     */
    virtual ~ocean_model() {
        delete _surface ;
        delete _bottom ;
        for ( std::vector<volume_model*>::iterator iter =_volume.begin();
              iter != _volume.end(); ++iter)
        {
            delete *iter ;
        }
        delete _profile ;
    }

  private:

    /** Model of the ocean surface. */
    boundary_model* _surface ;

    /** Model of the ocean bottom. */
    boundary_model* _bottom ;

    /** Models of ocean volume scattering strength layers. */
    std::vector<volume_model*> _volume ;

    /** Model of the sound speed profile and attenuation. */
    profile_model* _profile ;
};

/// @}
}  // end of namespace ocean
}  // end of namespace usml
