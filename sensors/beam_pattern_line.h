/**
 * @file beam_pattern_line.h
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
 * Models a beam pattern constructed from an array of elements
 * that are linearly oriented and spaced apart along the array's
 * major axis.
 */
class beam_pattern_line : public beam_pattern_model {

    public:

        /**
         * Type of linear array
         * Specifies which axis is the reference axis of
         * the array. Vertical being in the z-direction
         * spatially and horizontal being in the xy-planar
         * direction.
         */
        typedef enum { VERTICAL, HORIZONTAL } reference_axis ;

        /**
         * Constructs a beam pattern for a linear array.
         *
         * @param sound_speed       speed of sound in water at the array
         * @param spacing           distance between each element on the array
         * @param elements          number of elements on the line array
         * @param frequencies       list of operating frequencies
         * @param steering_angles   list of steering angles relative to the
         *                          reference axis
         * @param reference_axis    the reference axis of the array.
         */
        beam_pattern_line( double c0, double d, size_t elements,
                           const seq_vector& frequencies,
                           vector<double>* steering_angles=NULL,
                           reference_axis axis=VERTICAL ) : _axis(axis)
        {
            switch( _axis ) {
                case HORIZONTAL :
                    _n = elements ;
                    if( !steering_angles ) {
                        vector<double> steerings = scalar_vector<double>( 1, M_PI_2 ) ;
                        initialize_beams( c0, d, frequencies, steerings ) ;
                    } else {
                        (*steering_angles) = (*steering_angles) + M_PI_2 ;
                        initialize_beams( c0, d, frequencies, *steering_angles ) ;
                    }
                    _pitch = M_PI_2 ;
                    break ;
                default :
                    _n = elements ;
                    if( !steering_angles ) {
                        vector<double> steerings = scalar_vector<double>( 1, 0.0 ) ;
                        initialize_beams( c0, d, frequencies, steerings ) ;
                    } else {
                        initialize_beams( c0, d, frequencies, *steering_angles ) ;
                    }
                    break ;
            }
        }

        /**
         * Computes the beam level
         *
         * @param  de            Depression/Elevation angle
         * @param  az            Azimuthal angle
         * @param  beam          beam steering to find response level for
         * @return level         beam level for each frequency (linear)
         */
        virtual void beam_level( double de, double az, size_t beam,
                                 vector<double>* level ) ;

        /**
         * Rotates the array by a given roll, pitch, and yaw
         *
         * @param roll      rotation of the beam around the North/South axis (up positive)
         * @param pitch     rotation of the beam around the East/West axis (up positive)
         * @param yaw       rotation of the beam around the Up/Down axis (clockwise positive)
         */
        virtual void orient_beam( double roll, double pitch, double yaw ) {
            if( _axis == HORIZONTAL ) {
                _roll = roll ;
                _pitch = -(pitch + M_PI_2) ;
                _yaw = -yaw ;
            } else {
                _roll = roll ;
                _pitch = -pitch ;
                _yaw = -yaw ;
            }
        }

    protected:

        /**
         * Number of elements on the linear array
         */
        size_t _n ;

        /**
         * Local cache of commonly computed values
         * using the frequencies.
         */
        vector<double> _omega ;
        vector<double> _omega_n ;

        /**
         * Local cache of commonly computed
         * using the steering angles
         */
        vector<double> _steering ;
        vector<double> _steering_n ;

        /**
         * Spatial orientation of the array
         */
        double _roll ;
        double _pitch ;
        double _yaw ;

        /**
         * Defines the reference axis for this linear array's beam
         * pattern.
         */
        reference_axis _axis ;

        /**
         * Initializes the beam pattern. To save execution time, common computations
         * are done and stored locally to be used at a later time. since directivity
         * index is purely depenedent on the physical parameters of the array, this can
         * be computed and then returned as needed.
         */
        void initialize_beams( double sound_speed, double spacing,
                               const seq_vector& frequencies,
                               vector<double>& steering_angles ) ;

};

/// @}
}   // end of namespace sensors
}   // end of namespace usml
