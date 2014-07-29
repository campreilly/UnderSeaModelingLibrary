/**
 * @file eigenverb_monostatic.h
 */

#ifndef USML_WAVEQ3D_EIGENVERB_MONOSTATIC_H
#define USML_WAVEQ3D_EIGENVERB_MONOSTATIC_H

#include <usml/waveq3d/eigenverb_model.h>

namespace usml {
namespace waveq3d {

using namespace usml::ocean ;

using namespace boost::numeric::ublas ;

/**
 * A reverberation model listens for interface collision callbacks from
 * a wavefront. Used in monostatic scenarios, i.e. source and receiver are
 * co-located.
 *
 * @todo The reverberation_model class is currently just a stub for future behaviors.
 */
class USML_DECLSPEC eigenverb_monostatic : public eigenverb_model {

    public:

        eigenverb_monostatic( ocean_model& ocean,
                              wave_queue_reverb& wave,
                              double pulse,
                              unsigned num_bins,
                              double max_time ) ;

        virtual ~eigenverb_monostatic() {}

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
         * @param de            D/E angle index number.
         * @param az            AZ angle index number.
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
        void compute_bottom_energy() ;

        /**
         * Computes the energy contributions to the reverberation
         * energy curve from the surface interactions.
         */
        void compute_surface_energy() ;

        /**
         * Calculate the contributions due to collisions from below
         * a volume layer.
         */
        void compute_upper_volume_energy() ;

        /**
         * Calculate the contributions due to collisions from above
         * a volume layer.
         */
        void compute_lower_volume_energy() ;

        /**
         * Takes a set of eigenrays, boundary model, and convolves the set of
         * eigenverbs with itself and makes contributions to the reverebation
         * level curve.
         */
        void convolve_eigenverbs( std::vector<eigenverb>& set,
                                  boundary_model* boundary ) ;

        /**
         * Vector of eigenverbs that impacted the surface.
         */
        std::vector< eigenverb > _surface ;

        /**
         * Vector of eigenverbs that impacted the bottom.
         */
        std::vector< eigenverb > _bottom ;

        /**
         * Vector of eigenverbs that collide with the volume
         * boundarys from below.
         */
        std::vector< std::vector< eigenverb > > _upper ;

        /**
         * Vector of eigenverbs that collide with the volume
         * boundarys from above.
         */
        std::vector< std::vector< eigenverb > > _lower ;

};

/// @}
}  // end of namespace waveq3d
}  // end of namespace usml

#endif
