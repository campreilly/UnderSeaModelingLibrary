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
class USML_DECLSPEC beam_pattern_solid : public beam_pattern_model {

    public:

        /**
         * Constructs a solid-angle beam pattern.
         * DE \elem [-90.0 90.0], AZ \elem [0.0 360.0]
         *
         * @param max_de        maximum DE of the solid angle (deg)
         * @param min_de        minimum DE of the solid angle (deg)
         * @param max_az        maximum AZ of the solid angle (deg)
         * @param min_az        minimum AZ of the solid angle (deg)
         */
        beam_pattern_solid( double max_de, double min_de,
                            double max_az, double min_az )
            : _max_de(max_de), _min_de(min_de),
              _max_az(max_az), _min_az(min_az)
        {
            initialize_beam() ;
        }

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
         * Directivity index for a beam pattern of solid angle.
         *
         * @param frequencies   list of frequencies to compute DI for
         * @param level         gain for each frequency
         */
        virtual void directivity_index( const vector<double>& frequencies,
                                        vector<double>* level ) ;

    protected:

        /**
         * Directivity index of this beam pattern
         */
        double _directivity_index ;

        /**
         * Solid angle information
         */
        double _max_de ;
        double _min_de ;
        double _max_az ;
        double _min_az ;

        /**
         * The directivity index array size to frequencies size and
         * zero.
         */
        void initialize_beam() ;

};

/// @}
}   // end of namespace sensors
}   // end of namespace usml
