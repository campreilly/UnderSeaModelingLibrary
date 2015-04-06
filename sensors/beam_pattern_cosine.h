/**
 * @file beam_pattern_cosine.h
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
 * Models a North/South-directional beam pattern. This
 * pattern can be simulated by a line array with two
 * elements that are spaced by half the wavelength.
 *
 * NOTE: All computation are done in traditional theta and phi
 * of spherical coordinates. As such all DE, AZ, roll, pitch,
 * and yaw are transformed before used in computations.
 */
class USML_DECLSPEC beam_pattern_cosine : public beam_pattern_model {

    public:

        /**
         * Constructs a cosine-directional beam pattern.
         */
        beam_pattern_cosine() {}

        /**
         * Computes the response level in a specific DE/AZ pair and
         * beam steering angle. The return, level, is passed
         * back in linear units.
         *
         * @param de            Depression/Elevation angle (rad)
         * @param az            Azimuthal angle (rad)
         * @param theta         spherical offset in theta from reference axis (rad)
         * @param phi           spherical offset in phi from reference axis (rad)
         * @param frequencies   list of frequencies to compute beam level for
         * @param level         beam level for each frequency
         */
        virtual void beam_level( double de, double az,
                                 double theta, double phi,
                                 const vector<double>& frequencies,
                                 vector<double>* level ) ;

        /**
         * Directivity index for an cosine-directional beam pattern
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
