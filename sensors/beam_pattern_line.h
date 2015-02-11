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
 *
 * NOTE: All computation are done in traditional theta and phi
 * of spherical coordinates. As such all DE, AZ, roll, pitch,
 * and yaw are transformed before used in computations.
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
         * @param steering_angles   list of steering angles relative to the
         *                          reference axis
         * @param axis              the reference axis of the array.
         */
        beam_pattern_line( double sound_speed, double spacing,
                           size_t elements, double steering_angle,
                           reference_axis axis=VERTICAL ) : _axis(axis)
        {
            switch( _axis ) {
                case HORIZONTAL :
                    _n = elements ;
                    initialize_beams( sound_speed, spacing, (steering_angle + M_PI_2) ) ;
                    break ;
                default :
                    _n = elements ;
                    initialize_beams( sound_speed, spacing, steering_angle ) ;
                    break ;
            }
        }

        /**
         * Computes the response level in a specific DE/AZ pair and
         * beam steering angle. The return, level, is passed
         * back in linear units.
         *
         * @param de            Depression/Elevation angle (rad)
         * @param az            Azimuthal angle (rad)
         * @param pitch         pitch in the DE dimension (rad)
         * @param yaw           yaw in the AZ dimension (rad)
         * @param beam          beam steering to find response level (size_t)
         * @param frequencies   frequencies to compute beam level for
         * @param level         beam level for each frequency
         */
        virtual void beam_level( double de, double az,
                                 double pitch, double yaw,
                                 const vector<double>& frequencies,
                                 vector<double>* level ) ;

        /**
         * Computes the directivity index for a list of frequencies
         *
         * @param frequencies   frequencies to determine DI at
         * @param level         gain for the provided frequencies
         */
        virtual void directivity_index( const vector<double>& frequencies,
                                        vector<double>* level ) ;

    protected:

        /**
         * Number of elements on the linear array
         */
        size_t _n ;

        /**
         * Local cache of commonly computed values
         * for frequency computations.
         */
        double _omega ;
        double _omega_n ;

        /**
         * Local cache of commonly computed
         * using the steering angles
         */
        double _steering ;
        double _steering_n ;

        /**
         * Defines the reference axis for this linear array's beam
         * pattern.
         */
        reference_axis _axis ;

        /**
         * Initializes the beam pattern. To save execution time, common computations
         * are done and stored locally to be used at a later time.
         */
        void initialize_beams( double sound_speed,
                               double spacing,
                               double steering_angle ) ;

};

/// @}
}   // end of namespace sensors
}   // end of namespace usml
