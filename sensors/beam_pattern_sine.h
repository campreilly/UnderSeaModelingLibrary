/**
 * @file beam_pattern_sine.h
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
 * Models a East/West-directional beam pattern. This
 * pattern can be simulated by a line array with two
 * elements that are spaced by half the wavelength.
 *
 * NOTE: All computation are done in traditional theta and phi
 * of spherical coordinates. As such all DE, AZ, roll, pitch,
 * and yaw are transformed before used in computations.
 */
class beam_pattern_sine : public beam_pattern_model {

    public:

        /**
         * Constructs a sine-directional beam pattern.
         */
        beam_pattern_sine() {
            _roll = 0.0 ;
            _pitch = M_PI_2 ;
            _yaw = 0.0 ;
        }

        /**
         * Computes the response level in a specific DE/AZ pair and
         * beam steering angle. The return, level, is passed
         * back in linear units.
         *
         * @param de            Depression/Elevation angle (rad)
         * @param az            Azimuthal angle (rad)
         * @param frequencies   list of frequencies to compute beam level for
         * @param level         beam level for each frequency
         */
        virtual void beam_level( double de, double az,
                                 const vector<double>& frequencies,
                                 vector<double>* level ) ;

        /**
         * Rotates the array by a given roll, pitch, and yaw
         *
         * @param roll      rotation of the beam around the North/South axis (up positive)
         * @param pitch     rotation of the beam around the East/West axis (up positive)
         * @param yaw       rotation of the beam around the Up/Down axis (clockwise positive)
         */
        virtual void orient_beam( double roll, double pitch, double yaw ) ;

        /**
         * Directivity index for an sine-directional beam pattern
         * The gain for this type of beam pattern is 0 dB.
         *
         * @param frequencies   list of frequencies to compute DI for
         * @param level         gain for each frequency
         */
        virtual void directivity_index( const vector<double>& frequencies,
                                        vector<double>* level ) ;

    protected:

        /**
         * Spatial orientation of the array
         */
        double _roll ;
        double _pitch ;
        double _yaw ;

};

/// @}
}   // end of namespace sensors
}   // end of namespace usml
