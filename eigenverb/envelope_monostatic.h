/**
 * @file envelope_monostatic.h
 */
#pragma once

#include <usml/eigenverb/envelope_generator.h>

namespace usml {
namespace eigenverb {

using namespace usml::ocean ;
using namespace boost::numeric::ublas ;

/// @ingroup eigenverb
/// @{

/**
 * A reverberation model listens for interface collision callbacks from
 * a wavefront. Used in monostatic scenarios, i.e. source and receiver are
 * co-located.
 */
class USML_DECLSPEC envelope_monostatic : public envelope_generator {

    public:

        envelope_monostatic( ocean_model& ocean,
                              size_t num_radials,
                              double pulse,
							  size_t num_bins,
                              double max_time ) ;

        virtual ~eigenverb_monostatic() {}

         /**
          * Saves the eigenverb data to a text file.
          */
         virtual void save_eigenverbs(const char* filename) ;

    private:

        /**
         * Computes the energy contributions to the reverberation
         * energy curve from the bottom interactions.
         */
        virtual void compute_bottom_energy() ;

        /**
         * Computes the energy contributions to the reverberation
         * energy curve from the surface interactions.
         */
        virtual void compute_surface_energy() ;

        /**
         * Calculate the contributions due to collisions from below
         * a volume layer.
         */
        virtual void compute_upper_volume_energy() ;

        /**
         * Calculate the contributions due to collisions from above
         * a volume layer.
         */
        virtual void compute_lower_volume_energy() ;

        /**
         * Takes a set of eigenrays, boundary model, and convolves the set of
         * eigenverbs with itself and makes contributions to the reverebation
         * level curve.
         */
        void convolve_eigenverbs( std::vector<eigenverb>* set ) ;
//        void convolve_eigenverbs( const eigenverb_collection::eigenverb_tree* set ) ;

};

/// @}
}  // end of namespace waveq3d
}  // end of namespace usml
