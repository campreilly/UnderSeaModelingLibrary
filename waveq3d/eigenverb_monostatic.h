/**
 * @file eigenverb_monostatic.h
 */

#ifndef USML_WAVEQ3D_EIGENVERB_MONOSTATIC_H
#define USML_WAVEQ3D_EIGENVERB_MONOSTATIC_H

#include <usml/waveq3d/waveq3d.h>
#include <usml/waveq3d/eigenverb.h>
#include <usml/waveq3d/reverberation_model.h>
#include <usml/ublas/ublas.h>
#include <usml/types/types.h>
#include <vector>
#include <cstring>

namespace usml {
namespace waveq3d {

using namespace usml::ocean ;

using boost::numeric::ublas::vector;

/**
 * A reverberation model listens for interface collision callbacks from
 * a wavefront. Used in monostatic scenarios, i.e. source and receiver are
 * co-located.
 *
 * @todo The reverberation_model class is currently just a stub for future behaviors.
 */
class USML_DECLSPEC eigenverb_monostatic : public reverberation_model {

    public:

        eigenverb_monostatic( ocean_model& ocean,
                              wave_queue& wave,
                              double pulse,
                              unsigned num_bins,
                              double max_time ) ;

        virtual ~eigenverb_monostatic() ;

        /**
         * React to the collision of a single ray with a reverberation
         * when colliding from below the boundary.
         *
         * @param ID            (Used to identify source/receiver/volume layer)
         * @param de            D/E angle index number.
         * @param az            AZ angle index number.
         * @param time          Current time of the wavefront (sec)
         * @param dt            Offset in time to collision with the boundary
         * @param grazing       The grazing angle at point of impact (rads)
         * @param speed         Speed of sound at the point of collision.
         * @param frequencies   Frequencies over which to compute reverb. (Hz)
         * @param position      Location at which the collision occurs
         * @param ndirection    Normalized direction at the point of collision.
         */
        virtual void notifyUpperCollision( unsigned de, unsigned az, double time,
               double dt, double grazing, double speed, const seq_vector& frequencies,
               const wposition1& position, const wvector1& ndirection, int ID ) ;

        /**
         * React to the collision of a single ray with a reverberation
         * when colliding from above the boundary.
         *
         * @param ID            (Used to identify source/receiver/volume layer)
         * @param de            D/E angle index number.
         * @param az            AZ angle index number.
         * @param time          Current time of the wavefront (sec)
         * @param dt            Offset in time to collision with the boundary
         * @param grazing       The grazing angle at point of impact (rads)
         * @param speed         Speed of sound at the point of collision.
         * @param frequencies   Frequencies over which to compute reverb. (Hz)
         * @param position      Location at which the collision occurs
         * @param ndirection    Normalized direction at the point of collision.
         */
        virtual void notifyLowerCollision( unsigned de, unsigned az, double time,
               double dt, double grazing, double speed, const seq_vector& frequencies,
               const wposition1& position, const wvector1& ndirection, int ID ) ;

        /**
         * Computes the reverberation curve from the data cataloged from the
         * wavefront(s).
         */
        virtual void compute_reverberation() ;

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
         * Computes the energy contributions to the reverberation
         * energy curve from the volume interactions.
         */
        void compute_volume_energy() ;

        /**
         * Pulse length of the signal (sec)
         */
        double _pulse ;

        /**
         * Number of time bins to store the energy distribution
         */
        unsigned _num_bins ;

        /**
         * Max time for the reverberation curve
         */
        double _max_time ;

        /**
         * The reverberation energy distribution curve.
         */
        double* _energy ;

        /**
         * Defines the type of spreading model that is used to compute
         * one-way TLs and sigma of each dimension.
         */
        spreading_model* _spreading_model ;

        /**
         * Defines the type of scattering model that is used to compute
         * the scattering strength off the boundary.
         */
        scattering_model* _bottom_scatter ;

        /**
         * Defines the type of scattering model that is used to compute
         * the scattering strength off the boundary.
         */
        scattering_model* _surface_scatter ;

        /**
         * Defines the type of scattering model that is used to compute
         * the scattering strength off the boundary.
         */
//        scattering_model* _volume_scatter ;

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
        std::vector< vector< eigenverb > > _upper ;

        /**
         * Vector of eigenverbs that collide with the volume
         * boundarys from above.
         */
        std::vector< vector< eigenverb > > _lower ;

};

/// @}
}  // end of namespace waveq3d
}  // end of namespace usml

#endif
