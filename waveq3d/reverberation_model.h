/**
 * @file reverberation_model.h
 * Listens for interface collision callbacks from a wavefront.
 */
#ifndef USML_WAVEQ3D_REVERBERATION_MODEL_H
#define USML_WAVEQ3D_REVERBERATION_MODEL_H

#include <usml/ublas/ublas.h>
#include <usml/ublas/math_traits.h>
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

        /**
         * Computes the reverberation curve from the data cataloged from the
         * wavefront(s).
         */
        virtual void compute_reverberation() = 0 ;

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
         */
        inline double gaussian( matrix<double> mu, matrix<double> sigma ) {
            matrix<double> mu_trans = boost::numeric::ublas::trans(mu) ;
            matrix<double> mu_prod = prod( mu_trans, sigma ) ;
            double kappa = prod( mu_prod, mu ) ;
            double det = TWO_PI * sigma(0,0)*sigma(1,1)
                         - TWO_PI * sigma(0,1)*sigma(1,0) ;
            det = 1.0 / sqrt(det) ;
            kappa *= -0.5 ;
            return det * exp( kappa(0,0) ) ;
        }

};

/// @}
}  // end of namespace waveq3d
}  // end of namespace usml

#endif
