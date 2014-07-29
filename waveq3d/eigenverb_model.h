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

        /****/
        void create_eigenverb( unsigned de, unsigned az, double time,
               double dt, double grazing, double speed, const seq_vector& frequencies,
               const wposition1& position, const wvector1& ndirection,
               const vector<double>& boundary_loss, eigenverb& verb ) const
        {
            verb.de = de ;
            verb.az = az ;
            verb.time = time + dt ;
            verb.grazing = grazing ;
            verb.c = speed ;
            verb.pos = position ;
            verb.ndir = ndirection ;
            verb.frequencies = &frequencies ;

                // Calculate the one way TL and the width of the gaussian
                // at the time of impact with the boundary.
            c_vector<double,3> offset, distance ;
            offset(0) = dt ;                                      // Temporal offset still exists
            offset(1) = offset(2) = 0.0 ;                           // No offset in DE and AZ
            distance(0) = distance(1) = distance(2) = 0.0 ;         // Zero distance

            #ifdef EIGENVERB_COLLISION_DEBUG
                std::cout << "\t---Calling _spreading_model->getIntensity()---" << std::endl ;
                std::cout << "de: " << de << " az: " << az << std::endl ;
                std::cout << "offset: " << offset << " distance: " << distance << std::endl ;
            #endif

            const vector<double> amp = _spreading_model->getIntensity( position, de, az, offset, distance ) ;
            verb.intensity = element_prod( amp, boundary_loss ) ;
            verb.sigma_de = _spreading_model->getWidth_DE( de, az, offset ) ;
            verb.sigma_az = _spreading_model->getWidth_AZ( de, az, offset ) ;

            #ifdef EIGENVERB_COLLISION_DEBUG
                std::cout << "\t---Added eigenverb to collection---" << endl ;
                std::cout << "\tverb de: " << verb.de << " az: " << verb.az
                          << " time: " << verb.time << std::endl ;
                std::cout << "\tgrazing: " << verb.grazing*180.0/M_PI << " speed: " << verb.c << std::endl ;
                std::cout << "\tintensity: " << verb.intensity << " sigma_de: " << verb.sigma_de
                          << " sigma_az: " << verb.sigma_az << std::endl ;
            #endif
        }

        /**
         * Computes the contribution value of two eigenverbs to the total
         * reverberation level.
         */
        void compute_contribution( const eigenverb& u, const eigenverb& v,
                                   boundary_model* boundary )
        {
            double travel_time = u.time + v.time ;
            #ifdef EIGENVERB_MODEL_DEBUG
                std::cout << "Contribution data:" << std::endl ;
                std::cout << "\ttravel_time: " << travel_time << std::endl ;
            #endif
            double theta = 0.0 ;                                                        /// @todo add angle calculation between u relative to v
            matrix<double> mu1 = mu( u ) ;
            matrix<double> sigma1 = sigma( u, theta ) ;
            matrix<double> mu2 = mu( v ) ;
            matrix<double> sigma2 = sigma( v ) ;
                // Compute the intersection of the gaussian profiles
            double dA = area( mu1, sigma1, mu2, sigma2 ) ;

                // Compute the energy reflected of of this patch
            double _energy = energy( u, v, dA, boundary ) ;

                // Only added value if contribution is significant
            if ( _energy > 1e-20 ) {
                    // determine where on the temporally this contributions calls on the curve
                    // and add it to the cumulative curve value.
                unsigned t = floor( _max_index * travel_time / _max_time ) ;
                double two_way = _max_time * t / _max_index ;

                    // Calculate the time spread of the energy
                double _time_spread = time_spread( v, sigma1, sigma2, travel_time, two_way ) ;

                double value = _energy * _time_spread ;

                #ifdef EIGENVERB_MODEL_DEBUG
                    std::cout << "\tcontribution: " << value << " bin: " << t << std::endl ;
                #endif

                _reverberation_curve(t) += value ;
            }
        }

        /**
         * Creates a 2x1 matrix from the eigenverb data. These are used to
         * find the realitive angle and separation distance from two eigenverbs'
         * gaussian profile.
         */
        inline matrix<double> mu( const eigenverb& e ) {
            matrix<double> t (2,1) ;
            t(0,0) = e.pos.longitude() ;
            t(1,0) = e.pos.latitude() ;
            return t ;
        }

        /**
         * Gaussian profile for this eigenverb.
         */
        inline matrix<double> sigma( const eigenverb& e, double theta=0.0 ) {
                // create a non-rotated matrix with covariances
            matrix<double> t (2,2) ;
            t(0,1) = t(1,0) = 0.0 ;
            t(0,0) = e.sigma_az * e.sigma_az ;
            t(1,1) = e.sigma_de * e.sigma_de / (sin(e.grazing) * sin(e.grazing)) ;
            if ( theta > 1e-20 ) {
                    // create a rotation matrix
                matrix<double> r (2,2) ;
                r(0,0) = cos(theta) ;
                r(1,0) = sin(theta) ;
                r(0,1) = -sin(theta) ;
                r(1,1) = cos(theta) ;
                matrix<double> r_trans = trans(r) ;
                    // apply rotation
                t = prod( r, t ) ;
                t = prod( t, r_trans ) ;
            }
            return t ;
        }

        /**
         *
         */
        inline double area( const matrix<double>& mu1, const matrix<double>& sigma1,
                            const matrix<double>& mu2, const matrix<double>& sigma2 )
        {
            matrix<double> mu = mu1 - mu2 ;
            matrix<double> sigma = sigma1 + sigma2 ;
            double d1 = sqrt( sigma1(0,0)*sigma1(1,1) - sigma1(0,1)*sigma1(1,0) ) ;
            double d2 = sqrt( sigma2(0,0)*sigma2(1,1) - sigma2(0,1)*sigma2(1,0) ) ;

                // determinent and inverse of sigma
            double det = sigma(0,0)*sigma(1,1) - sigma(0,1)*sigma(1,0) ;
            matrix<double> s_inv(sigma) ;
            s_inv(0,0) = sigma(1,1) ;
            s_inv(1,1) = sigma(0,0) ;
            s_inv(0,1) = -sigma(0,1) ;
            s_inv(1,0) = -sigma(1,0) ;
            s_inv /= det ;
                // exponent product
            matrix<double> mu_trans = trans(mu) ;
            matrix<double> mu_prod = prod( mu_trans, s_inv ) ;
            matrix<double> kappa = prod( mu_prod, mu ) ;
            det = 1.0 / sqrt(det) ;
            kappa *= -0.5 ;

                // calculate the area
            double a = 0.5 * d1 * d2 * det * exp( kappa(0,0) ) ;
            #ifdef EIGENVERB_MODEL_DEBUG
                std::cout << "\tmu: " << mu << " sigma: " << sigma << std::endl ;
                std::cout << "\tarea: " << a << std::endl ;
            #endif
            return  a ;
        }

        /**
         *
         */
        inline double energy( const eigenverb& in, const eigenverb& out,
                              const double dA, boundary_model* b )
        {
                // calculate the additional loss due to collision with the boundary
                // for only one direction of the transmission
            vector<double> _loss( (*in.frequencies).size() ) ;
            b->reflect_loss( in.pos, *(in.frequencies), in.grazing, &_loss ) ;
            for(unsigned f=0; f<(*in.frequencies).size(); ++f) {
                _loss(f) = pow( 10.0, _loss(f) / -20.0 ) ;
            }
            vector<double> TL_in = element_prod( in.intensity, _loss ) ;
            vector<double> two_way_TL = element_prod( TL_in, out.intensity ) ;
                // calculate the scattering loss from the interface
            vector<double> s_sr( (*out.frequencies).size() ) ;
            vector<double> phase( (*out.frequencies).size() ) ;
            b->getScattering_Model()->scattering_strength( out.pos, (*out.frequencies),
                    in.grazing, out.grazing, in.az, out.az, &s_sr, &phase ) ;
                // caluclate the total energy reflected from this patch
            double _e = _pulse * two_way_TL(0) * s_sr(0) * dA ;
            #ifdef EIGENVERB_MODEL_DEBUG
                std::cout << "\tboundary loss: " << _loss
                          << " scattering strength: " << s_sr
                          << std::endl ;
                std::cout << "\tTL to patch: " << in.intensity
                          << " TL from patch: " << out.intensity
                          << std::endl ;
                std::cout << "\tenergy: " << _e << std::endl ;
            #endif
            return _e ;
        }

        /**
         *
         */
        inline double time_spread( const eigenverb& out, const matrix<double>& s1,
                                   const matrix<double>& s2, const double travel_time,
                                   const double two_way_time )
        {
            double time = travel_time ;
            double l_r = 1.0 / s1(1,1) ;
            double l_s = 1.0 / s2(1,1) ;
            double sigma_p = sqrt( 1.0 / (l_r + l_s) ) ;
            double Tarea = sigma_p * sin(out.grazing) / out.c ;
            double T_sr = sqrt( _pulse*_pulse + Tarea*Tarea ) / 2.0 ;               /// @caution should this be sine or cosine of grazing angle????
            time += T_sr ;
            double time_exp = (two_way_time-time) / T_sr ;
            #ifdef EIGENVERB_MODEL_DEBUG
                std::cout << "\tT_sr: " << T_sr << std::endl ;
            #endif
            return exp( -0.5 * time_exp * time_exp ) / ( T_sr * sqrt(TWO_PI) ) ;
        }

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
        inline double gaussian( const matrix<double>& mu, const matrix<double>& sigma ) {
            matrix<double> s_inv(sigma) ;
            inverse( sigma, s_inv ) ;
            double det = determinent(TWO_PI*sigma) ;
            matrix<double> mu_trans = trans(mu) ;
            matrix<double> mu_prod = prod( mu_trans, s_inv ) ;
            matrix<double> kappa = prod( mu_prod, mu ) ;
            det = 1.0 / sqrt(det) ;
            kappa *= -0.5 ;
            return det * exp( kappa(0,0) ) ;
        }

        /**
         * Calculates the matrix determinent of a uBlas matrix
         */
        inline double determinent( const matrix<double>& m ) {
            double det = 1.0 ;
            matrix<double> a(m) ;
            permutation_matrix<size_t> pivot( a.size1() ) ;
            if( lu_factorize(a,pivot) ) return 0.0 ;
            for(size_t i=0; i<pivot.size(); ++i) {
                if (pivot(i) != i) det *= -1.0 ;
                det *= a(i,i) ;
            }
            return det ;
        }

        /**
         * Computes the inverse of a matrix.
         */
        inline bool inverse( const matrix<double>& m, matrix<double>& i) {
            matrix<double> a(m) ;
            permutation_matrix<size_t> pm(a.size1()) ;
            int result = lu_factorize(a, pm) ;
            if( result != 0 ) return false ;
            i.assign(identity_matrix<double>(a.size1())) ;
            lu_substitute(a, pm, i) ;
            return true ;
        }

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

    /**** Member Variables ****/

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

};

}   // end of namespace waveq3d
}   // end of namespace usml

#endif
