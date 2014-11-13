/**
 * @file ocean_model.h
 * Combines the effects of surface, bottom, and profile into a single model.
 */
#ifndef USML_OCEAN_OCEAN_MODEL_H
#define USML_OCEAN_OCEAN_MODEL_H

#include <usml/ocean/boundary_model.h>
#include <usml/ocean/profile_model.h>
#include <usml/ocean/volume_layer.h>

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

        /** Retrieve current model for the ocean profile. */
        inline profile_model& profile() {
            return *_profile ;
        }

        /** Retrieve current model for the ocean volume. */
        inline volume_layer* volume() {
            return _volume ;
        }

        /**
        * Associate ocean parts with this model.
        * The ocean model takes over ownership of these models and
        * destroys them in its destructor.
        */
        ocean_model( boundary_model* surface,
                     boundary_model* bottom,
                     profile_model* profile,
                     volume_layer* volume=NULL ) :
        _surface(surface), _bottom(bottom), _profile(profile), _volume(volume)
        {}

        /**
        * Destroys ocean model components.
        */
        virtual ~ocean_model() {
            delete _surface ;
            delete _bottom ;
            delete _profile ;
            delete _volume ;
        }

    private:

        /** Model of the ocean surface. */
        boundary_model* _surface ;

        /** Model of the ocean bottom. */
        boundary_model* _bottom ;

        /** Model of the sound speed profile and attenuation. */
        profile_model* _profile ;

        /** Model of the ocean volume. Used exclusively in reverberation */
        volume_layer* _volume ;

};

/// @}
}  // end of namespace ocean
}  // end of namespace usml

#endif
