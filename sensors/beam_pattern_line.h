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
class USML_DECLSPEC beam_pattern_line : public beam_pattern_model {

    public:

        /**
         * Type of linear array
         * Specifies which axis is the reference axis of
         * the array. Vertical being in the z-direction
         * spatially and horizontal being in the xy-planar
         * direction.
         */
        typedef enum { VERTICAL, HORIZONTAL } orientation_axis ;

        /**
         * Constructs a beam pattern for a linear array.
         *
         * @param sound_speed       speed of sound in water at the array
         * @param spacing           distance between each element on the array
         * @param elements          number of elements on the line array
         * @param steering_angle    list of steering angles relative to the
         *                          reference axis
         * @param axis              the reference axis of the array.
         */
        beam_pattern_line( double sound_speed, double spacing,
                           size_t elements, double steering_angle,
                           orientation_axis axis=VERTICAL ) ;

        /**
         * Destructor
         */
        virtual ~beam_pattern_line() ;

        /**
         * Computes the response level in a specific DE/AZ pair and
         * beam steering angle. The return, level, is passed
         * back in linear units.
         *
         * @param de            Depression/Elevation angle (rad)
         * @param az            Azimuthal angle (rad)
         * @param orient        Orientation of the array
         * @param frequencies   List of frequencies to compute beam level for
         * @param level         Beam level for each frequency (linear units)
         */
        virtual void beam_level( double de, double az,
                                 orientation& orient,
                                 const vector<double>& frequencies,
                                 vector<double>* level) ;

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

        /**
         * Local cache of commonly computed
         * using the steering angles
         */
        double _steering ;

        /**
         * Defines the reference axis for this linear array's beam
         * pattern.
         */
        orientation_axis _axis ;

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
