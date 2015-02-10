/**
 * @file beam_pattern_omni.h
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
 * Models an omni-directional beam pattern.
 */
class beam_pattern_omni : public beam_pattern_model {

    public:

        /**
         * Constructors an omni-directional beam pattern.
         */
        beam_pattern_omni() {}

        /**
         * Computes the response level in a specific DE/AZ pair and
         * beam steering angle. The return, level, is passed
         * back in linear units.
         *
         * @param de            Depression/Elevation angle (rad)
         * @param az            Azimuthal angle (rad)
         * @param freuencies    list of frequencies to compute beam level for
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
        virtual void orient_beam( double roll, double pitch, double yaw ) {}

        /**
         * Directivity index for an omni-directional beam pattern
         * The gain for this type of beam pattern is 0 dB.
         *
         * @param frequencies   list of frequencies to compute DI for
         * @param level         gain for each frequency
         */
        virtual void directivity_index( const vector<double>& frequencies,
                                        vector<double>* level ) ;
};

/// @}
}   // end of namespace sensors
}   // end of namespace usml
