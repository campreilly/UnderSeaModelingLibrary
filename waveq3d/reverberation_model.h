/**
 * @file reverberation_model.h
 * Listens for interface collision callbacks from a wavefront.
 */
#ifndef USML_WAVEQ3D_REVERBERATION_MODEL_H
#define USML_WAVEQ3D_REVERBERATION_MODEL_H

#include <usml/ocean/ocean.h>
#include <usml/ublas/ublas.h>
#include <usml/ublas/math_traits.h>
#include <usml/types/types.h>
#include <boost/numeric/ublas/lu.hpp>

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
               const wposition1& position, const wvector1& ndirection, unsigned ID ) = 0 ;

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
               const wposition1& position, const wvector1& ndirection, unsigned ID ) = 0 ;

        /**
         * Computes the reverberation curve from the data cataloged from the
         * wavefront(s).
         */
        virtual void compute_reverberation() = 0 ;

        /**
         * Returns the reverberation data
         */
        virtual const vector<double> getReverberation_curve() = 0 ;

    protected:

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
            matrix<double> mu_trans = trans(mu) ;
            matrix<double> s_inv(sigma) ;
            inverse(sigma, s_inv) ;
            matrix<double> mu_prod = prod( mu_trans, s_inv ) ;
            matrix<double> kappa = prod( mu_prod, mu ) ;
            double det = determinent(sigma) ;
            det = 1.0 / sqrt(TWO_PI*TWO_PI*det) ;
            kappa *= -0.5 ;
            return det * exp( kappa(0,0) ) ;
        }

        /**
         * Calculates the matrix determinent of a uBlas matrix
         */
        inline double determinent( const matrix<double>& m ) {
            matrix<double> a(m) ;
            permutation_matrix<size_t> pivot( a.size1() ) ;
            if( lu_factorize(a,pivot) ) return 0.0 ;
            double det = 1.0 ;
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
};

/// @}
}  // end of namespace waveq3d
}  // end of namespace usml

#endif
