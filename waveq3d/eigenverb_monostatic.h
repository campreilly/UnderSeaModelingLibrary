/**
 * @file eigenverb_monostatic.h
 */

#ifndef USML_WAVEQ3D_EIGENVERB_MONOSTATIC_H
#define USML_WAVEQ3D_EIGENVERB_MONOSTATIC_H

#include <usml/waveq3d/eigenverb.h>
#include <usml/waveq3d/reverberation_model.h>
#include <usml/waveq3d/wave_queue_reverb.h>
#include <usml/waveq3d/spreading_model.h>
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
         * @param ID            (Used to identify source/receiver/volume layer)
         */
        virtual void notifyUpperCollision( unsigned de, unsigned az, double time,
               double dt, double grazing, double speed, const seq_vector& frequencies,
               const wposition1& position, const wvector1& ndirection, unsigned ID ) ;

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
               const wposition1& position, const wvector1& ndirection, unsigned ID ) ;

        /**
         * Computes the reverberation curve from the data cataloged from the
         * wavefront(s).
         */
        virtual void compute_reverberation() ;

        /**
         * Gains access to the reverberation data. The user should first execute
         * compute_reverberation() prior to requesting access to the entire
         * reverberation curve.
         * @return      pointer to _reverberation_curve
         */
         virtual const vector<double> getReverberation_curve() {
            return _reverberation_curve ;
         }

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
         * Calculate the contributions due to collisions from below
         * a volume layer.
         */
        void compute_upper_volume() ;

        /**
         * Calculate the contributions due to collisions from above
         * a volume layer.
         */
        void compute_lower_volume() ;

        inline matrix<double> mu( const eigenverb& e ) ;

        inline matrix<double> sigma( const eigenverb& e ) ;

        inline double area( const matrix<double>& mu1, const matrix<double>& sigma1,
                            const matrix<double>& mu2, const matrix<double>& sigma2 ) ;

        inline double energy( const eigenverb& in, const eigenverb& out,
                              const double area, scattering_model* s ) ;

        inline double time_spread( const eigenverb& out, const matrix<double>& s1,
                                   const matrix<double>& s2, const double travel_time ) ;

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
         * The reverberation energy distribution curve. The values in this
         * array are in linear units.
         */
        vector<double> _reverberation_curve ;

        /**
         * Origin of the wavefront
         * In monostatic this will always be 10.
         */
        unsigned _origin ;

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
        scattering_model* _volume_scatter ;

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
