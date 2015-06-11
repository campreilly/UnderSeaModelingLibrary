/**
 * @file beam_pattern_cosine.h
 * Frequency independent North/South directional beam pattern.
 */
#pragma once

#include <usml/sensors/beam_pattern_model.h>

namespace usml {
namespace sensors {

using boost::numeric::ublas::vector ;
using namespace usml::types ;

/// @ingroup beams
/// @{

/**
 * Models a North/South-directional beam pattern. This
 * pattern can be simulated by a line array with two
 * elements that are spaced by half the wavelength.
 *
 * Beam level is given as below:
 * \f[
 *      b(\theta,\phi,f,L,\theta_0,\phi_0) =
 *          \left[ l+(1-l) \hat{r} \cdot \hat{k} \right]^2
 * \f]
 * where
 *
 *      \n \f$ \theta \f$ is the rotation from the positive up-axis,
 *          \f$ \theta \in \left[ 0, \pi \right] \f$
 *      \n \f$ \phi \f$ is the clockwise rotation from the positive north-axis,
 *          \f$ \phi \in \left[ 0, 2\pi \right] \f$
 *      \n \f$ f \f$ is the frequency of interest
 *      \n \f$ L \f$ is the minimum beam level for the pattern's null zones
 *      \n \f$ \theta_0 \f$ is the array's orientation from the positive up-axis,
 *          \f$ \theta_0 \in \left[ 0, \pi \right] \f$
 *      \n \f$ \phi_0 \f$ is the array's orientation from the positive north-axis,
 *          \f$ \phi_0 \in \left[ 0, 2\pi \right] \f$
 *      \n \f$ l \f$ is given as \f$ 10^{L / 10} \f$
 *      \n \f$ \hat{r} \f$ is the rotated reference axis of the beam pattern
 *      \n \f$ \hat{k} \f$ is the incident wave direction
 *
 * NOTE: All computation are done in traditional theta and phi
 * of spherical coordinates. As such all DE, AZ, roll, pitch,
 * and yaw are transformed before used in computations.
 */
class USML_DECLSPEC beam_pattern_cosine : public beam_pattern_model {

    public:

        /**
         * Constructs a cosine-directional beam pattern.
         *
         * @param null      minimum loss value (dB)
         */
        beam_pattern_cosine( double null = -300.0 ) ;

        /**
         * Destructor
         */
        virtual ~beam_pattern_cosine() ;

        /**
         * Computes the response level in a specific DE/AZ pair and
         * beam steering angle. The return, level, is passed
         * back in linear units.
         *
         * @param de            Depression/Elevation angle (rad)
         * @param az            Azimuthal angle (rad)
         * @param orient        Orientation of the array
         * @param frequencies   List of frequencies to compute beam level for
         * @param level         Beam level for each frequency (squared linear units)
         */
        virtual void beam_level( double de, double az,
                                 orientation& orient,
                                 const vector<double>& frequencies,
                                 vector<double>* level) ;

        /**
         * Directivity index for an cosine-directional beam pattern
         *
         * @param frequencies   list of frequencies to compute DI for
         * @param level         gain for each frequency
         */
        virtual void directivity_index( const vector<double>& frequencies,
                                        vector<double>* level ) ;

    private:

        /**
         * Minimum loss value in a null zone (linear)
         */
        double _null ;

        /**
         * Gain factor of the array, 1.0 - null (linear)
         */
        double _gain ;

        /**
         * Direcitivity index of the array
         */
        double _directivity_index ;

};

/// @}
}   // end of namespace sensors
}   // end of namespace usml
