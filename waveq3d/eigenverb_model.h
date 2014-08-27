/**
 * @file eigenverb_model.h
 */

#ifndef USML_WAVEQ3D_EIGENVERB_MODEL_H
#define USML_WAVEQ3D_EIGENVERB_MODEL_H

//#define EIGENVERB_COLLISION_DEBUG
//#define EIGENVERB_MODEL_DEBUG

#include <usml/ocean/ocean.h>
#include <usml/ublas/math_traits.h>
#include <usml/waveq3d/eigenverb.h>
#include <usml/waveq3d/reverberation_model.h>
#include <usml/waveq3d/wave_queue_reverb.h>
#include <usml/waveq3d/spreading_model.h>
#include <usml/ublas/ublas.h>
#include <boost/numeric/ublas/lu.hpp>
#include <usml/types/types.h>
#include <vector>

namespace usml {
namespace waveq3d {

using namespace usml::ocean ;

using namespace boost::numeric::ublas ;

class USML_DECLSPEC eigenverb_model : public reverberation_model {

    public:

        /**
         * Virtual destructor
         */
        virtual ~eigenverb_model() {}

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
               const vector<double>& boundary_loss, unsigned ID ) {}

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
               const vector<double>& boundary_loss, unsigned ID ) {}

        /**
         * Computes the reverberation curve from the data cataloged from the
         * wavefront(s).
         */
        virtual void compute_reverberation() {
            compute_bottom_energy() ;
            compute_surface_energy() ;
            compute_upper_volume_energy() ;
            compute_lower_volume_energy() ;
        }

        /**
         * Gains access to the reverberation data. The user should first execute
         * compute_reverberation() prior to requesting access to the entire
         * reverberation curve.
         * @return      pointer to _reverberation_curve
         */
         virtual const vector<double> getReverberation_curve() {
            return _reverberation_curve ;
         }

    protected:

        /**
         * Constructs and eigenverb from the data provided in a notify collision.
         *
         * @param  de            D/E angle index number.
         * @param  az            AZ angle index number.
         * @param  time          Current time of the wavefront (sec)
         * @param  dt            Offset in time to collision with the boundary
         * @param  grazing       The grazing angle at point of impact (rads)
         * @param  speed         Speed of sound at the point of collision.
         * @param  frequencies   Frequencies over which to compute reverb. (Hz)
         * @param  position      Location at which the collision occurs
         * @param  ndirection    Normalized direction at the point of collision.
         * @param  boundary_loss Cumulative attenuation/boundary loss
         * @return verb         newly constructed eigenverb
         */
        void create_eigenverb( unsigned de, unsigned az, double time,
               double dt, double grazing, double speed, const seq_vector& frequencies,
               const wposition1& position, const wvector1& ndirection,
               const vector<double>& boundary_loss, eigenverb& verb ) const ;

        /**
         * Computes the contribution value of two eigenverbs to the total
         * reverberation level.
         */
        void compute_contribution( const eigenverb& u,
                                   const eigenverb& v,
                                   boundary_model* boundary ) ;

        /**
         * Creates a 2x1 matrix from the eigenverb data. These are used to
         * find the relative angle and separation distance from two eigenverbs'
         * gaussian profile.
         *
         * @param e         The eigenverb used to construct the mu portion of the
         *                  gaussian profile from
         * @return          matrix with dimensional gaussian mean values
         */
        inline matrix<double> mu( const eigenverb& e ) ;

        /**
         * Creates a 2x2 matrix from the eigenverb data. These are used to
         * find the intersection between two gaussian profile.
         *
         * @param e         The eigenverb used to construct the mu portion of the
         *                  gaussian profile from
         * @param theta     Angle relative to some frame of reference
         * @return          matrix with dimensional gaussian standard deviations
         */
        inline matrix<double> sigma( const eigenverb& e, double theta=0.0 ) ;

        /**
         * Computes the intersection between two gaussian profiles.
         *
         * @param mu1       gaussian means for the first gaussian
         * @param sigma1    gaussian standard deviations for the first gaussian
         * @param mu2       gaussian means for the second gaussian
         * @param sigma2    gaussian standard deviations for the second gaussian
         * @return          intersection of the two gaussian profiles
         */
        inline double area( const matrix<double>& mu1, const matrix<double>& sigma1,
                            const matrix<double>& mu2, const matrix<double>& sigma2 ) ;

        /**
         * Computes the total energy reflecting from this ensonified patch on the boundary.
         *
         * @param in        eigenverb that is coming to this patch on the boundary
         * @param out       eigenverb that is leaving from this patch on the boundary
         * @param dA        The area of this ensonified patch
         * @param b         pointer to the boundary_model for this boundary
         * @return          total energy reflected from this ensonified patch
         */
        inline double energy( const eigenverb& in, const eigenverb& out,
                              const double dA, boundary_model* b ) ;

