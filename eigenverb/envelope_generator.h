/**
 * @file envelope_generator.h
 */
#pragma once

//#define EIGENVERB_COLLISION_DEBUG
//#define EIGENVERB_MODEL_DEBUG

#include <usml/ocean/ocean.h>
#include <usml/eigenverb/eigenverb_collection.h>
#include <usml/eigenverb/envelope_collection.h>
#include <iostream>
#include <fstream>

namespace usml {
namespace eigenverb {

using namespace usml::ocean ;
using namespace boost::numeric::ublas ;

/// @ingroup waveq3d
/// @{

class USML_DECLSPEC envelope_generator {

    public:

        /**
         * Virtual destructor
         */
        virtual ~envelope_generator() {}

        /**
         * Computes the reverberation curve from the data cataloged from the
         * wavefront(s).
         */
        virtual void compute_reverberation() {
            compute_bottom_energy() ;
            compute_surface_energy() ;
            compute_upper_volume_energy() ;
            compute_lower_volume_energy() ;
        }

         /**
          * Saves the eigenverb data to a text file.
          */
         virtual void save_eigenverbs(const char* filename) {}

    protected:

        /**
         * Storage for the envelope collection
         */
        envelope_collection* _envelope ;

        /**
         * Computes the contribution value of two eigenverbs to the total
         * reverberation level.
         */
         void compute_contribution( const eigenverb* u, const eigenverb* v ) ;

        /**
         * Computes the energy contributions to the reverberation
         * energy curve from the bottom interactions.
         */
        virtual void compute_bottom_energy() = 0 ;

        /**
         * Computes the energy contributions to the reverberation
         * energy curve from the surface interactions.
         */
        virtual void compute_surface_energy() = 0 ;

        /**
         * Calculate the contributions due to collisions from below
         * a volume layer.
         */
        virtual void compute_upper_volume_energy() = 0 ;

        /**
         * Calculate the contributions due to collisions from above
         * a volume layer.
         */
        virtual void compute_lower_volume_energy() = 0 ;

        /**
         * Pulse length of the signal (sec)
         */
        double _pulse ;

        /**
         * Number of time bins to store the energy distribution
         */
        size_t _max_index ;

        /**
         * Max time for the reverberation curve
         */
        double _max_time ;

        /**
         * Number of layers within the volume
         */
        size_t _n ;

        /**
         * Defines the type of boundary model for the bottom.
         */
        boundary_model* _bottom_boundary ;

        /**
         * Defines the type of boundary model for the surface.
         */
        boundary_model* _surface_boundary ;

        /**
         * The current boundary that computations need
         */
        boundary_model* _current_boundary ;

        /**
         * Time resolution of the reverberation curve. This is used to spread
         * the energy from a contribution out in time along the curve.
         */
        vector<double> _two_way_time ;

};

/// @}
}   // end of namespace waveq3d
}   // end of namespace usml
