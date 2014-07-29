/**
 * @file eigenverb_bistatic.h
 */

#ifndef USML_WAVEQ3D_EIGENVERB_BISTATIC_H
#define USML_WAVEQ3D_EIGENVERB_BISTATIC_H

#include <usml/waveq3d/eigenverb_model.h>

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
class USML_DECLSPEC eigenverb_bistatic : public eigenverb_model {

    public:

        eigenverb_bistatic( ocean_model& ocean,
                            wave_queue_reverb& wave_source,
                            wave_queue_reverb& wave_receiver,
                            double pulse, unsigned num_bins,
                            double max_time ) ;

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

    private:

        /**
         * Computes the energy contributions to the reverberation
         * energy curve from the bottom interactions.
         */
        virtual void compute_bottom_energy() ;

        /**
         * Computes the energy contributions to the reverberation
         * energy curve from the surface interactions.
         */
        virtual void compute_surface_energy() ;

        /**
         * Calculate the contributions due to collisions from below
         * a volume layer.
         */
        virtual void compute_upper_volume_energy() ;

        /**
         * Calculate the contributions due to collisions from above
         * a volume layer.
         */
        virtual void compute_lower_volume_energy() ;

        /**
         * Takes a set of eigenrays, boundary model, and convolves the set of
         * eigenverbs with itself and makes contributions to the reverebation
         * level curve.
         */
        void convolve_eigenverbs( std::vector<eigenverb>& set1,
                std::vector<eigenverb>& set2, boundary_model* boundary ) ;

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

        /**
         * Vector of eigenverbs that originate from the
         * source and impact the volume layer from below.
         */
        std::vector< std::vector< eigenverb > > _source_upper ;

        /**
         * Vector of eigenverbs that originate from the
         * receiver and impact the volume layer from below.
         */
        std::vector< std::vector< eigenverb > > _receiver_upper ;

        /**
         * Vector of eigenverbs that originate from the
         * source and impact the volume layer from above.
         */
        std::vector< std::vector< eigenverb > > _source_lower ;

        /**
         * Vector of eigenverbs that originate from the
         * receiver and impact the volume layer from above.
         */
        std::vector< std::vector< eigenverb > > _receiver_lower ;


};

/// @}
}  // end of namespace waveq3d
}  // end of namespace usml

#endif