        /**
         * Spreads out the energy from the ensonified patch out in time.
         *
         * @param out           eigenverb that is leaving from this patch on the boundary
         * @param s1            gaussian standard deviation values for first eigenverb
         * @param s2            gaussian standard deviation values for second eigenverb
         * @param travel_time   Sum of the travel times from the two eigenverbs
         * @return              total energy spread out in time
         */
        inline vector<double> time_spread( const eigenverb& out, const matrix<double>& s1,
                                   const matrix<double>& s2, const double travel_time ) ;

        /**
         * Let \mathcal{N}_x( \mu, \Sigma ) denote a gaussian density in \mathrm{x},
         * then the product of two gaussians is given by:
         *
         *\f[
         * \mathcal{N}_x(\mu_1,\Sigma_1 ) \cdot \mathcal{N}_x(\mu_2,\Sigma_2 )
         * \eq \mathcal{c}_c \mathcal{N}_x(\mu_c,\Sigma_c )
         *\f]
         * where:
         *
         *\f[
         * \mathcal{N}_x(\mu,\Sigma)  \eq \frac{1}{\sqrt{mathrm{det}(2\pi(\Sigma_1+\Sigma_2))}}
         * \mathrm{exp}[\-\frac{1}{2}(\mathrm{x}-\mu)^{\mathrm{T}}\Sigma(\mathrm{x}-\mu)]
         *\f]
         *\f[
         * \mathcal{c}_c \eq \mathcal{N}_{ \mu_1 }( \mu_2,( \Sigma_1 + \Sigma_2 ) )
         *\f]
         *\f[
         * \mu_c \eq ( \Sigma_1^{-1} + \Sigma_2^{-1} )^{-1}
         * ( \Sigma_1^{-1} \mu_1 + \Sigma_2^{-1} \mu_2 )
         *\f]
         *\f[
         * \Sigma_c = ( \Sigma_1^{-1} + \Sigma_2^{-1} )^{-1}
         *\f]
         *
         * This produces the area of the intersection of the gaussians, used during the production
         * of the reverberation curve.
         * @param mu            mu_c described above
         * @param sigma         Sigma_c described above
         * @return              c_c described above
         */
        inline double gaussian( const matrix<double>& mu, const matrix<double>& sigma ) ;

        /**
         * Calculates the matrix determinent of a uBlas matrix
         */
        inline double determinent( const matrix<double>& m ) ;

        /**
         * Computes the inverse of a matrix.
         */
        inline bool inverse( const matrix<double>& m, matrix<double>& i ) ;

        /**
         * Computes the energy contributions to the reverberation
         * energy curve from the bottom interactions.
         */
        virtual void compute_bottom_energy() = 0 ;

        /**
         * Computes the energy contributions to the reverberation
         * energy curve from the surface interactions.
         */
        virtual void compute_surface_energy() = 0 ;

        /**
         * Calculate the contributions due to collisions from below
         * a volume layer.
         */
        virtual void compute_upper_volume_energy() = 0 ;

        /**
         * Calculate the contributions due to collisions from above
         * a volume layer.
         */
        virtual void compute_lower_volume_energy() = 0 ;

        /**
         * Pulse length of the signal (sec)
         */
        double _pulse ;

        /**
         * Number of time bins to store the energy distribution
         */
        unsigned _max_index ;

        /**
         * Max time for the reverberation curve
         */
        double _max_time ;

        /**
         * Number of layers within the volume
         */
        unsigned _n ;

        /**
         * Origin ID of the source wavefront
         */
        unsigned _source_origin ;

        /**
         * Origin ID of the reciever wavefront
         */
        unsigned _receiver_origin ;

        /**
         * Defines the type of spreading model that is used to compute
         * one-way TLs and sigma of each dimension.
         */
        spreading_model* _spreading_model ;

        /**
         * Defines the type of boundary model for the bottom.
         */
        boundary_model* _bottom_boundary ;

        /**
         * Defines the type of boundary model for the surface.
         */
        boundary_model* _surface_boundary ;

        /**
         * Defines the type(s) of boundary model(s) for the volume.
         */
        volume_layer* _volume_boundary ;

        /**
         * The reverberation energy distribution curve. The values in this
         * array are in linear units.
         */
        vector<double> _reverberation_curve ;

        /**
         * Time resolution of the reverberation curve. This is used to spread
         * the energy from a contribution out in time along the curve.
         */
        vector<double> _two_way_time ;

};

}   // end of namespace waveq3d
}   // end of namespace usml

#endif
