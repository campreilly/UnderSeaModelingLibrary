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
class USML_DECLSPEC beam_pattern_omni : public beam_pattern_model {

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
         * @param orient        Orientation of the array
         * @param frequencies   List of frequencies to compute beam level for
         * @param level         Beam level for each frequency (linear units)
         */
        virtual void beam_level( double de, double az,
                                 orientation& orient,
                                 const vector<double>& frequencies,
                                 vector<double>* level) ;

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
