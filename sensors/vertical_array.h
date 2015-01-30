/**
 * @file vertical_array.h
 */
#pragma once

#include <usml/sensors/beam_pattern_line.h>

namespace usml {
namespace sensors {

using boost::numeric::ublas::vector ;
using namespace usml::types ;

/// @ingroup beams
/// @{

/**
 * Specialized class for a beam pattern that models a
 * vertical array.
 */
class vertical_array : public beam_pattern_line {

    public:

        /**
         * Cosntructor for a vertically oriented array of linear elements
         *
         * @param sound_speed   speed of sound in water at the array
         * @param spacing       distance between each element on the array
         * @param elements      number of elements on the line array
         */
        vertical_array( double c0, double d, size_t elements,
                        const seq_vector& freq,
                        vector<double>* steering_angles=NULL )
         {
            _n = elements ;
            if( !steering_angles ) {
                vector<double> steerings = scalar_vector<double>( 1, 0.0 ) ;
                initialize_beams( c0, d, freq, steerings ) ;
            } else {
                initialize_beams( c0, d, freq, *steering_angles ) ;
            }
         }

        /**
         * Rotates the array by a given roll, pitch, and yaw
         *
         * @param roll      rotation of the beam around the North/South axis (clockwise positive)
         * @param pitch     rotation of the beam around the East/West axis (clockwise positive)
         * @param yaw       rotation of the beam around the Up/Down axis (up positive)
         */
        virtual void orient_beam( double roll, double pitch, double yaw ) {
            _roll = roll ;
            _pitch = -pitch ;
            _yaw = -yaw ;
        }

};

/// @}
}   // end of namespace sensors
}   // end of namespace usml
