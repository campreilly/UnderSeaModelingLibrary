/**
 * @file eigenverb_bistatic.h
 */

#ifndef USML_WAVEQ3D_EIGENVERB_BISTATIC_H
#define USML_WAVEQ3D_EIGENVERB_BISTATIC_H

#include <usml/waveq3d/eigenverb.h>
#include <usml/waveq3d/reverberation_model.h>
#include <usml/waveq3d/wave_queue_reverb.h>
#include <usml/waveq3d/spreading_hybrid_gaussian.h>
#include <usml/ublas/ublas.h>
#include <usml/types/types.h>
#include <vector>

namespace usml {
namespace waveq3d {

using namespace usml::ocean ;

using boost::numeric::ublas::vector;

/**
 * A reverberation model listens for interface collision callbacks from
 * a wavefront. Used in bistatic scenarios, i.e. source and receiver are not
 * co-located.
 *
 * @todo The reverberation_model class is currently just a stub for future behaviors.
 */
class USML_DECLSPEC eigenverb_bistatic : public reverberation_model {

    public:

        eigenverb_bistatic( wave_queue_reverb& wave ) ;

        virtual ~eigenverb_bistatic() {}

        /**
         * React to the collision of a single ray with a reverberation
         * when colliding from below the boundary.
         *
         * @param de            D/E angle index
         * @param az            AZ angle index
         * @param time          Current time of the wavefront (sec)
         * @param dt            Offset in time to collision with the boundary
         * @param grazing       The grazing angle at point of impact (rads)
         * @param speed         Speed of sound at the point of collision.
         * @param frequencies   Frequencies over which to compute reverb. (Hz)
         * @param position      Location at which the collision occurs
         * @param ndirection    Normalized direction at the point of collision.
         * @param boundary_loss Cumulative attenuation/boundary loss
         * @param ID            (Used to identify source/receiver/volume layer)
         */
        virtual void notifyUpperCollision( unsigned de, unsigned az, double time,
               double dt, double grazing, double speed, const seq_vector& frequencies,
               const wposition1& position, const wvector1& ndirection,
               const vector<double>& boundary_loss, unsigned ID ) ;

        /**
         * React to the collision of a single ray with a reverberation
         * when colliding from above the boundary.
         *
         * @param de            D/E angle index
         * @param az            AZ angle index
         * @param time          Current time of the wavefront (sec)
         * @param dt            Offset in time to collision with the boundary
         * @param grazing       The grazing angle at point of impact (rads)
         * @param speed         Speed of sound at the point of collision.
         * @param frequencies   Frequencies over which to compute reverb. (Hz)
         * @param position      Location at which the collision occurs
         * @param ndirection    Normalized direction at the point of collision.
         * @param boundary_loss Cumulative attenuation/boundary loss
         * @param ID            (Used to identify source/receiver/volume layer)
         */
        virtual void notifyLowerCollision( unsigned de, unsigned az, double time,
               double dt, double grazing, double speed, const seq_vector& frequencies,
               const wposition1& position, const wvector1& ndirection,
               const vector<double>& boundary_loss, unsigned ID ) ;

        /**
         * Computes the reverberation curve from the data cataloged from the
         * wavefront(s).
         */
        virtual void compute_reverberation() ;

    private:

        /**
         * Defines the type of spreading model that is used to compute
         * one-way TLs and sigma of each dimension.
         */
        spreading_model* _spreading_model ;

        /**
         * Vector of eigenverbs that originate from the
         * source and impact the surface.
         */
        std::vector< eigenverb > _source_surface ;

        /**
         * Vector of eigenverbs that originate from the
         * receiver and impact the surface.
         */
        std::vector< eigenverb > _receiver_surface ;

        /**
         * Vector of eigenverbs that originate from the
         * source and impact the bottom.
         */
        std::vector< eigenverb > _source_bottom ;

        /**
         * Vector of eigenverbs that originate from the
         * receiver and impact the bottom.
         */
        std::vector< eigenverb > _receiver_bottom ;

};

/// @}
}  // end of namespace waveq3d
}  // end of namespace usml

#endif

