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
         * Constructor
         */
        beam_pattern_omni( const seq_vector& frequencies) {
            initialize_beams( frequencies ) ;
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
         * Creating the beam_level return vector at instantion saves time
         * by not requiring a constructor call when needed.
         */
        vector<double> _level ;

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
