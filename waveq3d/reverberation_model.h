/**
 * @file reverberation_model.h
 * Listens for interface collision callbacks from a wavefront.
 */
#ifndef USML_WAVEQ3D_REVERBERATION_MODEL_H
#define USML_WAVEQ3D_REVERBERATION_MODEL_H

#include <usml/ublas/ublas.h>
#include <usml/types/types.h>

namespace usml {
namespace waveq3d {

using namespace usml::ocean ;

using boost::numeric::ublas::vector;

/**
 * A reverberation model listens for interface collision callbacks from
 * a wavefront.
 *
 * @todo The reverberation_model class is currently just a stub for future behaviors.
 */
class USML_DECLSPEC reverberation_model {

    public:

        reverberation_model() {}

        virtual ~reverberation_model() {}

        /**
         * React to the collision of a single ray with a reverberation
         * when colliding from below the boundary.
         *
         * @param ID            (Used to identify source/receiver/volume layer)
         * @param de            D/E angle index number.
         * @param az            AZ angle index number.
         * @param time          Offset time to impact the boundary (sec)
         * @param grazing       The grazing angle at point of impact (rads)
         * @param speed         Speed of sound at the point of collision.
         * @param frequencies   Frequencies over which to compute reverb. (Hz)
         * @param position      Location at which the collision occurs
         * @param ndirection    Normalized direction at the point of collision.
         */
        virtual bool notifyUpperCollision( unsigned de, unsigned az, double time,
               double grazing, double speed, const seq_vector& frequencies,
               const wposition1& position, const wvector1& ndirection, int ID ) = 0 ;

        /**
         * React to the collision of a single ray with a reverberation
         * when colliding from above the boundary.
         *
         * @param ID            (Used to identify source/receiver/volume layer)
         * @param de            D/E angle index number.
         * @param az            AZ angle index number.
         * @param time          Offset time to impact the boundary (sec)
         * @param grazing       The grazing angle at point of impact (rads)
         * @param speed         Speed of sound at the point of collision.
         * @param frequencies   Frequencies over which to compute reverb. (Hz)
         * @param position      Location at which the collision occurs
         * @param ndirection    Normalized direction at the point of collision.
         */
        virtual bool notifyLowerCollision( unsigned de, unsigned az, double time,
               double grazing, double speed, const seq_vector& frequencies,
               const wposition1& position, const wvector1& ndirection, int ID ) = 0 ;

        virtual void compute_reverberation() = 0 ;

};

/// @}
}  // end of namespace waveq3d
}  // end of namespace usml

#endif
