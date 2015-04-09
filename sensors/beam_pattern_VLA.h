/**
 * @file beam_pattern_VLA.h
 */
#pragma once

#include <usml/sensors/beam_pattern_line.h>
#include <usml/sensors/orientation_VLA.h>

namespace usml {
namespace sensors {

using boost::numeric::ublas::vector ;
using namespace usml::types ;

/// @ingroup beams
/// @{

/**
 * Models a beam pattern constructed from a vertical line array, VLA.
 * This class utilizes many features from the beam_pattern_line class
 * but requires specialized computations for rotations, without the
 * requirement of multiple orientation classes for a given sensor.
 */
class USML_DECLSPEC beam_pattern_VLA : public beam_pattern_line {

    public:

        /**
         * Constructs a beam pattern for a linear array.
         *
         * @param sound_speed       speed of sound in water at the array
         * @param spacing           distance between each element on the array
         * @param elements          number of elements on the line array
         * @param steering_angle    list of steering angles relative to the
         *                          reference axis
         */
        beam_pattern_VLA( double sound_speed, double spacing,
                           size_t elements, double steering_angle ) ;

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

    protected:

        /**
         * Local orientation that uses specific calculation for a VLA.
         */
        orientation_VLA _orient_VLA ;

};

/// @}
}   // end of namespace sensors
}   // end of namespace usml
