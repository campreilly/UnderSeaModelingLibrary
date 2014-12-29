/**
 * @file reverberation_model.h
 * Listens for interface collision callbacks from a wavefront.
 */
#pragma once

#include <usml/ocean/ocean.h>
#include <usml/waveq3d/wave_queue.h>

namespace usml {
namespace waveq3d {

using namespace usml::ocean ;

using namespace boost::numeric::ublas ;

/**
 * A reverberation model listens for interface collision callbacks from
 * a wavefront.
 *
 * @todo The reverberation_model class is currently just a stub for future behaviors.
 */
class USML_DECLSPEC reverberation_model {

    public:

        /**
         * Virtual destructor
         */
        virtual ~reverberation_model() {}

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
        virtual void notifyUpperCollision( size_t de, size_t az,
               double dt, double grazing, double speed,
               const wposition1& position, const wvector1& ndirection,
               const wave_queue& wave, size_t ID ) = 0 ;

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
        virtual void notifyLowerCollision( size_t de, size_t az,
               double dt, double grazing, double speed,
               const wposition1& position, const wvector1& ndirection,
               const wave_queue& wave, size_t ID ) = 0 ;

        /**
         * Computes the reverberation curve from the data cataloged from the
         * wavefront(s).
         */
        virtual void compute_reverberation() = 0 ;

        /**
         * Returns the reverberation data
         */
        virtual const vector<double> getReverberation_curve() = 0 ;

};

/// @}
}  // end of namespace waveq3d
}  // end of namespace usml
