/**
 * @file reverberation_model.h
 * Listens for interface collision callbacks from a wavefront.
 */
#ifndef USML_WAVEQ3D_REVERBERATION_MODEL_H
#define USML_WAVEQ3D_REVERBERATION_MODEL_H

#include <usml/ocean/ocean.h>

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
         * @param time          Current time of the wavefront (sec)
         * @param dt            Offset in time to collision with the boundary
         * @param grazing       The grazing angle at point of impact (rads)
         * @param speed         Speed of sound at the point of collision.
         * @param frequencies   Frequencies over which to compute reverb. (Hz)
         * @param position      Location at which the collision occurs
         * @param ndirection    Normalized direction at the point of collision.
         * @param ID            (Used to identify source/receiver/volume layer)
         */
        virtual void notifyUpperCollision( unsigned de, unsigned az, double time,
               double dt, double grazing, double speed, const seq_vector& frequencies,
               const wposition1& position, const wvector1& ndirection,
               const vector<double>& boundary_loss, unsigned ID ) = 0 ;

        /**
         * React to the collision of a single ray with a reverberation
         * when colliding from above the boundary.
         *
         * @param de            D/E angle index number.
         * @param az            AZ angle index number.
         * @param time          Current time of the wavefront (sec)
         * @param dt            Offset in time to collision with the boundary
         * @param grazing       The grazing angle at point of impact (rads)
         * @param speed         Speed of sound at the point of collision.
         * @param frequencies   Frequencies over which to compute reverb. (Hz)
         * @param position      Location at which the collision occurs
         * @param ndirection    Normalized direction at the point of collision.
         * @param ID            (Used to identify source/receiver/volume layer)
         */
        virtual void notifyLowerCollision( unsigned de, unsigned az, double time,
               double dt, double grazing, double speed, const seq_vector& frequencies,
               const wposition1& position, const wvector1& ndirection,
               const vector<double>& boundary_loss, unsigned ID ) = 0 ;

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

#endif
