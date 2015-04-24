/**
 * @file envelope_bistatic.h
 */
#pragma once

#include <usml/eigenverb/envelope_generator.h>

//namespace usml {
//namespace eigenverb {
//
//using namespace usml::ocean ;
//using boost::numeric::ublas::vector;

/// @ingroup eigenverb
/// @{

///**
// * A reverberation model listens for interface collision callbacks from
// * a wavefront. Used in bistatic scenarios, i.e. source and receiver are not
// * co-located.
// */
//class USML_DECLSPEC envelope_bistatic : public envelope_generator {
//
//    public:
//
//        /**
//         * Constructor
//         *
//         * @param ocean
//         * @param pulse
//         * @param max_time
//         */
//        envelope_bistatic(
//            ocean_model& ocean,
//            double pulse,
//            double max_time ) ;
//
//        /**
//         * Destructor
//         */
//        virtual ~envelope_bistatic() ;
//
//    private:
//
//        /**
//         * Computes the energy contributions to the reverberation
//         * energy curve from the bottom interactions.
//         *
//         * @param source
//         * @param receiver
//         * @param levels
//         */
//        virtual void compute_bottom_energy(
//                const eigenverb_collection& source,
//                const eigenverb_collection& receiver,
//                envelope_collection* levels ) ;
//
//        /**
//         * Computes the energy contributions to the reverberation
//         * energy curve from the surface interactions.
//         *
//         * @param source
//         * @param receiver
//         * @param levels
//         */
//        virtual void compute_surface_energy(
//                const eigenverb_collection& source,
//                const eigenverb_collection& receiver,
//                envelope_collection* levels ) ;
//
//        /**
//         * Calculate the contributions due to collisions from below
//         * a volume layer.
//         *
//         * @param source
//         * @param receiver
//         * @param levels
//         */
//        virtual void compute_upper_volume_energy(
//                const eigenverb_collection& source,
//                const eigenverb_collection& receiver,
//                envelope_collection* levels ) ;
//
//        /**
//         * Calculate the contributions due to collisions from above
//         * a volume layer.
//         *
//         * @param source
//         * @param receiver
//         * @param levels
//         */
//        virtual void compute_lower_volume_energy(
//                const eigenverb_collection& source,
//                const eigenverb_collection& receiver,
//                envelope_collection* levels ) ;
//
//        /**
//         * Takes a set of eigenrays, boundary model, and convolves the set of
//         * eigenverbs with itself and makes contributions to the reverebation
//         * level curve.
//         */
//        void convolve_eigenverbs(
//                const eigenverb_list& source,
//                const eigenverb_list& receiver,
//                envelope_collection* levels ) ;
//
//        /**
//         * Defines the type of boundary model for the bottom.
//         */
//        boundary_model* _bottom_boundary ;
//
//        /**
//         * Defines the type of boundary model for the surface.
//         */
//        boundary_model* _surface_boundary ;
//
//};
//
///// @}
//}  // end of namespace waveq3d
//}  // end of namespace usml
