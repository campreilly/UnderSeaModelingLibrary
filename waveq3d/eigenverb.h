/**
 * @file eigenverb.h
 * A single acoustic path between a source or receiver and a boundary.
 */
#pragma once

#include <usml/types/types.h>

namespace usml {
namespace waveq3d {

using namespace usml::types ;

using boost::numeric::ublas::vector;

/// @ingroup waveq3d
/// @{

/**
 * A single acoustic path between a source or receiver and a boundary.
 */
struct eigenverb {

    /**
     * One way travel time for this path (sec).
     */
    double travel_time ;

    /**
     * One way transmission loss for this path (linear units).
     */
    vector< double > intensity ;

    /**
     * The grazing angle of this path at impact of the
     * boundary. (radians)
     */
    double grazing ;

    /**
     * Path length from to the boundary interaction
     */
    double distance ;

    /**
     * Location of impact with the boundary.
     */
    wposition1 position ;

    /**
     * Normalized derivative of the location at the impact.
     * This is used to compute the azimuthal angle between
     * two eigenverbs.
     */
    wvector1 direction ;

    /**
     * Frequeinces of the wavefront (Hz)
     */
    const seq_vector* frequencies ;

    /**
     * The index of the lanuch DE.
     */
    size_t de_index ;

    /**
     * The index of the lanuch AZ.
     */
    size_t az_index ;

    /**
     * The DE of the path from launch.
     */
    double launch_de ;

    /**
     * The AZ of the path from launch.
     */
    double launch_az ;

    /**
     * Sigma in the DE dimension
     */
    double sigma_de ;

    /**
     * Sigma in the AZ dimension
     */
    double sigma_az ;

    /**
     * The speed of sound at the point of impact
     * with the boundary.
     */
    double sound_speed ;

    /**
     * Number of interactions with the surface boundary
     */
    size_t surface ;

    /**
     * Number of interactions with the bottom boundary
     */
    size_t bottom ;

};

/// @}
}  // end of namespace waveq3d
}  // end of namespace usml
