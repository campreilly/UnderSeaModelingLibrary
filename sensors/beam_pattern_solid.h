/**
 * @file beam_pattern_solid.h
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
 * Models a beam pattern that has a maximum response of one
 * in a specific solid angle and zero everywhere else.
 *
 * NOTE: The reference axis for this beam pattern is
 * in the zero DE direction. As such no transformations
 * are necessary prior to computation.
 */
class beam_pattern_solid : public beam_pattern_model {

    public:

        /**
         * Constructors an solid-angle beam pattern.
         * DE \elem [-90.0 90.0], AZ \elem [0.0 360.0]
         *
         * @param max_de        maximum DE of the solid angle (deg)
         * @param min_de        minimum DE of the solid angle (deg)
         * @param max_az        maximum AZ of the solid angle (deg)
         * @param min_az        minimum AZ of the solid angle (deg)
         * @param frequencies   list of operating frequencies
         */
        beam_pattern_solid( double max_de, double min_de,
                            double max_az, double min_az,
                            const seq_vector& frequencies )
            : _max_de(max_de), _min_de(min_de),
              _max_az(max_az), _min_az(min_az)
        {
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
         * @param roll      rotation of the beam around the North/South axis (up positive)
         * @param pitch     rotation of the beam around the East/West axis (up positive)
         * @param yaw       rotation of the beam around the Up/Down axis (clockwise positive)
         */
        virtual void orient_beam( double roll, double pitch, double yaw ) ;

    protected:

        /**
         * Solid angle information
         */
        double _max_de ;
        double _min_de ;
        double _max_az ;
        double _min_az ;

        /**
         * Spatial orientation of the array
         */
        double _roll ;
        double _pitch ;
        double _yaw ;

        /**
         * The limits of the solid angle including the
         * pitch and yaw of the array.
         */
        double _up ;        // absolute maximum DE
        double _down ;      // absolute minimum DE
        double _right ;     // absolute maximum AZ
        double _left ;      // absolute minimun AZ

        /**
         * The directivity index array size to frequencies size and
         * zero.
         */
        void initialize_beams( const seq_vector& frequencies ) ;

};

/// @}
}   // end of namespace sensors
}   // end of namespace usml
