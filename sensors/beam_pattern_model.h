/**
 *  @file beam_pattern_model.h
 *  Generic interface for beam patterning
 */
#pragma once

#include <boost/numeric/ublas/vector_expression.hpp>
#include <usml/types/types.h>

namespace usml {
namespace sensors {

using boost::numeric::ublas::vector ;
using namespace usml::types ;

/// @ingroup beams
/// @{

/**
 * A "beam pattern" computes the gain for an incident wave as a function
 * of incident angles, beam steering angle, and frequency.
 *
 * This class implements an abstract func   tion used to spatially orient
 * the array, orient_beam, and two abstract functions to compute both
 * the directivity index of the array and the beam level gain.
 *
 * Many properties of the beam patterns depend on predetermined values.
 * Such as the beam steering angles, frequency spectrum, and physical
 * arrangement of the elements. Using these, many variables can be
 * pre-computed and cached locally to reduce computation time.
 */
class USML_DECLSPEC beam_pattern_model {

    public:

        /**
         * Computes the beam level gain along a specific DE and AZ direction
         * for a specific beam steering angle. The DE and AZ are passed in as
         * Eta/VarPhi values and then transformed to a theta/phi equivalent
         * that are used for computation.
         *
         * NOTE: The choice of return in linear units
         * is a development choice and may be changed depending on how
         * this value is used in the final release state.
         *
         * @param  de            Depression/Elevation angle
         * @param  az            Azimuthal angle
         * @param  beam          beam steering index
         * @return level         beam level for each frequency (linear)
         */
        virtual void beam_level( double de, double az, size_t beam,
                                 vector<double>* level ) = 0 ;

        /**
         * Rotates the array by a given roll, pitch, and yaw
         *
         * @param roll      rotation of the beam around the North/South axis (clockwise positive)
         *                  Up is 0 and Down is PI.
         * @param pitch     rotation of the beam around the East/West axis (clockwise positive)
         *                  Up is 0 and Down is PI.
         * @param yaw       rotation of the beam around the Up/Down axis (clockwise positive)
         *                  North is 0 and South is PI.
         */
        virtual void orient_beam( double roll, double pitch, double yaw ) = 0 ;

        /**
         * Accesor to the directivity index
         *
         * @param   f    frequency index
         * @return       directivity index for this frequency
         */
        inline double directivity_index( size_t f ) const {
            return _directivity_index[f] ;
        }

        /**
         * Destructor
         */
        virtual ~beam_pattern_model() {}

    protected:

        /**
         * Cache of the directivity index for an array.
         */
        vector<double> _directivity_index ;

};

/// @}
}   // end of namespace sensors
}   // end of namespace usml
