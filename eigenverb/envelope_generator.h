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
         *
         * @param source
         * @param receiver
         * @param curves
         */
        void generate_envelopes(
                const eigenverb_collection& source,
                const eigenverb_collection& receiver,
                envelope_collection* levels ) ;

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
         *
         * @param u
         * @param v
         * @param levels
         */
         void compute_contribution(
             const eigenverb* u, const eigenverb* v,
             envelope_collection* levels ) ;

        /**
         * Computes the energy contributions to the reverberation
         * energy curve from the bottom interactions.
         *
         * @param source
         * @param receiver
         * @param levels
         */
        virtual void compute_bottom_energy(
                const eigenverb_collection& source,
                const eigenverb_collection& receiver,
                envelope_collection* levels ) = 0 ;

        /**
         * Computes the energy contributions to the reverberation
         * energy curve from the surface interactions.
         *
         * @param source
         * @param receiver
         * @param levels
         */
        virtual void compute_surface_energy(
                const eigenverb_collection& source,
                const eigenverb_collection& receiver,
                envelope_collection* levels ) = 0 ;

        /**
         * Calculate the contributions due to collisions from below
         * a volume layer.
         *
         * @param source
         * @param receiver
         * @param levels
         */
        virtual void compute_upper_volume_energy(
                const eigenverb_collection& source,
                const eigenverb_collection& receiver,
                envelope_collection* levels ) = 0 ;

        /**
         * Calculate the contributions due to collisions from above
         * a volume layer.
         *
         * @param source
         * @param receiver
         * @param levels
         */
        virtual void compute_lower_volume_energy(
                const eigenverb_collection& source,
                const eigenverb_collection& receiver,
                envelope_collection* levels ) = 0 ;

        /**
         * Pulse length of the signal (sec)
         */
        double _pulse ;

        /**
         * Max time for the reverberation curve
         */
        double _max_time ;

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
