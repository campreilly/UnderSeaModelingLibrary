/**
 * @file beam_pattern_cosine.h
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
 * NOTE: All computation are done in traditional theta and phi
 * of spherical coordinates. As such all DE, AZ, roll, pitch,
 * and yaw are transformed before used in computations.
 */
class beam_pattern_cosine : public beam_pattern_model {

    public:

        /**
         * Constructors an cosine-directional beam pattern.
         * Uses the list of frequencies to construct vectors
         * of the necessary size for computation.
         *
         * @param frequencies   list of the operating frequencies
         */
        beam_pattern_cosine( const seq_vector& frequencies ) {
            initialize_beams( frequencies ) ;
        }

        /**
         * Computes the response level in a specific DE/AZ pair and
         * beam steering angle. The return, level, is passed
         * back in linear units.
         *
         * @param  de            Depression/Elevation angle (rad)
         * @param  az            Azimuthal angle (rad)
         * @param  beam          beam steering to find response level (size_t)
         * @param  level         beam level for each frequency
         */
        virtual void beam_level( double de, double az, size_t beam,
                                 vector<double>* level ) ;

        /**
         * Rotates the array by a given roll, pitch, and yaw
         *
         * @param roll      rotation of the beam around the North/South axis (clockwise positive)
         * @param pitch     rotation of the beam around the East/West axis (clockwise positive)
         * @param yaw       rotation of the beam around the Up/Down axis (up positive)
         */
        virtual void orient_beam( double roll, double pitch, double yaw ) ;

    protected:

        /**
         * The directivity index array size to frequencies size and
         * zero.
         */
        void initialize_beams( const seq_vector& frequencies ) ;

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
