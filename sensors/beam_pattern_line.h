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
 */
class beam_pattern_line : public beam_pattern_model {

    public:

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
         * @param roll      rotation of the beam around the North/South axis (up positive)
         * @param pitch     rotation of the beam around the East/West axis (up positive)
         * @param yaw       rotation of the beam around the Up/Down axis (clockwise positive)
         */
        virtual void orient_beam( double roll, double pitch, double yaw ) = 0 ;

    protected:

        /**
         * Number of elements on the linear array
         */
        size_t _n ;

        /**
         * Local cache of commonly computed values
         * using the frequencies.
         */
        vector<double> _omega ;
        vector<double> _omega_n ;

        /**
         * Local cache of commonly computed
         * using the steering angles
         */
        vector<double> _steering ;
        vector<double> _steering_n ;

        /**
         * Spatial orientation of the array
         */
        double _roll ;
        double _pitch ;
        double _yaw ;

        /**
         * Initializes the beam pattern. To save execution time, common computations
         * are done and stored locally to be used at a later time. since directivity
         * index is purely depenedent on the physical parameters of the array, this can
         * be computed and then returned as needed.
         */
        void initialize_beams( double sound_speed, double spacing,
                               const seq_vector& frequencies,
                               vector<double>& steering_angles ) ;

};

/// @}
}   // end of namespace sensors
}   // end of namespace usml
