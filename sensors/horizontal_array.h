/**
 * @file horizontal_array.h
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
 * horizontal array.
 */
class horizontal_array : public beam_pattern_line {

    public:

        /**
         * Cosntructor for a horizontally oriented array of linear elements.
         * The convention for the pitch and theta angles are from the
         * horizontal, as such the steering angles and the pitch are corrected
         * by adding a pi/2.
         *
         * @param sound_speed   speed of sound in water at the array
         * @param spacing       distance between each element on the array
         * @param elements      number of elements on the line array
         */
        horizontal_array( double c0, double d, size_t elements,
                          const seq_vector& freq,
                          vector<double>* steering_angles=NULL )
         {
            _n = elements ;
            _omega = vector<double>( freq.size() ) ;
            _omega_n = vector<double>( freq.size() ) ;
            if( !steering_angles ) {
                vector<double> steerings = scalar_vector<double>( 1, M_PI_2 ) ;
                initialize_beams( c0, d, freq, steerings ) ;
            } else {
                (*steering_angles) = (*steering_angles) + M_PI_2 ;
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
            _pitch = -(pitch + M_PI_2) ;
            _yaw = -yaw ;
        }

};

/// @}
}   // end of namespace sensors
}   // end of namespace usml
