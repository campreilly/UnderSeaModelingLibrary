/*
 * @file eigenverb_collection
 */

#pragma once

#include <usml/waveq3d/eigenverb.h>

namespace usml {
namespace waveq3d {

class eigenverb_collection {

    public:

        /**
         * Constructor
         */
        eigenverb_collection( const size_t layers = 0 )
            : _upper(layers), _lower(layers)
        {}

        /*
         * Destructor
         */
        ~eigenverb_collection() {}


        /**
         * React to the collision of a single ray with a reverberation
         * when colliding from below the boundary.
         *
         * @param de            D/E angle index number.
         * @param az            AZ angle index number.
         * @param dt            Offset in time to collision with the boundary
         * @param grazing       The grazing angle at point of impact (rads)
         * @param speed         Speed of sound at the point of collision.
         * @param position      Location at which the collision occurs
         * @param ndirection    Normalized direction at the point of collision.
         * @param wave          Wave queue, used to extract various data
         * @param ID            (Used to identify source/receiver/volume layer)
         */
        void notifyUpperCollision( size_t de, size_t az,
            double dt, double grazing, double speed,
            const wposition1& position, const wvector1& ndirection,
            const wave_queue& wave, size_t ID ) ;

        /**
         * React to the collision of a single ray with a reverberation
         * when colliding from above the boundary.
         *
         * @param de            D/E angle index number.
         * @param az            AZ angle index number.
         * @param dt            Offset in time to collision with the boundary
         * @param grazing       The grazing angle at point of impact (rads)
         * @param speed         Speed of sound at the point of collision.
         * @param position      Location at which the collision occurs
         * @param ndirection    Normalized direction at the point of collision.
         * @param wave          Wave queue, used to extract various data
         * @param ID            (Used to identify source/receiver/volume layer)
         */
        void notifyLowerCollision( size_t de, size_t az,
            double dt, double grazing, double speed,
            const wposition1& position, const wvector1& ndirection,
            const wave_queue& wave, size_t ID ) ;

        /*
         * Creates an eigenverb from the provided data
         */
        void create_eigenverb( size_t de, size_t az,
            double dt, double grazing, double speed,
            const wposition1& position, const wvector1& ndirection,
            const wave_queue& wave, eigenverb& verb ) ;

        /*
         * List of all the eigenverbs for bottom boundary collisions
         */
        eigenverb_list _bottom ;

        /*
         * List of all the eigenverbs for surface boundary collisions
         */
        eigenverb_list _surface ;

        /*
         * List of all the eigenverbs for upper volume layer collisions
         */
        std::list< eigenverb_list > _upper ;

        /*
         * List of all the eigenverbs for lower volume layer collisions
         */
        std::list< eigenverb_list > _lower ;
};

}   // end of namespace waveq3d
}   // end of namespace usml
